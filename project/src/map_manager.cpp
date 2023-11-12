#include "map_manager.hpp"
#include <ecl/geometry.hpp>

using namespace std;

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

void MapManager::dilateCell(int x, int y, int value, double radius) {
    int grid_numbers_in_radius = (int)(radius/GRID_SIZE) + 1;
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

void MapManager::updateMap(double x, double y, int value, double radius) {
    int row = (int)round(x / GRID_SIZE) + MAP_ORIGIN;
    int column = (int)round(y / GRID_SIZE) + MAP_ORIGIN;
    if (row < 0 || row >= MAP_SIZE || column < 0 || column >= MAP_SIZE) {
        throw std::out_of_range("updateMap Coordinates out of map boundaries");
    }
    std::cout << "updateMap x:" << row << " y:" << column << " val:" << value << std::endl;
    dilateCell(row, column, value, radius);
    return;
}

bool MapManager::checkMap(double x, double y) {
    int row = (int)round(x / GRID_SIZE) + MAP_ORIGIN;
    int column = (int)round(y / GRID_SIZE) + MAP_ORIGIN;
    if (row < 0 || row >= MAP_SIZE || column < 0 || column >= MAP_SIZE) {
        throw std::out_of_range("checkMap Coordinates out of map boundaries");
    }
    return (occupancy_grid[MAP_SIZE-column][row] == 1);
}

// New methods for polar coordinate support
void MapManager::updateMapPolar(double distance, double angle, double initial_x, double initial_y, int value, double radius) {
    double x = distance * cos(angle) + initial_x; // Convert to Cartesian x with initial point
    double y = distance * sin(angle) + initial_y; // Convert to Cartesian y with initial point
    updateMap(x, y, value, radius); // Delegate to updateMap with Cartesian coordinates
}

bool MapManager::checkMapPolar(double distance, double angle, double initial_x, double initial_y) {
    double x = distance * cos(angle) + initial_x; // Convert to Cartesian x with initial point
    double y = distance * sin(angle) + initial_y; // Convert to Cartesian y with initial point
    return checkMap(x, y); // Delegate to checkMap with Cartesian coordinates
}

void MapManager::printMap()
{
    for (auto &row : occupancy_grid) {
        bool allZeros = true; // skip the lines that contain only zeros:
        for (auto &column : row) {
            if (column != -1) {
                allZeros = false;
                break;
            }
        }
        if (!allZeros) {
            for (auto &column : row)
            {
                if (column == -1) {
                    cout << ".";
                } else if (column == 0) {
                    cout << " ";
                } else {
                    cout << "#";
                }
            }
            cout << endl;
        }
    }
}