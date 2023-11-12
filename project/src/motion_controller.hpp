/**
 * @file motion_controller.hpp
 *
 * @brief Motion controller module for robot navigation with BUG algorithms
 *
 * This file defines the MotionController class, which is responsible for implementing
 * BUG navigation algorithms for a robot. It manages the robot's movements based on sensor
 * inputs and environmental data, aiming to navigate to a specified target while avoiding obstacles.
 *
**/

/*****************************************************************************
** Ifdefs
*****************************************************************************/

#ifndef MOTION_CONTROLLER_HPP_
#define MOTION_CONTROLLER_HPP_

/*****************************************************************************
** Includes
*****************************************************************************/

#include <iostream>
#include "kobuki_manager.hpp"
#include "UltrasonicSensor.hpp"
#include "map_manager.hpp"

/*****************************************************************************
** Defines
*****************************************************************************/

#define FORWARD_SPEED 0.2
#define ROTATION_SPEED 1.0
#define FAST_ROTATION_SPEED 1.2
#define LEFT_BUMPER 4
#define RIGHT_BUMPER 1
#define CENTER_BUMPER 2

using namespace std;

/*****************************************************************************
** Classes
*****************************************************************************/

class MotionController
{
public:
    /**
     * @brief Constructor for MotionController.
     * Initializes the controller with target coordinates and sets up necessary
     * parameters for motion control.
     * @param target_x The X coordinate of the target location.
     * @param target_y The Y coordinate of the target location.
     */
    MotionController(double target_x, double target_y);

    ~MotionController();

    /**
     * @brief Implements the Bug2 algorithm for navigation.
     *
     */
    void Bug2Algorithm();

    /**
     * @brief Stops the robot's movement.
     *
     */
    void stop();


private:
    // Predefined GPIO pin numbers for the ultrasonic sensor
    static const int DEFAULT_TRIGGER_PIN = 18;
    static const int DEFAULT_ECHO_PIN = 24;

    KobukiManager kobuki_manager;
    MapManager map_manager;
    UltrasonicSensor ultrasonic_sensor;
    double current_x;
    double current_y;
    double current_yaw;
    // Target coordinates from a file
    double target_x;
    double target_y;
    double temp_target_x;
    double temp_target_y;
    // Coordinates of the first hit point
    double hit_x;
    double hit_y;
    double distance_to_goal_from_hit_point;
    // +/- 5.0 degrees of precision for the rotation angle
    double yaw_precision = 5.0 * (ecl::pi / 180);
    // variables for occupancy grid obstacles closer than 10cm
    bool left_obstacle, left_front_obstacle, front_obstacle, right_front_obstacle, right_obstacle, center_obstacle;
    bool button0_flag;
    ecl::TimeStamp start_time;

    /*  ############# MAIN ROBOT MODES ###################
        "go to goal mode": Robot will head to an x,y coordinate
        "wall following mode": Robot will follow a wall */
    enum robot_mode_enum
    {
        GO_TO_GOAL_MODE,
        WALL_FOLLOWING_MODE
    } robot_mode;

    /*  ############ GO TO TARGET MODES ##################
        adjust heading: Rotate towards the target
        go straight: Go straight towards the target
        goal achieved: Reached the goal x, y coordinates */
    enum moving_state_enum
    {
        ADJUST_HEADING,
        GO_STRAIGHT,
        GOAL_ACHIEVED
    } moving_state;

    /*
    * Calculate the desired heading based on the current position and the desired position
    */
    double getYawError(double current_x, double current_y, double current_yaw, double target_x, double target_y);
    /*

    /*
    * Check bumper and ultrasonic sensors
    */
    void checkSensors();
    /*

    /*
    * Set obstacle flags
    */
    void setObstacleFlags();
    /*

    * Button handler callback function
    */
    //static void buttonHandler(const kobuki::ButtonEvent &event);

};

#endif /* MOTION_CONTROLLER_HPP_ */