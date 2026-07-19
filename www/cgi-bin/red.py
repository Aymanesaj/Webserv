#!/usr/bin/env python3

import os
from urllib.parse import unquote

query = os.environ.get("QUERY_STRING", "")

# decode URL-encoded input
target = unquote(query)

# basic safety fallback
if not target.startswith("http://") and not target.startswith("https://"):
    target = "https://www.google.com"

print("Status: 302 Found")
print(f"Location: {target}")
print("Content-Type: text/html\n")

print(f"""
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Redirecting</title>

    <meta http-equiv="refresh" content="0; url={target}">
</head>
<body>
    <p>Redirecting to {target}</p>
</body>
</html>
""")