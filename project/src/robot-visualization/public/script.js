document.addEventListener('DOMContentLoaded', () => {
    const socket = io();
    const canvas = document.getElementById('robotMap');
    const ctx = canvas.getContext('2d');
    const robotImg = document.getElementById('robot');
    const startButton = document.getElementById('start');
    const stopButton = document.getElementById('stop');
    const targetXInput = document.getElementById('targetX');
    const targetYInput = document.getElementById('targetY');
    let grid = []; // Define grid at the top level of your script
    let robotPosition = { x: 0, y: 0, heading: 0 }; // Initial robot position and heading
    const MAP_SIZE = 200; // n of cells
    const MAP_ORIGIN = 100; // origin point is at [100][100]
    const GRID_SIZE  = 50 // mm

    function drawGrid() {
        if (!grid || !grid.length) {
            console.log("Grid is undefined or empty.");
            return; // Early return to prevent TypeError
        }

        const numRows = grid.length;
        const numCols = grid[0].length;
        const cellSize = canvas.width / numCols; // Assuming a square grid and canvas
        console.log('cellSize ', cellSize , numRows, numCols);

        ctx.clearRect(0, 0, canvas.width, canvas.height); // Clear canvas before drawing

        for (let row = 0; row < numRows; row++) {
            for (let col = 0; col < numCols; col++) {
                ctx.fillStyle = grid[row][col] === -1 ? '#D3D3D3' :
                    grid[row][col] === 0 ? '#FFFFFF' : '#000000';
                ctx.fillRect(col * cellSize, row * cellSize, cellSize, cellSize);
            }
        }
    }

    function drawRobot(x, y, heading) {
        const cellSize = canvas.width / grid[0].length; // Assuming grid is already defined
        x = (x / GRID_SIZE) + MAP_ORIGIN;
        y = MAP_ORIGIN - (y / GRID_SIZE);
        const centerX = (x + 0.5) * cellSize;
        const centerY = (y + 0.5) * cellSize;

        const imageSize = cellSize * 4; // Adjust size of the image as needed

        // Update the position of the robot image
        robotImg.style.left = (centerX - imageSize / 2) + 'px';
        robotImg.style.top = (centerY - imageSize / 2) + 'px';

        // Optionally, update the rotation of the image based on the heading
        robotImg.style.transform = 'rotate(' + heading + 'deg)';
    }

    socket.on('robot position', (position) => {
        robotPosition = position; // Update robot position
        drawGrid(); // Redraw the grid to clear previous positions
        drawRobot(robotPosition.x, robotPosition.y, robotPosition.heading); // Draw the robot at the new position
        console.log('Received robot position:', position);
    });

    socket.on('map update', (newGrid) => {
        console.log('Received map');
        grid = newGrid; // Assign the received grid data to your grid variable
        drawGrid(); // Now safe to call drawGrid
    });

    startButton.addEventListener('click', () => {
        const x = parseFloat(targetXInput.value); // Convert input value to float
        const y = parseFloat(targetYInput.value);
        const message = { x, y }; // Create the message object
        socket.emit('start command', message); // Emit the message object to the server
    });

    stopButton.addEventListener('click', () => {
        socket.emit('stop command'); // Emit a stop command event without additional data
    });
});
