#ifndef ULTRASONIC_SENSOR_HPP
#define ULTRASONIC_SENSOR_HPP

#include <gpiod.h>
#include <chrono>

class UltrasonicSensor {
public:
    UltrasonicSensor(int triggerPin, int echoPin);
    ~UltrasonicSensor();
    double getDistance();

private:
    int triggerPin;
    int echoPin;
    struct gpiod_chip* gpiod_chip;
    struct gpiod_line* triggerLine;
    struct gpiod_line* echoLine;
};

#endif // ULTRASONIC_SENSOR_HPP
