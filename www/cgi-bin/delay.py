#!/usr/bin/env python3
import os
import time
from urllib.parse import parse_qs

def main():
    query = os.environ.get("QUERY_STRING", "")
    params = parse_qs(query)

    seconds_raw = params.get("seconds", ["1.5"])[0]
    try:
        seconds = float(seconds_raw)
    except ValueError:
        seconds = 1.5

    if seconds < 0:
        seconds = 0
    if seconds > 15:
        seconds = 15

    print("Content-Type: text/plain; charset=utf-8")
    print("X-Galaxy-CGI: delay")
    print("")

    print(f"Sleeping for {seconds} seconds...")
    time.sleep(seconds)
    print("Done.")

if __name__ == "__main__":
    main()
