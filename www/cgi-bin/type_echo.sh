printf 'Content-Type: text/plain; charset=utf-8\n'
printf 'X-Galaxy-CGI: shell\n'
printf '\n'
printf 'cgi_type=sh\n'
printf 'method=%s\n' "${REQUEST_METHOD:-}"
printf 'query=%s\n' "${QUERY_STRING:-}"
