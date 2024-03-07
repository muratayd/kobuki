import json
from openpyxl import Workbook

# Read the data from mqtt_data.txt
with open('mqtt_data.txt', 'r') as file:
    lines = file.readlines()

# Create a new Excel workbook and select the active sheet
wb = Workbook()
ws = wb.active
ws.append(["Pozyx X", "Pozyx Y", "Robot X", "Robot Y"])

# Initialize variables to store the x and y data
pozyx_x, pozyx_y = None, None
robot_x, robot_y = None, None

# Iterate over the lines in the file
for i in range(len(lines)):
    if lines[i].startswith("Topic: pozyx/position"):
        pozyx_data = json.loads(lines[i + 1])
        pozyx_x = pozyx_data.get('x')
        pozyx_y = pozyx_data.get('y')
    elif lines[i].startswith("Topic: robot/coordinates"):
        robot_data = json.loads(lines[i + 1])
        robot_x = robot_data.get('x')
        robot_y = robot_data.get('y')
        
        # Write the data to the Excel sheet
        ws.append([pozyx_x, pozyx_y, robot_x, robot_y])

# Save the workbook
wb.save("data.xlsx")
