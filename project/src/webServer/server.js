const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const fs = require('fs');
const Papa = require('papaparse');

const app = express();
const server = http.createServer(app);
const io = socketIo(server);

app.use(express.static('public'));

const readFile = () => {
  const file = fs.readFileSync('../grid_map.csv', 'utf8');
  return Papa.parse(file, {
    delimiter: ',',
    dynamicTyping: true,
    skipEmptyLines: true
  }).data;
};

io.on('connection', (socket) => {
  console.log('New client connected');
  socket.emit('gridData', readFile());

  socket.on('disconnect', () => {
    console.log('Client disconnected');
  });
});

setInterval(() => {
  io.emit('gridData', readFile());
}, 5000); // Update interval in milliseconds

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => console.log(`Server running on port ${PORT}`));
