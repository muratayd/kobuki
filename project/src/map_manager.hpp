/**
 * @file map_manager.hpp
 *
 * @brief Simple map management module for occupancy grid algorithms
**/

/*****************************************************************************
** Ifdefs
*****************************************************************************/

#ifndef MAP_MANAGER_HPP_
#define MAP_MANAGER_HPP_

/*****************************************************************************
** Includes
*****************************************************************************/

#include <iostream>

/*****************************************************************************
** Defines
*****************************************************************************/

#define MAP_SIZE 200 // n of cells
#define MAP_ORIGIN 100 // origin point is at [100][100]
#define GRID_SIZE 0.05 // m
#define ROBOT_RADIUS 0.17 // m

using namespace std;

/*****************************************************************************
** Classes
*****************************************************************************/

class MapManager
{
public:
    MapManager();

    ~MapManager();

    /*
    * Mark the object cell from the hit point, and use Dilation function
    * Technique used for mapping is Point-robot assumption.
    * Meaning that the robot is represented as a point, and obstacles are Dilated by robot’s radius
    */
    void updateMap(double x, double y, double yaw, int value);

    /*
    * Check obstacles in front for movement controls
    */
    void checkMap(double x, double y, double yaw);

private:
    int occupancy_grid[MAP_SIZE][MAP_SIZE];
    // variables for occupancy grid obstacles closer than 10cm
    bool left_obstacle, left_front_obstacle, front_obstacle, right_front_obstacle, right_obstacle;
    
    /*
    * Dilates the cell with the robot radius
    */
    void dilateCell(int x, int y, int value);
    
};

#endif /* MAP_MANAGER_HPP_ */