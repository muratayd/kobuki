document.addEventListener('DOMContentLoaded', () => {
    const socket = io();
    const canvas = document.getElementById('robotMap');
    const ctx = canvas.getContext('2d');
    const startButton = document.getElementById('start');
    const stopButton = document.getElementById('stop');
    const targetXInput = document.getElementById('targetX');
    const targetYInput = document.getElementById('targetY');
    let grid = []; // Define grid at the top level of your script

    function drawGrid(grid) {
        if (!grid || !grid.length) {
            console.log("Grid is undefined or empty.");
            return; // Early return to prevent TypeError
        }
        const numRows = grid.length;
        const numCols = grid[0].length;
        const cellSize = canvas.width / numCols; // Assuming a square grid and canvas
        ctx.clearRect(0, 0, canvas.width, canvas.height); // Clear canvas before drawing
    
        for (let row = 0; row < numRows; row++) {
            for (let col = 0; col < numCols; col++) {
                ctx.fillStyle = grid[row][col] === -1 ? '#D3D3D3' : 
                                grid[row][col] === 0 ? '#FFFFFF' : 
                                '#000000';
                ctx.fillRect(col * cellSize, row * cellSize, cellSize, cellSize);
            }
        }
    }
    
    function drawRobot(x, y, heading) {
        if (!grid || !grid.length) {
            console.error("Grid data is not available. Cannot draw robot.");
            return; // Exit if grid data is not ready
        }
        const cellSize = canvas.width / grid[0].length; // Assuming grid is already defined
        x = (x / 50) + 100
        y = 100 - (y / 50)
        const centerX = x * cellSize + cellSize / 2;
        const centerY = y * cellSize + cellSize / 2;

        const radius = cellSize * 5; // Adjust size of the icon as needed
        console.log("x: ", x);
        console.log("y: ", y);
        console.log("centerX: ", centerX);
        console.log("centerY: ", centerY);
        // Draw robot as a green circle for simplicity
        ctx.fillStyle = 'green';
        ctx.beginPath();
        ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI);
        ctx.fill();

        // Optionally, draw heading indicator
        ctx.strokeStyle = 'white'; // Contrast color for the heading line
        ctx.lineWidth = 2;
        const endX = centerX + radius * Math.cos(heading * 2 * Math.PI / 360);
        const endY = centerY + radius * Math.sin(heading * 2 * Math.PI / 360);
        ctx.beginPath();
        ctx.moveTo(centerX, centerY);
        ctx.lineTo(endX, endY);
        ctx.stroke();
    }
    
    socket.on('robot position', (position) => {
        drawGrid(grid); // Redraw the grid to clear previous positions
        drawRobot(position.x, position.y, position.heading); // Draw the robot at the new position
        console.log('Received robot position:', position);
    });

    socket.on('map update', (newGrid) => {
        grid = newGrid; // Assign the received grid data to your grid variable
        drawGrid(grid); // Now safe to call drawGrid
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
