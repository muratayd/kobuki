const express = require('express');
const fs = require('fs');
const csv = require('csv-parser');
const mqtt = require('mqtt');
const socketIo = require('socket.io');
const http = require('http');

const app = express();
const server = http.createServer(app);
const io = socketIo(server);
const mqttClient = mqtt.connect('mqtt://localhost'); // Replace with your MQTT broker
const csvFilePath = '../grid_map.csv'; // Path to your CSV file

app.use(express.static('public'));

// Throttling control
let lastUpdateTime = 0;
const updateFrequency = 1000; // Minimum time between updates in milliseconds (1 second)

// MQTT Topics
const controlTopic = 'robot/control'; // Topic for start/stop commands
const targetTopic = 'robot/target'; // Topic for sending target position
const positionTopic = 'pozyx/position'; // Topic for receiving robot position

// Function to read and emit the CSV file content
function readAndEmitCSV() {
    const gridMap = [];
    fs.createReadStream(csvFilePath)
        .pipe(csv({ headers: false }))
        .on('data', (data) => gridMap.push(Object.values(data).map(Number)))
        .on('end', () => {
            const now = Date.now();
            if (now - lastUpdateTime >= updateFrequency) {
                io.emit('map update', gridMap);
                lastUpdateTime = now; // Update the last update time
            }
        });
}

// Watch the CSV file for changes and update the grid map accordingly
fs.watch(csvFilePath, (eventType) => {
    if (eventType === 'change') {
        console.log('grid_map.csv has been changed, updating grid map...');
        readAndEmitCSV();
    }
});

mqttClient.on('connect', () => {
    console.log('Connected to MQTT broker');
    mqttClient.subscribe(positionTopic, (err) => {
        if (!err) {
        console.log('Subscribed to "pozyx/position"');
        }
    });
});

mqttClient.on('message', (topic, message) => {
    if (topic === positionTopic) {
        const positionData = JSON.parse(message.toString()); // Assuming the message is a JSON string
        io.emit('robot position', positionData); // Emit to all connected clients
    }
});

// Socket.IO events
io.on('connection', (socket) => {
    console.log('A client connected');

    // Handle 'start command' from clients
    socket.on('start command', (data) => {
        const message = JSON.stringify(data);
        mqttClient.publish(targetTopic, message);
        console.log(`Publishing start command to ${targetTopic}:`, message);
    });

    // Handle 'stop command' from clients
    socket.on('stop command', () => {
        mqttClient.publish(controlTopic, 'STOP');
        console.log(`Publishing stop command to ${controlTopic}`);
    });

    // Initial CSV read and emit upon new client connection
    readAndEmitCSV();
});

server.listen(3000, () => {
  console.log('Server is running on http://localhost:3000');
});
