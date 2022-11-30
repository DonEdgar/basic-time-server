# basic-time-server
Simple web server that will give local time

# Will run on GNU/Linux, MacOS, and Windows
if compiling on GNU/Linux or MacOS:
        `gcc time_server.c -o time_server`

if on Windows and using MinGW compiler:
         `gcc time_server.c -o time_server.exe -lws2_32`
     Note: Must remove the line that starts with `#pragma`
     
# Specifing a port
By default this web server uses the port 8080, can easily be changed when calling  `gettaddrinfo()`
