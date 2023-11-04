//#include <string>
#include <fstream>
#include <sstream>
#include "kobuki_manager.hpp"

bool shutdown_req = false;
void signalHandler(int /* signum */)
{
    shutdown_req = true;
}

void examplePrint(const kobuki::ButtonEvent &event) {
    std::cout << "examplePrint: HELLO" << std::endl;
}

/*****************************************************************************
** Main
*****************************************************************************/

int main(int argc, char **argv)
{
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

    KobukiManager kobuki_manager;

    ecl::MilliSleep sleep(1);
    kobuki_manager.setUserButtonCallBack(examplePrint);

    try
    {
        while (!shutdown_req)
        {
            sleep(200);
            cout << "angle:" << kobuki_manager.getAngle() << endl;
            cout << "x,y: " << kobuki_manager.getCoordinates() << endl;
        }
    }
    catch (ecl::StandardException &e)
    {
        std::cout << e.what();
    }

    sleep(300);
    kobuki_manager.print(true);

    return 0;
}