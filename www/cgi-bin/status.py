#!/usr/bin/env python3
import os
from urllib.parse import parse_qs

def main():
    query = os.environ.get("QUERY_STRING", "")
    params = parse_qs(query)

    code = params.get("code", ["418"])[0]
    message = params.get("message", ["CGI status test"])[0]

    try:
        status = int(code)
    except ValueError:
        status = 418

    print(f"Status: {status}")
    print("Content-Type: text/plain; charset=utf-8")
    print("X-Galaxy-CGI: status")
    print("")

    print(f"status={status}")
    print(f"message={message}")
    print(f"method={os.environ.get('REQUEST_METHOD','')}")

if __name__ == "__main__":
    main()
