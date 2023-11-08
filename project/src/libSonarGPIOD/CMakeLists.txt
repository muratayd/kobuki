cmake_minimum_required(VERSION 3.0)
project(UltrasonicSensorExample)

# Set the C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)

# Find the libgpiod library
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGPIOD REQUIRED libgpiod)

# Add the Ultrasonic Sensor library source files
add_library(UltrasonicSensor
    UltrasonicSensor.cpp
)

# Include directories for the Ultrasonic Sensor library and libgpiod
target_include_directories(UltrasonicSensor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIBGPIOD_INCLUDE_DIRS}
)

# Create the executable for your application
add_executable(UltrasonicSensorApp
    example_ultrasonic_sensor_app.cpp
)

# Link the executable with the Ultrasonic Sensor library and libgpiod
target_link_libraries(UltrasonicSensorApp
    UltrasonicSensor
    ${LIBGPIOD_LIBRARIES}
)

# Optionally, you can set the output directory for the executable
# set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Set the output directory for the executable to a specific location
# You can uncomment and modify the following line to specify your desired directory.
# set_target_properties(UltrasonicSensorApp PROPERTIES RUNTIME_OUTPUT_DIRECTORY /path/to/your/output/directory)

# You can add additional CMake configuration, such as installing targets or files,
# as needed for your project.
