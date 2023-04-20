# TCP Chat Server in C and Linux

Author: Yaki Kol


## Files
chatServer.h <br>
chatServer.c

## Remarks
This is a basic TCP server implementation in C.
It uses the select() function to handle multiple client connections and uses a connection pool to keep track of the active connections.
The server listens for incoming connections on the specified port, and it uses a signal handler to gracefully exit when the user sends an interrupt signal (Ctrl+C).

## Usage 
To run the server, use the following command:<br>
gcc chatServer.c -o server <br>
./server <port> 

Where <port> is the port number on which the server should listen for incoming connections.<br>
The port number must be between 1 and 2^16.

You can stop the server by sending an interrupt signal (Ctrl+C) to the terminal.
