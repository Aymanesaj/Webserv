#!/usr/bin/env python3

import sys
import os
import time
from urllib.parse import parse_qs

def read_post():
    length = int(os.environ.get("CONTENT_LENGTH", 0))
    return sys.stdin.read(length) if length > 0 else ""

data = parse_qs(read_post())

def get(k, default=""):
    return data.get(k, [default])[0]

action = get("action")
start = int(get("start", "0"))
paused = int(get("paused", "0"))
duration = int(get("duration", "30"))

now = int(time.time())

if action == "start":
    if start == 0:
        start = now
    else:
        start = now - paused
    paused = 0

elif action == "pause":
    if start:
        paused = now - start
        start = 0

elif action == "reset":
    start = 0
    paused = 0

elapsed = (now - start) if start else paused
remaining = max(0, duration - elapsed)

print("Content-Type: text/html\n")

print(f"""
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Timer</title>

<style>
body {{
    margin: 0;
    height: 100vh;
    display: flex;
    justify-content: center;
    align-items: center;
    background: #0b0f1a;
    color: white;
    font-family: system-ui;
}}

.card {{
    text-align: center;
}}

.time {{
    font-size: 120px;
    font-weight: 700;
    background: linear-gradient(90deg, #7c3aed, #22c55e);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
}}

button {{
    padding: 10px 18px;
    margin: 5px;
    border: 0;
    border-radius: 10px;
    cursor: pointer;
    font-weight: bold;
}}

.start {{ background: #22c55e; }}
.pause {{ background: #f59e0b; }}
.reset {{ background: #ef4444; }}
</style>
</head>

<body>
<div class="card">

<h2>TIMER</h2>
<div class="time">{remaining}</div>

<form method="POST">
    <input type="hidden" name="start" value="{start}">
    <input type="hidden" name="paused" value="{paused}">
    <input type="hidden" name="duration" value="{duration}">

    <button class="start" name="action" value="start">Start</button>
    <button class="pause" name="action" value="pause">Pause</button>
    <button class="reset" name="action" value="reset">Reset</button>
</form>

</div>
</body>
</html>
""")