<?php
header('Content-Type: text/plain; charset=utf-8');
header('X-Galaxy-CGI: php');
echo "cgi_type=php\n";
echo "method=" . ($_SERVER['REQUEST_METHOD'] ?? '') . "\n";
echo "query=" . ($_SERVER['QUERY_STRING'] ?? '') . "\n";
?>
