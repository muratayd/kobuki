#include "motion_controller.hpp"

MotionController::MotionController() : ultrasonic_sensor(DEFAULT_TRIGGER_PIN, DEFAULT_ECHO_PIN){

}

MotionController::~MotionController() {

}

void MotionController::Bug2Algorithm() {
    const double buffer = 0.05;
    double longitudinal_velocity = 0.0;
    double rotational_velocity = 0.0;
    //checkMap();
    kobuki_manager.move(0.01, 0.1);
    double distance = ultrasonic_sensor.getDistance();
    std::cout << "Sensor Distance: " << distance << " cm" << std::endl;

    vector<double> arr = kobuki_manager.getCoordinates();
    std::cout << "TimeStamp:" << ecl::TimeStamp() << endl << "x: " << arr[0] << " y: " << arr[1];
    std::cout << " angle: " << kobuki_manager.getAngle() << endl;
    cout << "Bumper State: " << kobuki_manager.getBumperState() << endl;


    return;
}

double MotionController::getYawError(double current_x, double current_y, double current_yaw, double target_x, double target_y) {
    double desired_yaw = atan2(
        target_y - current_y,
        target_x - current_x);
    // How far off is the current heading in radians?
    double yaw_error = desired_yaw - current_yaw;
    return ecl::wrap_angle(yaw_error);
}