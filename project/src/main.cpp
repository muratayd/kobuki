//#include <string>
#include <fstream>
#include <sstream>
#include <csignal>
#include <iomanip>
#include "kobuki_manager.hpp"
#include "map_manager.hpp"

using namespace std;

bool shutdown_req = false;
KobukiManager kobuki_manager;

void signalHandler(int /* signum */)
{
    shutdown_req = true;
}

void exampleCliffHandlerPrint(const kobuki::ButtonEvent &event) {
    std::cout << "HELLO CLIFF" << std::endl;
}

void exampleButtonHandler(const kobuki::ButtonEvent &event) {
    std::cout << "exampleButtonHandler" << std::endl;
    if (event.state == kobuki::ButtonEvent::Released) {
        if (event.button == kobuki::ButtonEvent::Button0) {
            kobuki_manager.rotate(0.5);
        } else if (event.button == kobuki::ButtonEvent::Button1) {
            kobuki_manager.playSoundSequence(0x5);
        }
    }
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
    MapManager map_manager;
    std::cout << "m1" << std::endl;
    map_manager.updateMap(0,0,0.0,1);
        std::cout << "m2" << std::endl;

    map_manager.checkMap(0,0,0.0);
    std::cout << "m3" << std::endl;

    ecl::MilliSleep sleep(1);
    //kobuki_manager.setUserButtonEventCallBack(examplePrint);
    kobuki_manager.setUserButtonEventCallBack(exampleButtonHandler);
    //kobuki_manager.setUserCliffEventCallBack(exampleCliffHandlerPrint);

    try
    {
        while (!shutdown_req)
        {
            vector<double> arr = kobuki_manager.getCoordinates();
            std::cout << "TimeStamp:" << ecl::TimeStamp() << endl << "x: " << arr[0] << " y: " << arr[1];
            std::cout << " angle: " << kobuki_manager.getAngle() << endl;
            cout << "example getbumper: " << kobuki_manager.getBumperState() << endl;
            //kobuki_manager.playSoundSequence(0x6);
            kobuki_manager.move(0.03);
            sleep(5000);
        }
    }
    catch (ecl::StandardException &e)
    {
        std::cout << e.what();
    }

    sleep(300);

    return 0;
}