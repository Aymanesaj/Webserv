#!/usr/bin/env python3
import os

def main():
    print("Content-Type: text/plain; charset=utf-8")
    print("X-Galaxy-CGI: cookie_echo")
    print("")

    cookie = os.environ.get("HTTP_COOKIE", "")
    if not cookie:
        print("No Cookie header received.")
        return

    print("Cookie header received:")
    print(cookie)

if __name__ == "__main__":
    main()
