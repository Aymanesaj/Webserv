#!/usr/bin/env python3

import os
import sys
import random
from urllib.parse import parse_qs

print("Content-Type: text/html\n")

length = int(os.environ.get("CONTENT_LENGTH", 0))
raw = sys.stdin.read(length) if length > 0 else ""
data = parse_qs(raw)

board = data.get("board", ["---------"])[0]

def winner(b):
    wins = [
        (0,1,2),(3,4,5),(6,7,8),
        (0,3,6),(1,4,7),(2,5,8),
        (0,4,8),(2,4,6)
    ]
    for a,b1,c in wins:
        if b[a] != '-' and b[a] == b[b1] == b[c]:
            return b[a]
    return ""

def moves(b):
    return [i for i in range(9) if b[i] == '-']

def ai(b):
    for m in moves(b):
        t = b[:m] + "O" + b[m+1:]
        if winner(t) == "O":
            return m
    for m in moves(b):
        t = b[:m] + "X" + b[m+1:]
        if winner(t) == "X":
            return m
    return random.choice(moves(b)) if moves(b) else -1

game_over = False
win = ""

if "cell" in data:
    cell = int(data["cell"][0])

    if board[cell] == '-':
        board = board[:cell] + "X" + board[cell+1:]

        if not winner(board):
            m = ai(board)
            if m != -1:
                board = board[:m] + "O" + board[m+1:]

win = winner(board)
if win or '-' not in board:
    game_over = True


def cell(i):
    v = board[i]
    cls = "x" if v == "X" else "o" if v == "O" else ""

    return f"""
    <button class="cell {cls}" name="cell" value="{i}" {"disabled" if game_over or v != '-' else ""}>
        {v if v != '-' else ""}
    </button>
    """


print(f"""
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>XO Arena</title>

<style>

body {{
    margin:0;
    font-family: system-ui;
    background: radial-gradient(circle at top, #0b1220, #020617);
    color:white;
    overflow:hidden;
}}

.header {{
    position:fixed;
    top:0;
    width:100%;
    padding:20px;
    text-align:center;
    font-size:26px;
    letter-spacing:6px;
    background:rgba(0,0,0,0.4);
    backdrop-filter: blur(10px);
}}

.container {{
    display:flex;
    height:100vh;
    align-items:center;
    justify-content:space-around;
    padding-top:80px;
}}

.left-panel {{
    width:220px;
    padding:20px;
    background:rgba(255,255,255,0.05);
    border-radius:16px;
    box-shadow:0 0 20px rgba(0,0,0,0.4);
}}

.left-panel h2 {{
    margin-bottom:10px;
    font-size:18px;
    opacity:0.8;
}}

.stat {{
    margin:10px 0;
    padding:10px;
    background:rgba(255,255,255,0.05);
    border-radius:10px;
}}

.board {{
    display:grid;
    grid-template-columns:repeat(3,130px);
    gap:12px;
    padding:20px;
    background:rgba(255,255,255,0.03);
    border-radius:20px;
    box-shadow:0 0 40px rgba(0,0,0,0.5);
}}

.cell {{
    width:130px;
    height:130px;
    font-size:52px;
    border:none;
    border-radius:20px;
    cursor:pointer;
    background:rgba(255,255,255,0.06);
    transition:0.2s;
    color:white;
}}

.cell:hover {{
    transform:scale(1.05);
    background:rgba(255,255,255,0.12);
}}

.x {{
    color:#22c55e;
    text-shadow:0 0 15px #22c55e;
}}

.o {{
    color:#3b82f6;
    text-shadow:0 0 15px #3b82f6;
}}

.right-panel {{
    width:220px;
    padding:20px;
    background:rgba(255,255,255,0.05);
    border-radius:16px;
}}

.status {{
    font-size:20px;
    margin-bottom:10px;
}}

.glow {{
    animation:pulse 1.2s infinite;
}}

@keyframes pulse {{
    0% {{ opacity:0.6; }}
    50% {{ opacity:1; }}
    100% {{ opacity:0.6; }}
}}

.modal {{
    display:{'flex' if game_over else 'none'};
    position:fixed;
    top:0; left:0;
    width:100%;
    height:100%;
    background:rgba(0,0,0,0.7);
    justify-content:center;
    align-items:center;
}}

.popup {{
    padding:30px;
    background:#0f172a;
    border-radius:16px;
    text-align:center;
    box-shadow:0 0 40px rgba(0,0,0,0.7);
}}

button {{
    margin-top:10px;
    padding:10px 18px;
    border:none;
    border-radius:10px;
    cursor:pointer;
    background:linear-gradient(90deg,#22c55e,#06b6d4);
    color:white;
}}

</style>
</head>

<body>

<div class="header">XO ARENA</div>

<div class="container">

<div class="left-panel">
    <h2>GAME INFO</h2>
    <div class="stat">Mode: Player vs AI</div>
    <div class="stat">Player: X</div>
    <div class="stat">AI: O</div>
    <div class="stat">State: {"FINISHED" if game_over else "RUNNING"}</div>
</div>

<form method="POST">

<input type="hidden" name="board" value="{board}">

<div class="board">
""")

for i in range(9):
    print(cell(i))

print("""
</div>

</form>

<div class="right-panel">
""")

if win:
    print(f'<div class="status glow">Winner: {win}</div>')
else:
    print('<div class="status">No winner yet</div>')

print("""
</div>

</div>

<div class="modal">
    <div class="popup">
""")

if win:
    print(f"<h2>🏆 Winner: {win}</h2>")
else:
    print("<h2>🤝 Draw Game</h2>")

print("""
<form method="POST">
    <button>Play Again</button>
</form>

</div>
</div>

</body>
</html>
""")