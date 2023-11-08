# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11

# Libraries
LIBS = -lgpiod

# Executable name
TARGET = your_program

# Source files
SRC = your_source_file.cpp UltrasonicSensor.cpp

# Object files
OBJ = $(SRC:.cpp=.o)

# Build the executable
$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

# Clean up
clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean

all: $(TARGET)
