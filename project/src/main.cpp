//#include <string>
#include <fstream>
#include <sstream>
#include <csignal>
#include <iomanip>
#include "kobuki_manager.hpp"
#include "map_manager.hpp"
#include "motion_controller.hpp"

using namespace std;

bool shutdown_req = false;
//KobukiManager kobuki_manager;

void signalHandler(int /* signum */)
{
    shutdown_req = true;
}

void exampleCliffHandlerPrint(const kobuki::ButtonEvent &event) {
    std::cout << "HELLO CLIFF" << std::endl;
}

void exampleButtonHandler(const kobuki::ButtonEvent &event) {
    std::cout << "exampleButtonHandler" << std::endl;
    //if (event.state == kobuki::ButtonEvent::Released) {
        //if (event.button == kobuki::ButtonEvent::Button0) {
            //kobuki_manager.rotate(0.5);
        //} else if (event.button == kobuki::ButtonEvent::Button1) {
            //kobuki_manager.playSoundSequence(0x5);
        //}
    //}
}

/*****************************************************************************
** Main
*****************************************************************************/

int main(int argc, char **argv)
{
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
    //kobuki_manager.setUserButtonEventCallBack(examplePrint);
    //kobuki_manager.setUserButtonEventCallBack(exampleButtonHandler);
    //kobuki_manager.setUserCliffEventCallBack(exampleCliffHandlerPrint);
    int ultrasonic_sensor_trigger_pin = 18;
    int ultrasonic_sensor_echo_pin = 24;
    MotionController motion_controller;
    //kobuki_manager.playSoundSequence(0x6);
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