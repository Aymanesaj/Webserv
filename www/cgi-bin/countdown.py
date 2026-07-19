#!/usr/bin/env python3

import time

print("Content-Type: text/html\n")

print("""
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta http-equiv="refresh" content="1">
<title>Countdown</title>

<style>
    body {
        margin: 0;
        height: 100vh;
        display: flex;
        justify-content: center;
        align-items: center;
        background: radial-gradient(circle at top, #1e1e2f, #0a0a12);
        font-family: Arial, sans-serif;
        color: white;
    }

    .container {
        text-align: center;
    }

    .title {
        font-size: 20px;
        letter-spacing: 4px;
        opacity: 0.6;
    }

    .number {
        font-size: 120px;
        font-weight: bold;
        background: linear-gradient(90deg, #7f5af0, #2cb67d);
        -webkit-background-clip: text;
        -webkit-text-fill-color: transparent;
        margin: 20px 0;
    }

    .bar {
        width: 300px;
        height: 6px;
        background: #222;
        border-radius: 10px;
        overflow: hidden;
        margin: auto;
    }

    .progress {
        height: 100%;
        background: linear-gradient(90deg, #7f5af0, #2cb67d);
        width: 0%;
        animation: load 10s linear forwards;
    }

    @keyframes load {
        from { width: 100%; }
        to { width: 0%; }
    }

    .done {
        font-size: 24px;
        margin-top: 20px;
        opacity: 0.8;
    }
</style>

</head>
<body>
<div class="container">
""")

target = 10
current = int(time.time()) % (target + 1)
remaining = target - current

print(f'<div class="title">COUNTDOWN</div>')
print(f'<div class="number">{remaining}</div>')

print("""
<div class="bar">
    <div class="progress"></div>
</div>
""")

if remaining == 0:
    print('<div class="done">DONE</div>')

print("""
</div>
</body>
</html>
""")