# WinsockTut

Simple TCP echo server implement with different Winsock I/O strategies.


### Build    

Obtain [premake5](http://premake.github.io/download.html).

To build via Visual Studio simply execute:

    premake5 vs2013
    

### Introduction

I/O Model       | Description
----------------|------------
socket          | basic BSD socket
select          | with select()
async_select    | with WSAAsyncSelect()
async_event     | with WSAAsyncSelect()
complete_routine| with alertable I/O
overlap         | with overlapped I/O
iocp            | with Completion Port, both server and client usage


`test_echo.py` is a simple python script to test server performance
