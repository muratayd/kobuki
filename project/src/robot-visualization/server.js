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

app.use(express.static('public'));

mqttClient.on('connect', () => {
    console.log('Connected to MQTT broker');
    mqttClient.subscribe('pozyx/position', (err) => {
        if (!err) {
        console.log('Subscribed to "pozyx/position"');
        }
    });
});

mqttClient.on('message', (topic, message) => {
    if (topic === 'pozyx/position') {
        console.log('received pozyx/position', message.toString());

        const positionData = JSON.parse(message.toString()); // Assuming the message is a JSON string
        io.emit('robot position', positionData); // Emit to all connected clients
    }
});
  

io.on('connection', (socket) => {
  console.log('A user connected');
  socket.on('start command', (data) => {
    const message = JSON.stringify(data); // Convert the data object to a JSON string
    mqttClient.publish('robot/target', message);
    console.log('Publishing start command:', message);
  });

  socket.on('stop command', () => {
    mqttClient.publish('robot/control', 'STOP');
    console.log('Publishing stop command');
  });

  // Send initial grid map
  const gridMap = [];
  fs.createReadStream('../grid_map.csv')
    .pipe(csv({ headers: false }))
    .on('data', (data) => {
      gridMap.push(Object.values(data).map(Number));
    })
    .on('end', () => {
      socket.emit('map update', gridMap);
    });
});

server.listen(3000, () => {
  console.log('Server is running on http://localhost:3000');
});
