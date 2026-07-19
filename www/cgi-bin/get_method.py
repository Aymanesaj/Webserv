#!/usr/bin/env python3
import os
from urllib.parse import parse_qs

method = os.environ.get('REQUEST_METHOD', '')
query = os.environ.get('QUERY_STRING', '')
params = parse_qs(query)
name = params.get('name', ['guest'])[0]
topic = params.get('topic', ['none'])[0]

print('CGI GET script executed successfully')
print('method=' + method)
print('name=' + name)
print('topic=' + topic)
print('raw_query=' + query)
