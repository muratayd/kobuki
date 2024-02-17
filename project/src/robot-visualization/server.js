const express = require('express');
const mqtt = require('mqtt');
const socketIo = require('socket.io');
const http = require('http');

const app = express();
const server = http.createServer(app);
const io = socketIo(server);
const mqttClient = mqtt.connect('mqtt://localhost'); // Replace with your MQTT broker

app.use(express.static('public'));

// MQTT Topics
const controlTopic = 'robot/control'; // Topic for start/stop commands
const targetTopic = 'robot/target'; // Topic for sending target position
const mapTopic = 'robot/map'; // Topic for sending target position
const positionTopic = 'pozyx/position'; // Topic for receiving robot position

mqttClient.on('connect', () => {
    console.log('Connected to MQTT broker');
    mqttClient.subscribe(positionTopic, (err) => {
        if (!err) {
        console.log('Subscribed to "pozyx/position"');
        }
    });
    mqttClient.subscribe(mapTopic, (err) => {
        if (!err) {
        console.log('Subscribed to "robot/map"');
        }
    });
});

mqttClient.on('message', (topic, message) => {
    if (topic === positionTopic) {
        //console.log('Received "pozyx/position"');
        const positionData = JSON.parse(message.toString()); // Assuming the message is a JSON string
        io.emit('robot position', positionData); // Emit to all connected clients
    } else if (topic === mapTopic) {
        console.log('Received "robot/map"');
        // Assuming the message contains a flattened array of integers
        const data = new Int32Array(message.buffer);

        // Assuming the 2D array was originally a 200x200 map
        const map = [];
        for (let i = 0; i < 200; i++) {
            map.push(Array.from(data.slice(i * 200, (i + 1) * 200)));
        }
        // console.log('Received map update:', map);

        io.emit('map update', map); // Emit to all connected clients
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
});

io.on('error', function (err) {
    console.error('Error:', err);
});

server.listen(3000, () => {
  console.log('Server is running on http://localhost:3000');
});
