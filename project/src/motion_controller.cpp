#include "motion_controller.hpp"
#include <nlohmann/json.hpp>

// Use nlohmann::json namespace for convenience
using json = nlohmann::json;
using namespace std;

void buttonHandler(const kobuki::ButtonEvent &event) {
    cout << "motion controller: buttonHandler" << endl;
    if (event.state == kobuki::ButtonEvent::Released) {
        if (event.button == kobuki::ButtonEvent::Button0) {
            //button0_flag = true;
            cout << "here" << endl;
        }
    }
}

MotionController::MotionController() : mqtt_client("tcp://localhost:1883", "MotionControllerClient")  {
    this->temp_target_x = this->target_x = 0.0;
    this->temp_target_y = this->target_y = 0.0;
    UWB_x = 0.0;
    UWB_y = 0.0;
    UWB_yaw = 0.0;
    start_time.stamp();
    robot_mode = GO_TO_GOAL_MODE;
    moving_state = GOAL_ACHIEVED;
    button0_flag = false;
    kobuki_manager.setUserButtonEventCallBack(buttonHandler);
    initialize_mqtt_client();
    // Wait for the first pozyx/position message
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return pozyx_position_received; });
    kobuki_manager.setInitialPose(current_x, current_y, current_yaw);
}

MotionController::~MotionController() {
    map_manager.printMap(current_x, current_y);
    // Ensure to disconnect MQTT Client
    try {
        std::cout << "Disconnecting MQTT client..." << std::endl;
        mqtt_client.disconnect();
    } catch (const mqtt::exception& exc) {
        std::cerr << "MQTT disconnection failed: " << exc.what() << std::endl;
    }
}

// MQTT Client Initialization
void MotionController::initialize_mqtt_client() {
    using namespace std::placeholders;
    mqtt_client.set_connected_handler(std::bind(&MotionController::on_connected, this, _1));
    mqtt_client.set_message_callback(std::bind(&MotionController::mqtt_message_arrived, this, _1));

    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);

    try {
        std::cout << "Connecting to the MQTT broker..." << std::endl;
        mqtt_client.connect(connOpts)->wait();
    } catch (const mqtt::exception& exc) {
        std::cerr << exc.what() << std::endl;
        // Handle connection failure
    }
}

// MQTT Connection Success Handler
void MotionController::on_connected(const std::string& cause) {
    std::cout << "MQTT connection success" << std::endl;
    // Subscribe to topics
    mqtt_client.subscribe("sensor/distance", 1);
    mqtt_client.subscribe("pozyx/position", 1);
    mqtt_client.subscribe("robot/target", 1);
}

// MQTT Message Arrival Handler
void MotionController::mqtt_message_arrived(mqtt::const_message_ptr msg) {
    std::cout << "Message arrived: " << msg->get_topic() << ": " << msg->to_string() << std::endl;
    // Handle message based on topic
    if (msg->get_topic() == "sensor/distance") {
        // Update based on distance message
        // Convert the payload to a string
        std::string payload = msg->to_string();
        double distance = 0.0;
        try {
            // Attempt to convert the string payload to a double
            distance = std::stod(payload);

            std::cout << "Distance received: " << distance << " units" << std::endl;

            // Now you can use the distance value as needed in your application
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid argument: Could not convert the payload to a double." << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Out of range: The conversion resulted in an overflow or underflow." << std::endl;
        }
        if (pozyx_position_received) {
            checkDistance(distance);
        }
    } else if (msg->get_topic() == "pozyx/position") {
        // Update based on position message
        std::unique_lock<std::mutex> lock(mtx);
        try {
            // Convert the payload to a string and parse it as JSON
            auto payload = msg->to_string();
            auto j = json::parse(payload);

            // Extract the position data
            double x = j["x"];
            double y = j["y"];
            double heading = j["heading"];
            current_x = x * 0.001;
            current_y = y * 0.001;
            current_yaw = 2.0 * ecl::pi - (heading  * (ecl::pi / 180.0));

            std::cout << "Position received - X: " << current_x << ", Y: " << current_y << ", heading: " << current_yaw << std::endl;

            // Now you can use x, y, z as needed in your application
        } catch (json::parse_error& e) {
            std::cerr << "Parsing error: " << e.what() << '\n';
        } catch (json::type_error& e) {
            std::cerr << "Type error: " << e.what() << '\n';
        } catch (std::exception& e) {
            std::cerr << "Some other error: " << e.what() << '\n';
        }
        // Set the flag and notify
        pozyx_position_received = true;
        cv.notify_one();
    } else if (msg->get_topic() == "robot/target") {
        // Update based on target message
        std::string payload = msg->to_string();
        try {
            // Attempt to convert the string payload to a double
            auto j = json::parse(payload);
            double x = j["x"];
            double y = j["y"];
            temp_target_x = target_x = x;
            temp_target_y = target_y = y;
            moving_state = ADJUST_HEADING;
            kobuki_manager.playSoundSequence(0x5);
            std::cout << "New target received: X: " << temp_target_x << ", Y: " << temp_target_y << std::endl;
        } catch (json::parse_error& e) {
            std::cerr << "Parsing error: " << e.what() << '\n';
        } catch (json::type_error& e) {
            std::cerr << "Type error: " << e.what() << '\n';
        } catch (std::exception& e) {
            std::cerr << "Some other error: " << e.what() << '\n';
        }
    }
}

