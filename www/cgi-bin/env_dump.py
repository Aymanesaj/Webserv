#!/usr/bin/env python3
import os

def main():
    print("Content-Type: text/plain; charset=utf-8")
    print("X-Galaxy-CGI: env_dump")
    print("")

    keys = sorted(os.environ.keys())
    for k in keys:
        print(f"{k}={os.environ.get(k,'')}")

if __name__ == "__main__":
    main()
