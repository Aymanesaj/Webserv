#!/usr/bin/env python3
import os
from urllib.parse import parse_qs

def main():
    query = os.environ.get("QUERY_STRING", "")
    params = parse_qs(query)

    name = params.get("name", ["galaxy_demo"])[0]
    value = params.get("value", ["hello"])[0]
    path = params.get("path", ["/"])[0]
    samesite = params.get("samesite", ["Lax"])[0]
    max_age = params.get("max_age", [""])[0]

    cookie = f"{name}={value}; Path={path}; SameSite={samesite}"
    if max_age.strip():
        cookie += f"; Max-Age={max_age.strip()}"

    print("Content-Type: text/plain; charset=utf-8")
    print("X-Galaxy-CGI: cookie_set")
    print(f"Set-Cookie: {cookie}")
    print("")

    print("Set-Cookie sent:")
    print(cookie)

if __name__ == "__main__":
    main()
