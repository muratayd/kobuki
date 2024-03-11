import json
from openpyxl import Workbook
import math

# Function to calculate the distance between two points
def distance(x1, y1, x2, y2):
    return math.sqrt((x2 - x1)**2 + (y2 - y1)**2)

# Function to calculate the closest distance from a point to a circle
def closest_distance_to_circle(x, y, cx, cy, radius):
    dx = x - cx
    dy = y - cy
    if dx == 0 and dy == 0:
        return abs(radius)
    dist = math.sqrt(dx**2 + dy**2)
    return abs(dist - radius)

# Read the data from mqtt_data.txt
with open('mqtt_data.txt', 'r') as file:
    lines = file.readlines()

# Create a new Excel workbook and select the active sheet
wb = Workbook()
ws = wb.active
ws.append(["Pozyx X", "Pozyx Y", "UWB Closest Distance to Circle", "Robot X", "Robot Y", "Odometry Closest Distance to Circle", "Combined"])

# Initialize variables to store the x and y data
pozyx_x, pozyx_y = None, None
robot_x, robot_y = None, None

# Define the circle center and radius
circle_center = (-150, -75)
circle_radius = 1000

# Iterate over the lines in the file
i = 0
while i < len(lines):
    if lines[i].startswith("Topic: pozyx/position"):
        pozyx_data = json.loads(lines[i + 1])
        pozyx_x = pozyx_data.get('x')
        pozyx_y = pozyx_data.get('y')
        closest_distance_uwb = closest_distance_to_circle(pozyx_x, pozyx_y, *circle_center, circle_radius)
        i += 2  # Skip the next line which contains the data
    elif lines[i].startswith("Topic: robot/coordinates"):
        robot_data = json.loads(lines[i + 1])
        robot_x = robot_data.get('x')
        robot_y = robot_data.get('y')
        closest_distance_odometry = closest_distance_to_circle(robot_x, robot_y, *circle_center, circle_radius)
        i += 2  # Skip the next line which contains the data
        # Calculate the combined distance (80% odometry, 20% UWB)
        combined_distance = 0.8 * closest_distance_odometry + 0.2 * closest_distance_uwb
        # Write the data to the Excel sheet
        ws.append([pozyx_x, pozyx_y, closest_distance_uwb, robot_x, robot_y, closest_distance_odometry, combined_distance])

    else:
        i += 1  # Move to the next line

# Save the workbook
wb.save("data_with_circle_distances.xlsx")
