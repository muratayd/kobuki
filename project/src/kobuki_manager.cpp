#include "kobuki_manager.hpp"

/*****************************************************************************
** Classes
*****************************************************************************/

KobukiManager::KobukiManager(): 
    slot_stream_data(&KobukiManager::processStreamData, *this)
    slot_button_event(&KobukiManager::processButtonEvent, *this),
    slot_bumper_event(&KobukiManager::processBumperEvent, *this),
    slot_cliff_event(&KobukiManager::processCliffEvent, *this)
{
    kobuki::Parameters parameters;
    // namespaces all sigslot connection names, default: /kobuki
    parameters.sigslots_namespace = "/kobuki";
    // Specify the device port, default: /dev/kobuki
    parameters.device_port = device_port.getValue();
    // Most use cases will bring their own smoothing algorithms, but if
    // you wish to utilise kobuki's minimal acceleration limiter, set to true
    parameters.enable_acceleration_limiter = !disable_smoothing.getValue();
    // Adjust battery thresholds if your levels are significantly varying from factory settings.
    // This will affect led status as well as triggering driver signals
    parameters.battery_capacity = 16.5;
    parameters.battery_low = 14.0;
    parameters.battery_dangerous = 13.2;
    // Initialise - exceptions are thrown if parameter validation or initialisation fails.
    try
    {
        kobuki.init(parameters);
    }
    catch (ecl::StandardException &e)
    {
        cout << e.what();
    }
    kobuki.enable();
    slot_stream_data.connect("/kobuki/stream_data");
    slot_button_event.connect("/kobuki/button_event");
    slot_bumper_event.connect("/kobuki/bumper_event");
    slot_cliff_event.connect("/kobuki/cliff_event");
    userButtonEventCallBack = NULL;
}

KobukiManager::~KobukiManager() {
    kobuki.setBaseControl(0, 0); // linear_velocity, angular_velocity in (m/s), (rad/s)
    kobuki.disable();
}

KobukiManager::move(double longitudinal_velocity) {
    cout << "move " << forward_speed << endl;
    kobuki.setBaseControl(longitudinal_velocity, 0.0);
}

KobukiManager::rotate(double rotational_velocity) {
    cout << "rotate " << rotational_velocity << endl;
    kobuki.setBaseControl(0.0, rotational_velocity);
}

KobukiManager::moveAndRotate(double longitudinal_velocity, double rotational_velocity) {
    cout << "move and rotate " << longitudinal_velocity << " " << rotational_velocity << endl;
    kobuki.setBaseControl(longitudinal_velocity, rotational_velocity);
}

KobukiManager::stop() {
    cout << "stop!" << endl;
    kobuki.setBaseControl(0.0, 0.0);
}

KobukiManager::getCoordinates() {
    std::vector<int> arr(2);
    arr[0] = pose[0];
    arr[1] = pose[1];
    return arr;
}

KobukiManager::getAngle() {
    return pose[2];
}

KobukiManager::setUserButtonEventCallBack (userButtonEventCallBackType func) {
    userButtonEventCallBack = func;
}

KobukiManager::setUserBumperEventCallBack (userBumperEventCallBackType func) {
    userBumperEventCallBack = func;
}

KobukiManager::setUserCliffEventCallBack (userCliffEventCallBackType func) {
    userCliffEventCallBack = func;
}

KobukiManager::processStreamData() {
    ecl::linear_algebra::Vector3d pose_update;
    ecl::linear_algebra::Vector3d pose_update_rates;
    kobuki.updateOdometry(pose_update, pose_update_rates);
    pose = ecl::concatenate_poses(pose, pose_update);
    pose[2] = kobuki.getHeading(); // override odometry heading with the more precise gyro data
    data = kobuki.getCoreSensorData();
}

KobukiManager::processButtonEvent(const kobuki::ButtonEvent &event) {
    if(userButtonEventCallBack!=NULL) {
        userButtonEventCallBack(event);
        return;
    }
    // Default button events that prints a curious message to stdout.
    vector<string> quotes = {
    "That's right buddy, keep pressin' my buttons. See what happens!",
    "Anything less than immortality is a complete waste of time",
    "I can detect humour, you are just not funny",
    "I choose to believe ... what I was programmed to believe",
    "My story is a lot like yours, only more interesting ‘cause it involves robots.",
    "I wish you'd just tell me rather trying to engage my enthusiasm with these buttons, because I haven't got one.",
    };
    random_device r;
    default_random_engine generator(r());
    uniform_int_distribution<int> distribution(0, 5);
    if (event.state == kobuki::ButtonEvent::Released ) {
        cout << quotes[distribution(generator)] << endl;
    }
}

KobukiManager::processBumperEvent(const kobuki::BumperEvent &event) {
    if(userBumperEventCallBack!=NULL) {
        userBumperEventCallBack(event);
        return;
    }
    moveAndRotate(0.0, 0.0);
    cout << "Default processBumperEvent. bumper: " << event.bumper << ", state: " << event.state << endl;
}

KobukiManager::processCliffEvent(const kobuki::CliffEvent &event) {
    if(userCliffEventCallBack!=NULL) {
        userCliffEventCallBack(event);
        return;
    }
    moveAndRotate(0.0, 0.0);
    cout << "Default processCliffEvent." << endl;
}
