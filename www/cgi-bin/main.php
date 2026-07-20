#!/usr/bin/env php-cgi
<?php
echo "<html><head><title>PHP CGI Test</title></head><body>";

$method = isset($_SERVER['REQUEST_METHOD']) ? $_SERVER['REQUEST_METHOD'] : 'UNKNOWN';
echo "<h1>Method: " . htmlspecialchars($method) . "</h1>";

$data = array();
if ($method === 'POST') {
    $data = $_POST;
} else if ($method === 'GET') {
    $data = $_GET;
}

echo "<h2>Received Parameters:</h2><ul>";
if (!empty($data)) {
    foreach ($data as $key => $value) {
        if (is_array($value)) {
            $value = implode(", ", $value);
        }
        echo "<li><b>" . htmlspecialchars($key) . "</b>: " . htmlspecialchars($value) . "</li>";
    }
} else {
    echo "<li><i>No parameters received.</i></li>";
}
echo "</ul>";

echo "<h2>CGI Environment Variables:</h2><table border='1'>";
foreach ($_SERVER as $key => $value) {
    if (is_string($value) || is_numeric($value)) {
        echo "<tr><td><b>" . htmlspecialchars($key) . "</b></td><td>" . htmlspecialchars($value) . "</td></tr>";
    }
}
echo "</table>";

echo "</body></html>";
?>