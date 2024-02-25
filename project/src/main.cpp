//#include <string>
#include <fstream>
#include <sstream>
#include <csignal>
#include <iomanip>
#include "kobuki_manager.hpp"
#include "map_manager.hpp"
#include "motion_controller.hpp"
#include "mqtt/async_client.h"
#include <nlohmann/json.hpp>

using namespace std;

bool shutdown_req = false;

void signalHandler(int /* signum */)
{
    shutdown_req = true;
}

// Create an MQTT client instance
mqtt::async_client mqtt_client("tcp://localhost:1883", "MainClient");

void bumperHandler(const kobuki::BumperEvent &event) {
    cout << "sending BumperEvent" << endl;
    // Convert the bumper state to a string
    std::string bumper = std::to_string(event.bumper);
    std::string state = std::to_string(event.state);
    // Create a JSON object
    nlohmann::json j;
    j["bumper"] = bumper;
    j["state"] = state;
    // Convert the JSON object to a string
    std::string message = j.dump();
    // Publish the message to the MQTT topic
    mqtt_client.publish("bumperTopic", message);
}

void cliffHandler(const kobuki::CliffEvent &event) {
    cout << "sending CliffEvent" << endl;
    // Convert the cliff state to a string
    std::string sensor = std::to_string(event.sensor);
    std::string state = std::to_string(event.state);
    // Create a JSON object
    nlohmann::json j;
    j["state"] = state;
    j["sensor"] = sensor;
    // Convert the JSON object to a string
    std::string message = j.dump();
    // Publish the message to the MQTT topic
    mqtt_client.publish("cliffTopic", message);
}

// MQTT Client Initialization
void initialize_mqtt_client() {
    using namespace std::placeholders;
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

/*****************************************************************************
** Main
*****************************************************************************/

int main(int argc, char **argv)
{
    initialize_mqtt_client();
    cout << setprecision(3);
    fstream fs;
    fs.open("target.txt", ios::in);
    vector<vector<float>> floatVec;
    string strFloat;
    float targetX;
    float targetY;
    int counter = 0;
    getline(fs, strFloat);
    cout << fixed;
    cout.precision(3);
    std::stringstream linestream(strFloat);
    linestream >> targetX;
    linestream >> targetY;
    std::cout << "target x: " << targetX << " y: " << targetY << std::endl;

    signal(SIGINT, signalHandler);

    ecl::MilliSleep sleep(1000);
    int ultrasonic_sensor_trigger_pin = 18;
    int ultrasonic_sensor_echo_pin = 24;
    KobukiManager kobuki_manager;
    kobuki_manager.setUserBumperEventCallBack(bumperHandler);
    kobuki_manager.setUserCliffEventCallBack(cliffHandler);
    MotionController motion_controller(kobuki_manager);
    try
    {
        while (!shutdown_req)
        {
            motion_controller.Bug2Algorithm();
            sleep(150);
        }
        motion_controller.stop();
    }
    catch (ecl::StandardException &e)
    {
        std::cout << e.what();
    }
    motion_controller.stop();
    sleep(300);
    return 0;
}