#include "UltrasonicSensor.hpp"
#include <iostream>
#include <thread>

UltrasonicSensor::UltrasonicSensor(int triggerPin, int echoPin) : triggerPin(triggerPin), echoPin(echoPin) {
    gpiod_chip = gpiod_chip_open("/dev/gpiochip0");
    if (!gpiod_chip) {
        std::cerr << "Failed to open GPIO chip" << std::endl;
        return;
    }
    triggerLine = gpiod_chip_get_line(gpiod_chip, triggerPin);
    echoLine = gpiod_chip_get_line(gpiod_chip, echoPin);

    if (gpiod_line_request_output(triggerLine, "trigger", 0) < 0) {
        std::cerr << "Failed to request trigger line" << std::endl;
        return;
    }
    if (gpiod_line_request_input(echoLine, "echo") < 0) {
        std::cerr << "Failed to request echo line" << std::endl;
        return;
    }
}

UltrasonicSensor::~UltrasonicSensor() {
    gpiod_line_release(triggerLine);
    gpiod_line_release(echoLine);
    gpiod_chip_close(gpiod_chip);
}

double UltrasonicSensor::getDistance() {
    gpiod_line_set_value(triggerLine, 1);
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    gpiod_line_set_value(triggerLine, 0);

    struct gpiod_line_event event;
    if (gpiod_line_event_wait(echoLine, &event, nullptr) < 0) {
        std::cerr << "Failed to wait for echo line event" << std::endl;
        return -1.0;
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    if (gpiod_line_event_wait(echoLine, &event, nullptr) < 0) {
        std::cerr << "Failed to wait for echo line event" << std::endl;
        return -1.0;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto pulse_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    double distance = pulse_duration * 0.034 / 2;
    return distance;
}
