#!/usr/bin/env python3
import os

def main():
    print("Content-Type: text/plain; charset=utf-8")
    print("X-Galaxy-CGI: headers_echo")
    print("")

    print("Incoming request headers (mapped to CGI env as HTTP_*):")
    for key in sorted(os.environ.keys()):
        if key.startswith("HTTP_"):
            print(f"{key}={os.environ.get(key,'')}")

if __name__ == "__main__":
    main()
