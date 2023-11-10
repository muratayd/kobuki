#include "map_manager.hpp"
#include <ecl/geometry.hpp>

MapManager::MapManager() {
    for (auto &row : occupancy_grid)
    {
        for (auto &column : row)
        {
            column = -1;
        }
    }
}

MapManager::~MapManager() {
    return;
}

void MapManager::dilateCell(int x, int y, int value) {
    int grid_numbers_in_radius = (int)(ROBOT_RADIUS/GRID_SIZE) + 1;
    for (int row = y - grid_numbers_in_radius; row <= y + grid_numbers_in_radius; row++) {
        if (row < 0 || row >= MAP_SIZE) continue;
        for (int column = x - grid_numbers_in_radius; column <= x + grid_numbers_in_radius; column++) {
            if (column < 0 || column >= MAP_SIZE) continue;
            if ((abs(row-y) + abs(column-x)) <= grid_numbers_in_radius+1) {
                if (occupancy_grid[MAP_SIZE-row][column] != 1) {
                    occupancy_grid[MAP_SIZE-row][column] = value;
                }
            }
        }
    }
}

void MapManager::updateMap(double x, double y, int value) {
    std::cout << "updateMap" << std::endl;
    x = (int)round(x / GRID_SIZE) + MAP_ORIGIN;
    y = (int)round(y / GRID_SIZE) + MAP_ORIGIN;
    dilateCell(x, y, value);
    return;
}

void MapManager::checkMap(double x, double y, double yaw) {
    int row, column;
    front_obstacle = right_front_obstacle = right_obstacle = left_front_obstacle = left_obstacle = false;
    // check front
    row = (int)round((x + ROBOT_RADIUS * cos(yaw)) / GRID_SIZE) + MAP_ORIGIN;
    column = (int)round((y + ROBOT_RADIUS * sin(yaw)) / GRID_SIZE) + MAP_ORIGIN;
    if (occupancy_grid[MAP_SIZE-column][row] == 1) front_obstacle = true;
    // check right_front
    row = (int)round((x + ROBOT_RADIUS * cos(yaw-ecl::pi/6)) / GRID_SIZE) + MAP_ORIGIN;
    column = (int)round((y + ROBOT_RADIUS * sin(yaw-ecl::pi/6)) / GRID_SIZE) + MAP_ORIGIN;
    if (occupancy_grid[MAP_SIZE-column][row] == 1) right_front_obstacle = true;
    // check right
    row = (int)round((x + ROBOT_RADIUS * cos(yaw-ecl::pi*0.5)) / GRID_SIZE) + MAP_ORIGIN;
    column = (int)round((y + ROBOT_RADIUS * sin(yaw-ecl::pi*0.5)) / GRID_SIZE) + MAP_ORIGIN;
    if (occupancy_grid[MAP_SIZE-column][row] == 1) right_obstacle = true;
    // check left_front
    row = (int)round((x + ROBOT_RADIUS * cos(yaw+ecl::pi/6)) / GRID_SIZE) + MAP_ORIGIN;
    column = (int)round((y + ROBOT_RADIUS * sin(yaw+ecl::pi/6)) / GRID_SIZE) + MAP_ORIGIN;
    if (occupancy_grid[MAP_SIZE-column][row] == 1) left_front_obstacle = true;
    // check left
    row = (int)round((x + ROBOT_RADIUS * cos(yaw+ecl::pi*0.5)) / GRID_SIZE) + MAP_ORIGIN;
    column = (int)round((y + ROBOT_RADIUS * sin(yaw+ecl::pi*0.5)) / GRID_SIZE) + MAP_ORIGIN;
    if (occupancy_grid[MAP_SIZE-column][row] == 1) left_obstacle = true;
}

void MapManager::printMap()
{
    for (auto &row : occupancy_grid)
    {
        for (auto &column : row)
        {     
            if (column != -1) cout << column;
            else cout << " "; // print undiscovered cells as empty
        }
        cout << endl;
    }
}