#!/usr/bin/env python3
import os
import sys
import cgi

def main():
    print("Content-Type: text/plain; charset=utf-8")
    print("X-Galaxy-CGI: upload_inspect")
    print("")

    method = os.environ.get("REQUEST_METHOD", "")
    content_type = os.environ.get("CONTENT_TYPE", "")
    length = os.environ.get("CONTENT_LENGTH", "0")

    print(f"method={method}")
    print(f"content_type={content_type}")
    print(f"content_length={length}")

    if method != "POST":
        print("This endpoint expects POST multipart/form-data.")
        return

    try:
        form = cgi.FieldStorage(fp=sys.stdin, environ=os.environ, keep_blank_values=True)
    except Exception as e:
        print("FieldStorage error:")
        print(str(e))
        return

    if not form:
        print("No form fields parsed.")
        return

    print("\nfields:")
    for key in form.keys():
        field = form[key]
        if isinstance(field, list):
            print(f"- {key}: {len(field)} items")
            continue

        filename = getattr(field, 'filename', None)
        if filename:
            fileobj = field.file
            fileobj.seek(0, 2)
            size = fileobj.tell()
            fileobj.seek(0)
            head = fileobj.read(64)
            print(f"- {key}: file filename={filename} size={size} bytes")
            print(f"  first64={head!r}")
        else:
            value = field.value
            if isinstance(value, bytes):
                try:
                    value = value.decode('utf-8', 'replace')
                except Exception:
                    value = str(value)
            print(f"- {key}: value={value}")

if __name__ == "__main__":
    main()
