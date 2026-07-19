#!/usr/bin/env python3
import os
from urllib.parse import parse_qs

def main():
    query = os.environ.get("QUERY_STRING", "")
    params = parse_qs(query)

    location = params.get("to", ["/index.html"])[0]
    code = params.get("code", ["302"])[0]

    try:
        status = int(code)
    except ValueError:
        status = 302

    print(f"Status: {status}")
    print(f"Location: {location}")
    print("Content-Type: text/plain; charset=utf-8")
    print("X-Galaxy-CGI: redirect")
    print("")

    print(f"Redirecting to {location} with status {status}.")

if __name__ == "__main__":
    main()
