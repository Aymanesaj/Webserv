#!/usr/bin/env python3
import os

print('Content-Type: text/plain; charset=utf-8')
print('X-Galaxy-CGI: python')
print('')
print('cgi_type=py')
print('method=' + os.environ.get('REQUEST_METHOD', ''))
print('query=' + os.environ.get('QUERY_STRING', ''))