void MotionController::Bug2Algorithm() {
    const double buffer = 0.05;
    double longitudinal_velocity = 0.0;
    double rotational_velocity = 0.0;

    vector<double> arr = kobuki_manager.getCoordinates();
    current_x = (current_x + arr[0]) * 0.5;
    current_y = (current_y + arr[1]) * 0.5;
    //current_yaw = kobuki_manager.getAngle();

    cout << ecl::green << "TimeStamp:" << double(ecl::TimeStamp() - start_time) << ". [x: " << current_x << ", y: " << current_y;
    cout << ", heading: " << current_yaw << "]. Bumper State: " << kobuki_manager.getBumperState() << ". array: " << arr[0] << " " << arr[1] << ecl::reset << endl;

    setObstacleFlags();

    // Start of BUG 2 Algorithm
    if (robot_mode == GO_TO_GOAL_MODE) { // "go to target mode"
        if ((moving_state == GO_STRAIGHT) && (kobuki_manager.getBumperState() != 0 || right_front_obstacle || front_obstacle || left_front_obstacle)) { // HIT!
            kobuki_manager.move(longitudinal_velocity, rotational_velocity);
            robot_mode = WALL_FOLLOWING_MODE; // wall following mode
            // save hit point coordinates:
            hit_x = current_x;
            hit_y = current_y;
            distance_to_goal_from_hit_point = sqrt((
                    pow(target_x - hit_x, 2)) +
                    (pow(target_y - hit_y, 2)));
            map_manager.printMap(current_x, current_y);
            cout << "hit_x: " << hit_x << " hit_y: " << hit_y << " distance_to_goal_from_hit_point: " << distance_to_goal_from_hit_point << endl;
            cout << "GO_TO_GOAL_MODE, GO_STRAIGHT -> WALL_FOLLOWING_MODE robot_mode: " << robot_mode << " WALL_FOLLOWING_MODE, moving_mode: " << moving_state << endl;
            // stop and switch to wall mode
            return;
        }

        if (moving_state == ADJUST_HEADING) { // ADJUST HEADING
            double yaw_error = getYawError(current_x, current_y, current_yaw, target_x, target_y);
            // Adjust heading if heading is not good enough
            if (fabs(yaw_error) > yaw_precision) {
                if (yaw_error > 0.5) {
                    // Turn left (counterclockwise)
                    rotational_velocity = FAST_ROTATION_SPEED;
                } else if (yaw_error > 0) { // very close, rotate slowly
                    // Turn left (counterclockwise)
                    longitudinal_velocity = FORWARD_SPEED * 0.2;
                    rotational_velocity = ROTATION_SPEED;
                } else if (yaw_error < -0.5) {
                    // Turn right (clockwise)
                    rotational_velocity = -FAST_ROTATION_SPEED;
                } else { // very close, rotate slowly
                    // Turn right (clockwise)
                    longitudinal_velocity = FORWARD_SPEED * 0.2;
                    rotational_velocity = -ROTATION_SPEED;
                }
            } else {
                moving_state = GO_STRAIGHT;
                cout << "ADJUST_HEADING -> GO_STRAIGHT robot_mode: " << robot_mode << ", moving_mode: GO_STRAIGHT " << moving_state << endl;
            }
        } else if (moving_state == GO_STRAIGHT) { // GO STRAIGHT
            double position_error = sqrt(
                pow(target_x - current_x, 2) + pow(target_y - current_y, 2));

            if (position_error > 0.25) {
                longitudinal_velocity = FORWARD_SPEED;
                // How far off is the current heading in radians?
                double yaw_error = getYawError(current_x, current_y, current_yaw, target_x, target_y);
                cout << "yaw_error: " << yaw_error << " position_error: " << position_error << endl;

                // Adjust heading if heading is not good enough
                if (fabs(yaw_error) > yaw_precision + 0.2) {
                    moving_state = ADJUST_HEADING; // ADJUST HEADING
                    cout << "GO_STRAIGHT -> ADJUST_HEADING robot_mode: " << robot_mode << ", moving_mode: " << moving_state << endl;
                }
            } else {   // If distance to target is smaller than 20cm
                moving_state = GOAL_ACHIEVED; // finish successfully
                cout << "GO_STRAIGHT -> GOAL_ACHIEVED robot_mode: " << robot_mode << ", moving_mode: " << moving_state << endl;
                kobuki_manager.move(0.0, 0.0);
                kobuki_manager.playSoundSequence(0x6);
                cout << "DONE!" << endl;
            }
        } else if (moving_state == GOAL_ACHIEVED) { // GOAL_ACHIEVED
            /*TODO: use target list or buttons for new targets in the future*/
            if (button0_flag) {
                cout << "B0 pressed!!!" << endl;
                target_x = 0.0;
                target_y = 0.0;
                moving_state = ADJUST_HEADING;
                kobuki_manager.playSoundSequence(0x5);
                cout << "GOAL_ACHIEVED -> ADJUST_HEADING Robot mode: " << robot_mode << ", moving mode: " << moving_state << endl;
                button0_flag = false;
            }
        }
    } else if (robot_mode == WALL_FOLLOWING_MODE) { // "wall following mode"
        // Distance to the line:
        double a = hit_y - target_y;
        double b = target_x - hit_x;
        double c = hit_x * target_y - target_x * hit_y;
        double distance_to_target_line = abs(a * current_x + b * current_y + c) / sqrt(a * a + b * b);
        if (distance_to_target_line < 0.10) { // If we hit the start-goal line again?
            // Is the leave point closer to the goal than the hit point?
            // If yes, go to goal. (Should be at least 20cm closer in order to avoid looping)
            double distance_to_goal_from_crossing_point = sqrt(
                pow(target_x - current_x, 2) + pow(target_y - current_y, 2));
            if ((distance_to_goal_from_hit_point - distance_to_goal_from_crossing_point) > 0.2) {
                cout << "HIT the GOAL LINE! ";
                map_manager.printMap(current_x, current_y);
                robot_mode = GO_TO_GOAL_MODE; // "go to goal mode"
                moving_state = ADJUST_HEADING;
                cout << "WALL_FOLLOWING_MODE -> GO_TO_GOAL_MODE, ADJUST_HEADING Robot mode: " << robot_mode << ", moving mode: " << moving_state << endl;
                kobuki_manager.move(longitudinal_velocity, rotational_velocity);
                return;
            }
        }

        // BUMPERS: 0, 1=R, 2=C, 4=L, 3=RC, 5=RL, 6=CL, 7=RCL
        if (kobuki_manager.getBumperState() != 0 || center_obstacle) {
            // move backwards
            cout << "Moving backwards" << endl;
            rotational_velocity = 0.0;
            longitudinal_velocity = -FORWARD_SPEED;
            //rotational_velocity = ROTATION_SPEED * 0.5;
        } else if (front_obstacle || left_front_obstacle) {
            // turn left and move forward slowly
            rotational_velocity = FAST_ROTATION_SPEED;
        } else if (right_front_obstacle) {
            // turn left and move forward slowly
            longitudinal_velocity = FORWARD_SPEED * 0.5;
            rotational_velocity = FAST_ROTATION_SPEED;
        } else if (right_obstacle) {
            // move straight to follow the wall
            longitudinal_velocity = FORWARD_SPEED;
        } else if (kobuki_manager.getBumperState() == 0) {
            // turn right and move forward slowly
            longitudinal_velocity = FORWARD_SPEED;
            rotational_velocity = -FAST_ROTATION_SPEED;
        }
    }
    kobuki_manager.move(longitudinal_velocity, rotational_velocity);
    return;
}

