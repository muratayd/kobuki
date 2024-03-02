document.addEventListener('DOMContentLoaded', () => {
    const socket = io();
    const canvas = document.getElementById('robotMap');
    const ctx = canvas.getContext('2d');
    const robotImg = document.getElementById('robot');
    const startButton = document.getElementById('start');
    const stopButton = document.getElementById('stop');
    const stop2Button = document.getElementById('stop2');
    const moveButton = document.getElementById('move');
    const targetXInput = document.getElementById('targetX');
    const targetYInput = document.getElementById('targetY');
    const longitudinalInput = document.getElementById('longitudinal_velocity');
    const rotationalInput = document.getElementById('rotational_velocity');
    let grid = []; // Define grid at the top level of your script
    let targetPosition = { x: 0, y: 0 }; // Initial target position
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
                    grid[row][col] === 0 ? '#FFFFFF' : '#FF0000';
                ctx.fillRect(col * cellSize, row * cellSize, cellSize, cellSize);
            }
        }
        // Draw target icon
        ctx.fillStyle = '#00FF00'; // Green color for the target icon
        ctx.fillRect(targetPosition.x * cellSize, targetPosition.y * cellSize, cellSize, cellSize);
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

    function updateRobotInfoMode(mode) {
        const modes = ["CUSTOM MODE", "GO TO GOAL MODE", "WALL FOLLOWING MODE"];
        modes.forEach((m, index) => {
            const modeElement = document.getElementById(`mode-${index}`);
            if (m === mode) {
                modeElement.classList.add('active');
            } else {
                modeElement.classList.remove('active');
            }
        });
    }

    function updateRobotInfoState(state) {
        const states = ["ADJUST HEADING", "GO STRAIGHT", "GOAL ACHIEVED"];
        states.forEach((s, index) => {
            const stateElement = document.getElementById(`state-${index}`);
            if (s === state) {
                stateElement.classList.add('active');
            } else {
                stateElement.classList.remove('active');
            }
        });
    }

    function updateTargetIcon(x, y) {
        const cellSize = canvas.width / grid[0].length; // Assuming grid is already defined
        x = (x * 1000 / GRID_SIZE) + MAP_ORIGIN;
        y = MAP_ORIGIN - (y * 1000 / GRID_SIZE);
        const centerX = (x + 0.5) * cellSize;
        const centerY = (y + 0.5) * cellSize;
        const targetIcon = document.querySelector('.targetIcon');
        targetIcon.style.left = centerX + 'px';
        targetIcon.style.top = centerY + 'px';
    }

    function updateSensorState(sensorType, sensor, state) {
        const sensorElement = document.getElementById(`${sensorType}${sensor}`);
        if (!sensorElement) return; // Sensor element not found, return early
        console.log('sensorElement', sensorElement);
        sensorElement.classList.remove('pressed', 'released', 'floor', 'cliff');
        sensorElement.classList.add(state.toLowerCase());
    }

    socket.on('robot position', (position) => {
        drawRobot(position.x, position.y, position.heading); // Draw the robot at the new position
        //console.log('Received robot position:', position);
    });

    socket.on('map update', (newGrid) => {
        console.log('Received map');
        grid = newGrid; // Assign the received grid data to your grid variable
        drawGrid(); // Now safe to call drawGrid
    });

    socket.on('robot mode', (mode) => {
        updateRobotInfoMode(mode); // Update the robot mode display
    });

    socket.on('moving state', (state) => {
        updateRobotInfoState(state); // Update the moving state display
    });

    socket.on('bumper state', (data) => {
        updateSensorState('bumper', data.bumper, data.state);
        console.log('Received bumper state:', data);
    });

    socket.on('cliff state', (data) => {
        updateSensorState('cliff', data.sensor, data.state);
        console.log('Received cliff state:', data);
    });

    startButton.addEventListener('click', () => {
        const x = parseFloat(targetXInput.value); // Convert input value to float
        const y = parseFloat(targetYInput.value);
        const message = { x, y }; // Create the message object
        updateTargetIcon(x, y);
        socket.emit('start command', message); // Emit the message object to the server
    });

    stopButton.addEventListener('click', () => {
        socket.emit('stop command'); // Emit a stop command event without additional data
    });

    stop2Button.addEventListener('click', () => {
        socket.emit('stop command'); // Emit a stop command event without additional data
    });

    moveButton.addEventListener('click', () => {
        const longitudinal_velocity = parseFloat(longitudinalInput.value); // Convert input value to float
        const rotational_velocity = parseFloat(rotationalInput.value);
        const message = { longitudinal_velocity, rotational_velocity }; // Create the message object
        socket.emit('move command', message); // Emit the message object to the server
    });
});
