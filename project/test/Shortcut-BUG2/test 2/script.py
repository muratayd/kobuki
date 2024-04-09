import json
import pandas as pd
import os
from math import sqrt

def calculate_distance(x1, y1, x2, y2):
    return sqrt((x2 - x1) ** 2 + (y2 - y1) ** 2)

def process_data(file_path):
    robot_data = {}
    with open(file_path, 'r') as file:
        lines = file.readlines()
        for i in range(0, len(lines), 3):
            data = json.loads(lines[i + 2].strip())
            robot_id = data['robot_id']
            if robot_id not in robot_data:
                robot_data[robot_id] = {'robot_x': [], 'robot_y': [], 'robot_distance': [],
                                        'pozyx_x': [], 'pozyx_y': [], 'pozyx_distance': []}
            
            if robot_data[robot_id]['robot_x']:
                prev_robot_x = robot_data[robot_id]['robot_x'][-1]
                prev_robot_y = robot_data[robot_id]['robot_y'][-1]
                prev_pozyx_x = robot_data[robot_id]['pozyx_x'][-1]
                prev_pozyx_y = robot_data[robot_id]['pozyx_y'][-1]
                robot_dist = robot_data[robot_id]['robot_distance'][-1] + \
                             calculate_distance(prev_robot_x, prev_robot_y, data['x'], data['y'])
                pozyx_dist = robot_data[robot_id]['pozyx_distance'][-1] + \
                             calculate_distance(prev_pozyx_x, prev_pozyx_y, data['pozyx_x'], data['pozyx_y'])
            else:
                robot_dist = 0
                pozyx_dist = 0
            
            robot_data[robot_id]['robot_x'].append(data['x'])
            robot_data[robot_id]['robot_y'].append(data['y'])
            robot_data[robot_id]['robot_distance'].append(robot_dist)
            robot_data[robot_id]['pozyx_x'].append(data['pozyx_x'])
            robot_data[robot_id]['pozyx_y'].append(data['pozyx_y'])
            robot_data[robot_id]['pozyx_distance'].append(pozyx_dist)
    
    return robot_data

def create_excel(robot_data, output_file):
    max_length = max(len(values['robot_x']) for values in robot_data.values())
    data = []
    for i in range(max_length):
        row = []
        for robot_id in sorted(robot_data.keys()):
            values = robot_data[robot_id]
            if i < len(values['robot_x']):
                row.extend([values['robot_x'][i], values['robot_y'][i], values['robot_distance'][i],
                            values['pozyx_x'][i], values['pozyx_y'][i], values['pozyx_distance'][i]])
            else:
                row.extend([None, None, None, None, None, None])
        data.append(row)
    
    column_headers = []
    for robot_id in sorted(robot_data.keys()):
        column_headers.extend([f'Robot {robot_id} X', f'Robot {robot_id} Y', f'Robot {robot_id} Distance',
                               f'Robot {robot_id} Pozyx X', f'Robot {robot_id} Pozyx Y', f'Robot {robot_id} Pozyx Distance'])
    
    df = pd.DataFrame(data, columns=column_headers)
    df.to_excel(output_file, index=False)

def main():
    for folder in os.listdir('.'):
        if os.path.isdir(folder):
            file_path = os.path.join(folder, 'mqtt_data.txt')
            if os.path.exists(file_path):
                print(f'Processing {file_path}...')
                robot_data = process_data(file_path)
                output_file = os.path.join(folder, 'robot_data.xlsx')
                create_excel(robot_data, output_file)
                print(f'Excel file created: {output_file}')

if __name__ == '__main__':
    main()
