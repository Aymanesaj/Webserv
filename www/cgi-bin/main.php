<?php

if (isset($_POST['firstName']) && isset($_POST['lastName']))
{
    $firstName = htmlspecialchars($_POST['firstName']);
    $lastName = htmlspecialchars($_POST['lastName']);

    echo "<h1>Success!</h1>";
    echo "<p>First Name: " . $firstName . "</p>";
    echo "<p>Last Name: " . $lastName . "</p>";
}
else if (isset($_GET['firstName']) && isset($_GET['lastName']))
{
    $firstName = htmlspecialchars($_GET['firstName']);
    $lastName = htmlspecialchars($_GET['lastName']);

    echo "<h1>Success!</h1>";
    echo "<p>First Name: " . $firstName . "</p>";
    echo "<p>Last Name: " . $lastName . "</p>";
}
else
{
    echo "<h1>Error!</h1>";
    echo "<p>Missing required fields.</p>";
}

?>