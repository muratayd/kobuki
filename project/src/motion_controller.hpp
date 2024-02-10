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
#include "map_manager.hpp"
#include "mqtt/async_client.h"
#include <mutex>
#include <condition_variable>

/*****************************************************************************
** Defines
*****************************************************************************/

#define FORWARD_SPEED 0.2
#define ROTATION_SPEED 0.8
#define FAST_ROTATION_SPEED 1.1
#define LEFT_BUMPER 4
#define RIGHT_BUMPER 1
#define CENTER_BUMPER 2
#define CM_TO_M 0.01
#define MM_TO_M 0.001

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
     */
    MotionController();

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
    double current_x;
    double current_y;
    double current_yaw;
    // UWB coordinates
    double UWB_x;
    double UWB_y;
    double UWB_yaw;
    // Target coordinates from a file
    double target_x;
    double target_y;
    double temp_target_x;
    double temp_target_y;
    // Coordinates of the first hit point
    double hit_x;
    double hit_y;
    double distance_to_goal_from_hit_point;
    // +/- 7.0 degrees of precision for the rotation angle
    double yaw_precision = 7.0 * (ecl::pi / 180);
    // variables for occupancy grid obstacles closer than 10cm
    bool left_obstacle, left_front_obstacle, front_obstacle, right_front_obstacle, right_obstacle, center_obstacle;
    bool button0_flag;
    ecl::TimeStamp start_time;

    mqtt::async_client mqtt_client;
    void mqtt_message_arrived(mqtt::const_message_ptr msg);
    void initialize_mqtt_client();
    std::mutex mtx;
    std::condition_variable cv;
    bool pozyx_position_received = false; // This flag will be set to true once the first pozyx/position message arrives

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
    void checkDistance(double sensor_distance);
    /*

    /*
    * Set obstacle flags
    */
    void setObstacleFlags();
    /*

    * Button handler callback function
    */
    //static void buttonHandler(const kobuki::ButtonEvent &event);

    /* Add this line for the MQTT connection success handler
    * MQTT connection success handler
    */
    void on_connected(const std::string& cause);

};

#endif /* MOTION_CONTROLLER_HPP_ */