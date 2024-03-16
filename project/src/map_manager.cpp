#include "map_manager.hpp"
#include <ecl/geometry.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>

using namespace std;

const string SERVER_ADDRESS("tcp://192.168.0.12:1883");
const string CLIENT_ID("map_publisher");
const string TOPIC("robot/map");

MapManager::MapManager() : client_(SERVER_ADDRESS, CLIENT_ID), callback_() {
    for (auto &row : occupancy_grid)
    {
        for (auto &column : row)
        {
            column = -1;
        }
    }
    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);
    client_.set_callback(callback_);
    cout << "Connecting to server..." << endl;
    client_.connect(connOpts)->wait();
    cout << "Connected" << endl;
    sendGridToMQTT();
}

MapManager::~MapManager() {
    client_.disconnect()->wait();
    cout << "Disconnected" << endl;
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

void MapManager::sendGridToMQTT() {
    // Flatten the 2D array into a 1D array
    vector<int> flattened_data;
    for (auto &row : occupancy_grid)
    {
        for (auto &column : row)
        {
            flattened_data.push_back(column);
        }
    }
    // Publish the flattened array
    client_.publish(TOPIC, flattened_data.data(), flattened_data.size() * sizeof(int), 0, false);
    cout << "Grid map sent to MQTT" << endl;
}

void MapManager::updateMap(double x, double y, int value, double radius) {
    int row = (int)round(x / GRID_SIZE) + MAP_ORIGIN;
    int column = (int)round(y / GRID_SIZE) + MAP_ORIGIN;
    //std::cout << "updateMap x:" << row << " y:" << column << " val:" << value << std::endl;
    if (row < 0 || row >= MAP_SIZE || column < 0 || column >= MAP_SIZE) {
        throw std::out_of_range("updateMap Coordinates out of map boundaries");
    }
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

void MapManager::printMap(double robot_x, double robot_y) {
    int robot_column = (int)round(robot_x / GRID_SIZE) + MAP_ORIGIN;
    int robot_row = MAP_SIZE - ((int)round(robot_y / GRID_SIZE) + MAP_ORIGIN);
    cout << robot_row << robot_column << endl;
    int row_cnt = 0;
    for (auto &row : occupancy_grid) {
        bool allZeros = true; // skip the lines that contain only zeros:
        for (auto &column : row) {
            if (column != -1) {
                allZeros = false;
                break;
            }
        }
        if (!allZeros) {
            int column_cnt = 0;
            for (auto &column : row) {
                if (robot_row == row_cnt && robot_column == column_cnt) {
                    cout << "+";
                } else if (column == -1) {
                    cout << ".";
                } else if (column == 0) {
                    cout << " ";
                } else {
                    cout << "#";
                }
                column_cnt++;
            }
            cout << endl;
        }
        row_cnt++;
    }
    sendGridToMQTT();
}