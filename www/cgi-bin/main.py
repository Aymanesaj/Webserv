#!/usr/bin/env python3
import os
import sys

# test POST method
if os.environ['REQUEST_METHOD'] == 'POST':
    content_length = int(os.environ['CONTENT_LENGTH'])
    body = sys.stdin.read(content_length)
    print("Content-Type: text/html")
    print()
    print("<html><body><h1>POST Method</h1>")
    print(f"<p>firstName: {body.split('&')[0].split('=')[1]}</p>")
    print(f"<p>lastName: {body.split('&')[1].split('=')[1]}</p>")
    print("</body></html>")
# test GET method
elif os.environ['REQUEST_METHOD'] == 'GET':
    query_string = os.environ['QUERY_STRING']
    print("Content-Type: text/html")
    print()
    print("<html><body><h1>GET Method</h1>")
    print(f"<p>firstName: {query_string.split('&')[0].split('=')[1]}</p>")
    print(f"<p>lastName: {query_string.split('&')[1].split('=')[1]}</p>")
    print("</body></html>")
print('<h1>CGI Test Script</h1>')
for key, value in os.environ.items():
    print(f'<p>{key}: {value}</p>')
