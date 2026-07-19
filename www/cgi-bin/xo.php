#!/usr/bin/php-cgi
<?php

header("Content-Type: text/html; charset=UTF-8");

$board = isset($_POST['board']) ? $_POST['board'] : "---------";
$turn  = isset($_POST['turn']) ? $_POST['turn'] : "X";

function checkWinner($board)
{
    $wins = [
        [0,1,2],
        [3,4,5],
        [6,7,8],
        [0,3,6],
        [1,4,7],
        [2,5,8],
        [0,4,8],
        [2,4,6]
    ];

    foreach ($wins as $w)
    {
        if (
            $board[$w[0]] != '-' &&
            $board[$w[0]] == $board[$w[1]] &&
            $board[$w[1]] == $board[$w[2]]
        )
        {
            return $board[$w[0]];
        }
    }

    return "";
}

if (isset($_POST['cell']))
{
    $cell = (int)$_POST['cell'];

    if ($board[$cell] == '-')
    {
        $board[$cell] = $turn;

        $turn = ($turn == "X") ? "O" : "X";
    }
}

$winner = checkWinner($board);

$draw = true;

for ($i = 0; $i < 9; $i++)
{
    if ($board[$i] == '-')
    {
        $draw = false;
        break;
    }
}

?>
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>XO CGI Game</title>

<style>

body {
    margin: 0;
    height: 100vh;
    display: flex;
    justify-content: center;
    align-items: center;
    background:
        radial-gradient(circle at top, #111827, #020617);
    font-family: Arial, sans-serif;
    color: white;
}

.container {
    text-align: center;
}

.title {
    font-size: 42px;
    letter-spacing: 5px;
    margin-bottom: 25px;
}

.board {
    display: grid;
    grid-template-columns: repeat(3, 120px);
    gap: 12px;
}

.cell {
    width: 120px;
    height: 120px;
    border: none;
    border-radius: 22px;
    font-size: 54px;
    font-weight: bold;
    cursor: pointer;
    background: rgba(255,255,255,0.08);
    color: white;
    transition: 0.2s;
    backdrop-filter: blur(10px);
}

.cell:hover {
    transform: scale(1.05);
    background: rgba(255,255,255,0.15);
}

.x {
    color: #22c55e;
    text-shadow: 0 0 20px #22c55e;
}

.o {
    color: #3b82f6;
    text-shadow: 0 0 20px #3b82f6;
}

.info {
    margin-top: 25px;
    font-size: 24px;
    opacity: 0.9;
}

.reset {
    margin-top: 20px;
    padding: 12px 24px;
    border: none;
    border-radius: 12px;
    font-size: 16px;
    font-weight: bold;
    cursor: pointer;
    color: white;
    background: linear-gradient(
        90deg,
        #22c55e,
        #06b6d4
    );
}

.reset:hover {
    opacity: 0.9;
}

</style>
</head>

<body>

<div class="container">

<div class="title">TIC TAC TOE</div>

<form method="POST">

<input type="hidden" name="board"
value="<?php echo htmlspecialchars($board); ?>">

<input type="hidden" name="turn"
value="<?php echo htmlspecialchars($turn); ?>">

<div class="board">

<?php

for ($i = 0; $i < 9; $i++)
{
    $value = $board[$i];

    $class = "";

    if ($value == "X")
        $class = "x";

    else if ($value == "O")
        $class = "o";

    echo '<button
            type="submit"
            class="cell ' . $class . '"
            name="cell"
            value="' . $i . '"';

    if ($winner || $value != '-')
        echo ' disabled';

    echo '>';

    echo ($value == '-') ? "" : $value;

    echo '</button>';
}

?>

</div>

</form>

<div class="info">

<?php

if ($winner)
{
    echo "Winner: " . $winner;
}
else if ($draw)
{
    echo "Draw Game";
}
else
{
    echo "Turn: " . $turn;
}

?>

</div>

<form method="POST">
    <button class="reset" type="submit">
        Reset Game
    </button>
</form>

</div>

</body>
</html>