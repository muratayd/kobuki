#include "map_manager.hpp"
#include <ecl/geometry.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;
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
    // Read robot_id from config.json
    ifstream configFile("config.json");
    json config;
    configFile >> config;
    robot_id = config["robot_id"];
    configFile.close();
    // Initialize the MQTT client
    initialize_mqtt_client();
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
    // Create a JSON object and add robot_id and grid data
    nlohmann::json j;
    j["robot_id"] = robot_id;
    j["grid"] = flattened_data;

    // Convert the JSON object to a string
    std::string payload = j.dump();

    // Publish the message with robot_id and grid data to the MQTT broker
    client_.publish(TOPIC, payload);
    std::cout << "Grid map sent to MQTT" << std::endl;
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

void MapManager::handleMapMessage(const std::vector<uint8_t>& received_map_data) {
    cout << "Received shared map data" << endl;
    // Deserialize the received map data into the occupancy grid format
    std::vector<int> received_grid(MAP_SIZE * MAP_SIZE, -1); // Assuming a default value of -1 for unknown cells
    for (size_t i = 0; i < received_map_data.size(); ++i) {
        received_grid[i] = static_cast<int>(received_map_data[i]);
    }

    // Merge the received grid with the local occupancy grid
    mergeMap(received_grid);
}

void MapManager::mergeMap(const std::vector<int>& received_grid) {
    cout << "Merging received map with local map" << endl;
    return;
    // Assuming received_grid is a 1D vector representing the received occupancy grid
    for (int i = 0; i < MAP_SIZE; ++i) {
        for (int j = 0; j < MAP_SIZE; ++j) {
            int index = i * MAP_SIZE + j;
            // Simple conflict resolution: if either grid has an obstacle, mark it as an obstacle
            if (occupancy_grid[i][j] == 1 || received_grid[index] == 1) {
                occupancy_grid[i][j] = 1;
            }
        }
    }
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

void MapManager::initialize_mqtt_client() {
    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);

    // Set the message callback
    client_.set_message_callback([this](mqtt::const_message_ptr msg) {
        if (msg->get_topic() == TOPIC) {
            // Convert the payload to a vector of bytes
            std::vector<uint8_t> payload(msg->get_payload().begin(), msg->get_payload().end());
            // Handle the map message
            handleMapMessage(payload);
        }
    });

    // Connect to the MQTT broker
    try {
        cout << "MapManager: Connecting to the MQTT server..." << endl;
        client_.connect(connOpts)->wait();
        cout << "MapManager: Connected to the MQTT server" << endl;

        // Subscribe to the shared map topic
        //client_.subscribe(TOPIC, 1);
    } catch (const mqtt::exception& e) {
        cerr << "MapManager: Error connecting to the MQTT server: " << e.what() << endl;
    }
}
