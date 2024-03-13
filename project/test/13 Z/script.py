import json
from openpyxl import Workbook
import math

# Function to calculate the distance between two points
def distance(x1, y1, x2, y2):
    return math.sqrt((x2 - x1)**2 + (y2 - y1)**2)

# Function to calculate the closest distance from a point to a line segment
def closest_distance_to_segment(x, y, x1, y1, x2, y2):
    dx = x2 - x1
    dy = y2 - y1
    if dx == 0 and dy == 0:
        return distance(x, y, x1, y1)
    t = ((x - x1) * dx + (y - y1) * dy) / (dx**2 + dy**2)
    if t < 0:
        closest_x, closest_y = x1, y1
    elif t > 1:
        closest_x, closest_y = x2, y2
    else:
        closest_x, closest_y = x1 + t * dx, y1 + t * dy
    return distance(x, y, closest_x, closest_y)

# Read the data from mqtt_data.txt
with open('mqtt_data.txt', 'r') as file:
    lines = file.readlines()

# Read the uwb_segments from the JSON file
with open('line_segments.json', 'r') as file:
    uwb_segments_data = json.load(file)
    uwb_segments = uwb_segments_data.get('segments')
    print(uwb_segments)
    for x1, y1, x2, y2 in uwb_segments:
        print(distance(x1, y1, x2, y2) )
# Calculate the total length of line segments
total_length = sum(distance(x1, y1, x2, y2) for x1, y1, x2, y2 in uwb_segments)
print(f"Total length of line segments: {total_length}")

# Create a new Excel workbook and select the active sheet
wb = Workbook()
ws = wb.active
ws.append(["Pozyx X", "Pozyx Y", "UWB Error", "Robot X", "Robot Y", "Odometry Error", "Combined (80% Odometry, 20% UWB)",
           "Combined (20% Odometry, 80% UWB)", "Combined (50% Odometry, 50% UWB)"])

# Initialize variables to store the x and y data
pozyx_x, pozyx_y = None, None
robot_x, robot_y = None, None

# Iterate over the lines in the file
i = 0
while i < len(lines):
    if lines[i].startswith("Topic: pozyx/position"):
        pozyx_data = json.loads(lines[i + 1])
        pozyx_x = pozyx_data.get('x')
        pozyx_y = pozyx_data.get('y')
        closest_distance_uwb = min(closest_distance_to_segment(pozyx_x, pozyx_y, *segment) for segment in uwb_segments)
        i += 2  # Skip the next line which contains the data
    elif lines[i].startswith("Topic: robot/coordinates"):
        robot_data = json.loads(lines[i + 1])
        robot_x = robot_data.get('x')
        robot_y = robot_data.get('y')
        closest_distance_odometry = min(closest_distance_to_segment(robot_x, robot_y, *segment) for segment in uwb_segments)
        i += 2  # Skip the next line which contains the data
        # Calculate the combined distances
        combined_distance_80_20 = 0.8 * closest_distance_odometry + 0.2 * closest_distance_uwb
        combined_distance_20_80 = 0.2 * closest_distance_odometry + 0.8 * closest_distance_uwb
        combined_distance_50_50 = 0.5 * closest_distance_odometry + 0.5 * closest_distance_uwb
        # Write the data to the Excel sheet
        ws.append([pozyx_x, pozyx_y, closest_distance_uwb, robot_x, robot_y, closest_distance_odometry,
                   combined_distance_80_20, combined_distance_20_80, combined_distance_50_50])
    else:
        i += 1  # Move to the next line

# Save the workbook
wb.save("data_with_distances.xlsx")
