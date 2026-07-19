#!/usr/bin/env python3

print("Content-Type: text/html\n")

print("""
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Neon Snake FX</title>

<style>
body {
    margin: 0;
    height: 100vh;
    display: flex;
    justify-content: center;
    align-items: center;
    background: radial-gradient(circle at top, #0b1020, #020617);
    font-family: system-ui;
    overflow: hidden;
    color: white;
}

.container {
    text-align: center;
    position: relative;
}

canvas {
    background: #0f172a;
    border-radius: 16px;
    box-shadow: 0 0 30px rgba(34,211,238,0.25);
}

.score {
    margin-top: 10px;
}

button {
    margin-top: 10px;
    padding: 10px 16px;
    border: none;
    border-radius: 10px;
    background: linear-gradient(90deg, #22c55e, #06b6d4);
    cursor: pointer;
    font-weight: bold;
}
</style>
</head>

<body>

<div class="container">
<h2>NEON SNAKE</h2>

<canvas id="game" width="420" height="420"></canvas>

<div class="score">Score: <span id="score">0</span></div>

<button onclick="resetGame()">Restart</button>
</div>

<script>
const canvas = document.getElementById("game");
const ctx = canvas.getContext("2d");

const grid = 21;
const cell = canvas.width / grid;

let snake = [{x: 10, y: 10}];
let dir = {x: 1, y: 0};
let food = spawnFood();
let score = 0;

/* ---------------- PARTICLES ---------------- */
let particles = [];

function spawnParticles(x, y) {
    for (let i = 0; i < 18; i++) {
        particles.push({
            x: x,
            y: y,
            vx: (Math.random() - 0.5) * 4,
            vy: (Math.random() - 0.5) * 4,
            life: 1
        });
    }
}

function updateParticles() {
    for (let p of particles) {
        p.x += p.vx;
        p.y += p.vy;
        p.vx *= 0.96;
        p.vy *= 0.96;
        p.life -= 0.03;
    }
    particles = particles.filter(p => p.life > 0);
}

function drawParticles() {
    ctx.shadowBlur = 20;
    ctx.shadowColor = "#22c55e";

    for (let p of particles) {
        ctx.globalAlpha = p.life;
        ctx.fillStyle = "#22c55e";
        ctx.fillRect(p.x, p.y, 4, 4);
    }

    ctx.globalAlpha = 1;
    ctx.shadowBlur = 0;
}

/* ---------------- GAME ---------------- */

function spawnFood() {
    return {
        x: Math.floor(Math.random() * grid),
        y: Math.floor(Math.random() * grid)
    };
}

document.addEventListener("keydown", e => {
    if (e.key === "ArrowUp" && dir.y === 0) dir = {x: 0, y: -1};
    if (e.key === "ArrowDown" && dir.y === 0) dir = {x: 0, y: 1};
    if (e.key === "ArrowLeft" && dir.x === 0) dir = {x: -1, y: 0};
    if (e.key === "ArrowRight" && dir.x === 0) dir = {x: 1, y: 0};
});

function update() {
    const head = {
        x: snake[0].x + dir.x,
        y: snake[0].y + dir.y
    };

    if (
        head.x < 0 || head.y < 0 ||
        head.x >= grid || head.y >= grid ||
        snake.some(s => s.x === head.x && s.y === head.y)
    ) {
        alert("Game Over: " + score);
        resetGame();
        return;
    }

    snake.unshift(head);

    if (head.x === food.x && head.y === food.y) {
        score++;
        document.getElementById("score").innerText = score;

        // PARTICLE BURST (center of cell)
        spawnParticles(
            food.x * cell + cell / 2,
            food.y * cell + cell / 2
        );

        food = spawnFood();
    } else {
        snake.pop();
    }

    updateParticles();
}

function draw() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    /* food glow */
    ctx.shadowBlur = 25;
    ctx.shadowColor = "#ff3b3b";
    ctx.fillStyle = "#ff3b3b";
    ctx.beginPath();
    ctx.arc(
        food.x * cell + cell/2,
        food.y * cell + cell/2,
        cell/3,
        0,
        Math.PI * 2
    );
    ctx.fill();

    /* snake */
    for (let i = 0; i < snake.length; i++) {
        const p = snake[i];
        const alpha = 1 - i * 0.03;

        ctx.shadowBlur = 15;
        ctx.shadowColor = "#22c55e";

        ctx.fillStyle = `rgba(34,197,94,${alpha})`;
        ctx.fillRect(
            p.x * cell + 1,
            p.y * cell + 1,
            cell - 2,
            cell - 2
        );
    }

    drawParticles();
}

function loop() {
    update();
    draw();
}

setInterval(loop, 110);

function resetGame() {
    snake = [{x: 10, y: 10}];
    dir = {x: 1, y: 0};
    score = 0;
    document.getElementById("score").innerText = 0;
    food = spawnFood();
    particles = [];
}
</script>

</body>
</html>
""")