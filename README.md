*This project has been created as part of the 42 curriculum by asajed, obouizi, atigzim.*

## Description
This project is an HTTP server written in C++ 98. The goal of this project is to understand the Hypertext Transfer Protocol (HTTP) and how web servers operate under the hood. The server is capable of serving static websites, handling file uploads, and executing CGI scripts in a non-blocking manner using `poll()` or equivalent multiplexing. It implements a subset of the HTTP/1.1 specifications, handling methods such as GET, POST, and DELETE, and allows configuring multiple virtual servers, routes, error pages, and more.

## Instructions
To build the project:
```bash
make
```

To run the server:
```bash
./webserv [configuration file]
```

To clean the compiled object files:
```bash
make clean
```

To remove the executable and object files:
```bash
make fclean
```

## Resources
- [RFC 9110: HTTP Semantics](https://www.rfc-editor.org/rfc/rfc9110.html)
- [RFC 9112: HTTP/1.1](https://www.rfc-editor.org/rfc/rfc9112.html)
- [Socket Programming](https://www.geeksforgeeks.org/c/socket-programming-cc/) - Reference for socket programming.
- [NGINX Documentation](https://nginx.org/en/docs/) - Used as a reference for configuration file structure and HTTP response behaviors.

**AI Usage:**
AI tools were used during this project to assist with the following tasks:
- Reducing repetitive boilerplate code writing (e.g., getter/setter methods in C++ classes).
- Providing explanations and examples of specific system calls (like `poll()`, `socket()`, etc.) and HTTP status codes.
- Assisting in the generation of documentation and this README file.
