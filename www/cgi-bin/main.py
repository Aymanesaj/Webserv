#!/usr/bin/env python3
import os
import sys
import urllib.parse

print("<html><head><title>Python CGI Test</title></head><body>")

method = os.environ.get('REQUEST_METHOD', 'UNKNOWN')
print(f"<h1>Method: {method}</h1>")

data = {}

if method == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        body = sys.stdin.read(content_length)
        data = urllib.parse.parse_qs(body)
elif method == 'GET':
    query_string = os.environ.get('QUERY_STRING', '')
    data = urllib.parse.parse_qs(query_string)

# Display extracted data
print("<h2>Received Parameters:</h2><ul>")
if data:
    for key, values in data.items():
        for value in values:
            print(f"<li><b>{key}</b>: {value}</li>")
else:
    print("<li><i>No parameters received.</i></li>")
print("</ul>")

# Display CGI environment
print("<h2>CGI Environment Variables:</h2><table border='1'>")
for key, value in sorted(os.environ.items()):
    print(f"<tr><td><b>{key}</b></td><td>{value}</td></tr>")
print("</table>")

print("</body></html>")
