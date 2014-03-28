Alex Barke 860883440
Sam Lee    860957929

CS 164 Final Project README

================================
General Outline
================================

Repeater
--------------------------------
Command Line Args:
To compile: 
gcc repeater.c -pthread -o repeater

To run:
<repeater> <hostname> <port> <[upstreamIP:upstream port]>

Example of run arguments:
repeater alpha-9.cs.ucr.edu 8001
OR
repeater alpha-9.cs.ucr.edu 8001 alpha-10.cs.ucr.edu:8002

The 4th argument is optional.  If you provide a 4th argument, you must
supply the parent repeater that the current repeater connects to along
with the port.

The first thing a non-master repeater is connect to a master repeater that is
given and sends a training message to it.  It then creates a socket and accepts
all endnodes that try to connect to it.  Once a client connects to it,
it spawns a thread to handle all subsequent messages to the repeater.

If the repeater is a master repeater, it creates a socket and accepts all
repeaters and clients that connect to it.

The repeater waits for input from the clients and responds to them using a 
switch statement based upon the message type. 
If a client send a request_normal or request_high, the repeater stores each 
request into a request array and evaluates them based upon the highest request 
given.  If there is a high request in the array, it is given the grant 
from the repeater.  If there is no high request, the first normal request is 
gievn the grant.  All other clients are given incoming messages.

If the repeater receives a data packet from a client that has grant 
privelidges, the repeater accepts the frame and passes it on to every client 
it knows.  Only the client that has a source address that matches the frame's 
destination address actually displays the message.



Client
--------------------------------
Command line Args:
To compile:
gcc node.c -o node

To run:
<node> <node IP Address> <Repeater IP Addr> <Port of Parent Repeater>

Example of run arguments:
node alpha-8.cs.ucr.edu alpha-9.cs.ucr.edu 8001

All 4 arguments are mandatory.  

A client must not run before the repeater it is trying to connect to has run 
first.  A client receives a idle down message after it sends a training message
to the parent repeater.  At this point, the node has options to:
1) Send a normal request to the repeater
2) Send a high request to the repeater
3) Send a data frame
4) Send another training message

Based on these frame options, the client sends a frame to the repeater and 
the repeater responds.  The client interprets repeater frames based upon a 
switch statement it has.  

The client is notified if it is granted access or the repeater has incoming 
messages.  From there, the client may send a data packet to the repeater for 
further forwarding.   
