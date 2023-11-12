#include "motion_controller.hpp"


void buttonHandler(const kobuki::ButtonEvent &event) {
    std::cout << "motion controller: buttonHandler" << std::endl;
    if (event.state == kobuki::ButtonEvent::Released) {
        if (event.button == kobuki::ButtonEvent::Button0) {
            //button0_flag = true;
            cout << "here" << endl;
        }
    }
}

MotionController::MotionController(double target_x, double target_y) : ultrasonic_sensor(DEFAULT_TRIGGER_PIN, DEFAULT_ECHO_PIN){
    this->temp_target_x = this->target_x = target_x;
    this->temp_target_y = this->target_y = target_y;
    start_time.stamp();
    robot_mode = GO_TO_GOAL_MODE;
    moving_state = ADJUST_HEADING;
    button0_flag = false;
    kobuki_manager.setUserButtonEventCallBack(buttonHandler);
}

MotionController::~MotionController() {
    map_manager.printMap();
}

void MotionController::Bug2Algorithm() {
    const double buffer = 0.05;
    double longitudinal_velocity = 0.0;
    double rotational_velocity = 0.0;

    vector<double> arr = kobuki_manager.getCoordinates();
    current_x = arr[0];
    current_y = arr[1];
    current_yaw = kobuki_manager.getAngle();

    std::cout << ecl::green << "TimeStamp:" << double(ecl::TimeStamp() - start_time) << ". [x: " << current_x << ", y: " << current_y;
    std::cout << ", heading: " << current_yaw << "]. Bumper State: " << kobuki_manager.getBumperState() << ecl::reset << endl;

    checkSensors();
    setObstacleFlags();

    // Start of BUG 2 Algorithm
    if (robot_mode == GO_TO_GOAL_MODE)
    { // "go to target mode"
        if (moving_state == GO_STRAIGHT && (kobuki_manager.getBumperState() != 0 || right_front_obstacle || front_obstacle || left_front_obstacle))
        {   // HIT!
            kobuki_manager.move(longitudinal_velocity, rotational_velocity);
            robot_mode = WALL_FOLLOWING_MODE; // wall following mode
            // save hit point coordinates:
            hit_x = current_x;
            hit_y = current_y;
            distance_to_goal_from_hit_point = sqrt((
                    pow(target_x - hit_x, 2)) +
                    (pow(target_y - hit_y, 2)));
            map_manager.printMap();
            cout << "hit_x: " << hit_x << " hit_y: " << hit_y << " distance_to_goal_from_hit_point: " << distance_to_goal_from_hit_point << endl;
            std::cout << "GO_TO_GOAL_MODE, GO_STRAIGHT -> WALL_FOLLOWING_MODE robot_mode: " << robot_mode << " WALL_FOLLOWING_MODE, moving_mode: " << moving_state << endl;
            // stop and switch to wall mode
            return;
        }

        if (moving_state == ADJUST_HEADING)
        { // ADJUST HEADING
            double yaw_error = getYawError(current_x, current_y, current_yaw, target_x, target_y);
            // Adjust heading if heading is not good enough
            if (fabs(yaw_error) > yaw_precision)
            {
                if (yaw_error > 0.5)
                {
                    // Turn left (counterclockwise)
                    rotational_velocity = FAST_ROTATION_SPEED;
                }
                else if (yaw_error > 0)
                { // very close, rotate slowly
                    // Turn left (counterclockwise)
                    longitudinal_velocity = FORWARD_SPEED * 0.5;
                    rotational_velocity = ROTATION_SPEED;
                }
                else if (yaw_error < -0.5)
                {
                    // Turn right (clockwise)
                    rotational_velocity = -FAST_ROTATION_SPEED;
                }
                else
                { // very close, rotate slowly
                    // Turn right (clockwise)
                    longitudinal_velocity = FORWARD_SPEED * 0.5;
                    rotational_velocity = -ROTATION_SPEED;
                }
            }
            else
            {
                moving_state = GO_STRAIGHT;
                std::cout << "ADJUST_HEADING -> GO_STRAIGHT robot_mode: " << robot_mode << ", moving_mode: GO_STRAIGHT " << moving_state << endl;
            }
        }
        else if (moving_state == GO_STRAIGHT)
        { // GO STRAIGHT
            double position_error = sqrt(
                pow(target_x - current_x, 2) + pow(target_y - current_y, 2));

            if (position_error > 0.25)
            {
                longitudinal_velocity = FORWARD_SPEED;
                // How far off is the current heading in radians?
                double yaw_error = getYawError(current_x, current_y, current_yaw, target_x, target_y);
                cout << "yaw_error: " << yaw_error << " position_error: " << position_error << endl;

                // Adjust heading if heading is not good enough
                if (fabs(yaw_error) > yaw_precision + 0.2)
                {
                    moving_state = ADJUST_HEADING; // ADJUST HEADING
                    std::cout << "GO_STRAIGHT -> ADJUST_HEADING robot_mode: " << robot_mode << ", moving_mode: " << moving_state << endl;
                }
            }
            else
            {   // If distance to target is smaller than 20cm
                moving_state = GOAL_ACHIEVED; // finish successfully
                std::cout << "GO_STRAIGHT -> GOAL_ACHIEVED robot_mode: " << robot_mode << ", moving_mode: " << moving_state << endl;
                kobuki_manager.move(0.0, 0.0);
                kobuki_manager.playSoundSequence(0x6);
                cout << "DONE!" << endl;
            }
        }
        else if (moving_state == GOAL_ACHIEVED)
        { // GOAL_ACHIEVED
            if (target_x > 0.1) {
                target_x = 0.0;
                target_y = 0.0;
                moving_state = ADJUST_HEADING;
                //kobuki_manager.playSoundSequence(0x5);
                std::cout << "GOAL_ACHIEVED -> ADJUST_HEADING robot_mode: " << robot_mode << ", moving_mode: " << moving_state << endl;
            }
            if (button0_flag)
            {
                cout << "B0 pressed!!!" << endl;
                if (fabs(target_x) > 0.1)
                {
                    target_x = 0.0;
                }
                else
                {
                    target_x = temp_target_x;
                }
                target_y = 0.0;
                moving_state = ADJUST_HEADING;
                kobuki_manager.playSoundSequence(0x5);
                std::cout << "GOAL_ACHIEVED -> ADJUST_HEADING Robot mode: " << robot_mode << ", moving mode: " << moving_state << endl;
                button0_flag = false;
            }
        }
    }
    else if (robot_mode == WALL_FOLLOWING_MODE)
    { // "wall following mode"
        // Distance to the line:
        double a = hit_y - target_y;
        double b = target_x - hit_x;
        double c = hit_x * target_y - target_x * hit_y;
        double distance_to_target_line = abs(a * current_x + b * current_y + c) / sqrt(a * a + b * b);
        if (distance_to_target_line < 0.10)
        { // If we hit the start-goal line again?
            // Is the leave point closer to the goal than the hit point?
            // If yes, go to goal. (Should be at least 20cm closer in order to avoid looping)
            double distance_to_goal_from_crossing_point = sqrt(
                pow(target_x - current_x, 2) + pow(target_y - current_y, 2));
            if ((distance_to_goal_from_hit_point - distance_to_goal_from_crossing_point) > 0.2)
            {
                cout << "HIT the GOAL LINE! ";
                map_manager.printMap();
                robot_mode = GO_TO_GOAL_MODE; // "go to goal mode"
                moving_state = ADJUST_HEADING;
                std::cout << "WALL_FOLLOWING_MODE -> GO_TO_GOAL_MODE, ADJUST_HEADING Robot mode: " << robot_mode << ", moving mode: " << moving_state << endl;
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
        } else if (front_obstacle || left_front_obstacle)
        {
            // turn left and move forward slowly
            rotational_velocity = FAST_ROTATION_SPEED;
        } else if (right_front_obstacle)
        {
            // turn left and move forward slowly
            longitudinal_velocity = FORWARD_SPEED * 0.5;
            rotational_velocity = FAST_ROTATION_SPEED;
        } else if (right_obstacle)
        {
            // move straight to follow the wall
            longitudinal_velocity = FORWARD_SPEED;
        } else if (kobuki_manager.getBumperState() == 0)
        {
            // turn right and move forward slowly
            longitudinal_velocity = FORWARD_SPEED;
            rotational_velocity = -FAST_ROTATION_SPEED;
        }
    }
    kobuki_manager.move(longitudinal_velocity, rotational_velocity);
    return;
}

void MotionController::checkSensors() {
    double distance = ultrasonic_sensor.getDistance() * 0.01;
    std::cout << "Sensor Distance: " << distance << "m" << std::endl;
    if (distance > 0.02 && distance < 0.5) {
        map_manager.updateMapPolar(distance, kobuki_manager.getAngle(), current_x, current_y, 1, ROBOT_RADIUS * 0.3);
        map_manager.printMap();
    }
    map_manager.updateMap(current_x, current_y, 0);

    if (kobuki_manager.getBumperState() != 0) {
        map_manager.updateMapPolar(ROBOT_RADIUS, kobuki_manager.getAngle(), current_x, current_y, 1);
        map_manager.printMap();
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
