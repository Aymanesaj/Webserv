#!/usr/bin/env python3
import os
import sys
from urllib.parse import parse_qs

method = os.environ.get('REQUEST_METHOD', '')
length = int(os.environ.get('CONTENT_LENGTH', '0') or '0')
raw_body = sys.stdin.read(length) if length > 0 else ''
fields = parse_qs(raw_body)
username = fields.get('username', [''])[0]
message = fields.get('message', [''])[0]

print('CGI POST script executed successfully')
print('method=' + method)
print('username=' + username)
print('message=' + message)
print('raw_body=' + raw_body)
