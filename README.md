# Go-Back-N
A robust yet efficient file transfer protocol implemented over UDP.

## Overview
Data loss is an ever-present possibility when transferring data over a network. This risk is mitigated by implementing data transfer protocols that guarantee reliable, in order data delivery to the application. These protocols are often implemented implemented at the OS level; such is the case with TCP sockets. In this project, I'm using UDP as my transport protocol, and I'm implementing my own reliability protocol on top of it.

### Why use UDP?
Because UDP doesn't guarantee reliability, it serves the purpose of emulating the link layer that one would be dealing with if they were implementing reliability for TCP. Since UDP sends data via explicit user-defined packets instead of via a byte stream like TCP does, it's easy to simuate packets and acknowledgements (ACKs) moving across the network. It also makes it easy to test the server's ability to handle lost ACKs.

## Running the Program
1. Download the source code
2. Make the server and run it on a port of your choice:
```make```
```./server [port]```
3. Open another shell and navigate to ```/clients```
4. Make the client and give it the server's IP and port number:
```./a.out [Server IP] [Server Port]```
5. The client is configured to request a valid file from the `source_files` dir. The client can be modified to simulate packet loss, or to request a file that doesn't exist in `source_files`. The server will continue trying to transmit a valid file until it receives 5 duplicate ACKs from the client.

## Acknowledgements
Professor Fahad Dogar, the Networks professor at Tufts.
Ismail Badrezzamane, for his help writing an excellent test suite for this program.

 
