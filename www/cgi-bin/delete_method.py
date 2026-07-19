#!/usr/bin/env python3
import os
from urllib.parse import parse_qs

method = os.environ.get('REQUEST_METHOD', '')
uri = os.environ.get('REQUEST_URI', '')
query = os.environ.get('QUERY_STRING', '')
params = parse_qs(query)
resource = params.get('resource', ['unknown'])[0]

print('CGI DELETE script executed successfully')
print('method=' + method)
print('resource=' + resource)
print('request_uri=' + uri)
