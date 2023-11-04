#include <iostream>
#include <string>
#include <vector>
#include <ecl/geometry.hpp>
#include <ecl/time.hpp>
#include <ecl/sigslots.hpp>
#include <ecl/linear_algebra.hpp>
#include <ecl/command_line.hpp>
#include "kobuki_core/kobuki.hpp"
#include "kobuki_core/packets/core_sensors.hpp"

using namespace std;

typedef void (*userButtonEventCallBackType)(const kobuki::ButtonEvent &event);
typedef void (*userBumperEventCallBackType)(const kobuki::BumperEvent &event);
typedef void (*userCliffEventCallBackType)(const kobuki::CliffEvent &event);

/*****************************************************************************
** Classes
*****************************************************************************/

class KobukiManager {
public:
    KobukiManager();

    ~KobukiManager();

    void move(double longitudinal_velocity);

    void rotate(double rotational_velocity);

    void moveAndRotate(double longitudinal_velocity, double rotational_velocity);

    void stop();

    /*
    * Get x and y coordinates
    */
    vector<int> getCoordinates();

    /*
    * Get current angle in radians
    */
    int getAngle();

    void setUserButtonEventCallBack (userButtonEventCallBackType func);

    void setUserBumperEventCallBack (userBumperEventCallBackType func);

    void setUserCliffEventCallBack (userCliffEventCallBackType func);

private:
    kobuki::Kobuki kobuki;
    ecl::linear_algebra::Vector3d pose;    // x, y, heading
    ecl::Slot<> slot_stream_data;
    ecl::Slot<const kobuki::ButtonEvent&> slot_button_event;
    ecl::Slot<const kobuki::BumperEvent&> slot_bumper_event;
    ecl::Slot<const kobuki::CliffEvent&> slot_cliff_event;
    kobuki::CoreSensors::Data data;

    // Function pointer callbacks:
    userButtonEventCallBackType userButtonEventCallBack;
    userBumperEventCallBackType userBumperEventCallBack;
    userCliffEventCallBackType userCliffEventCallBack;

    ecl::ValueArg<string> device_port(
        "p", "port",
        "Path to device file of serial port to open",
        false,
        "/dev/kobuki",
        "string");
    ecl::SwitchArg disable_smoothing(
        "d", "disable_smoothing",
        "Disable the acceleration limiter (smoothens velocity)",
        false);

    /*
    * Called whenever the kobuki receives a data packet.
    */
    void processStreamData();

    /*
    * Catches button events
    */
    void processButtonEvent(const kobuki::ButtonEvent &event);

    /*
    * Catches bumper events and prints a message to stdout.
    */
    void processBumperEvent(const kobuki::BumperEvent &event);
    
    /*
    * Catches cliff events and prints a message to stdout.
    */
    void processCliffEvent(const kobuki::CliffEvent &event);
};