void MotionController::checkDistance(double sensor_distance) {
    double distance = sensor_distance * 0.01;
    cout << "Sensor Distance: " << distance << "m" << kobuki_manager.getAngle() << " " << current_x << " " << current_y << endl;
    if (distance > 0.02 && distance < 0.5) {
        map_manager.updateMapPolar(distance + ROBOT_RADIUS, kobuki_manager.getAngle(), current_x, current_y, 1, ROBOT_RADIUS * 0.8);
        map_manager.printMap(current_x, current_y);
    }
    map_manager.updateMap(current_x, current_y, 0);

    if (kobuki_manager.getBumperState() != 0) {
        map_manager.updateMapPolar(ROBOT_RADIUS, kobuki_manager.getAngle(), current_x, current_y, 1);
        map_manager.printMap(current_x, current_y);
    }
}

void MotionController::stop() {
    kobuki_manager.stop();
}

void MotionController::setObstacleFlags() {
    double x, y; // row, column
    // check center
    center_obstacle = map_manager.checkMap(current_x, current_y);
    // check front
    x = current_x + ROBOT_RADIUS * cos(current_yaw);
    y = current_y + ROBOT_RADIUS * sin(current_yaw);
    //cout << x << " " << y << endl;
    front_obstacle = map_manager.checkMap(x, y);
    // check right_front
    x = current_x + ROBOT_RADIUS * cos(current_yaw-ecl::pi/6);
    y = current_y + ROBOT_RADIUS * sin(current_yaw-ecl::pi/6);
    //cout << x << " " << y << endl;
    right_front_obstacle = map_manager.checkMap(x, y);
    // check right
    x = current_x + ROBOT_RADIUS * cos(current_yaw-ecl::pi*0.5);
    y = current_y + ROBOT_RADIUS * sin(current_yaw-ecl::pi*0.5);
    //cout << x << " " << y << endl;
    right_obstacle = map_manager.checkMap(x, y);
    // check left_front
    x = current_x + ROBOT_RADIUS * 0.90 * cos(current_yaw+ecl::pi/6);
    y = current_y + ROBOT_RADIUS * 0.90 * sin(current_yaw+ecl::pi/6);
    //cout << x << " " << y << endl;
    left_front_obstacle = map_manager.checkMap(x, y);
    // check left
    x = current_x + ROBOT_RADIUS * 0.90 * cos(current_yaw+ecl::pi*0.5);
    y = current_y + ROBOT_RADIUS * 0.90 * sin(current_yaw+ecl::pi*0.5);
    //cout << x << " " << y << endl;
    left_obstacle = map_manager.checkMap(x, y);
    cout << "OBSTACLES:    " <<  front_obstacle << endl;
    cout << "OBSTACLES:  " << left_front_obstacle << "   " << right_front_obstacle << endl;
    cout << "OBSTACLES:" << left_obstacle << "   " << center_obstacle << "   " << right_obstacle << endl;
}

double MotionController::getYawError(double current_x, double current_y, double current_yaw, double target_x, double target_y) {
    double desired_yaw = atan2(
        target_y - current_y,
        target_x - current_x);
    // How far off is the current heading in radians?
    double yaw_error = desired_yaw - current_yaw;
    return ecl::wrap_angle(yaw_error);
}
