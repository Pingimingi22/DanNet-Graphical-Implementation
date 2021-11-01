# DanNet-Graphical-Implementation
An example implementation project to show off my networking library, DanNet.


# Implementation of DanNet

## Instructions
- To use my example implementation of DanNet:

	- Run the server exe first. The server will launch on port 25565 and will 
	be assigned the most appropriate ip address.
	
	- After the server has launched. Go into the client.cfg file and make sure
	the first line is the ip address of your machine. This is telling the client
	what ip address to connect to.
	
	- When you have configured the client.cfg file, run the client exe and you
	should be able to see a green circle appear. Run a few more instances of the client
	and you will see red circles appear on the first client. The green circle is your local
	player and the red ones are other connected players.
	
	
## Lag simulation feature

- To simulate lag:
	
	- In both the source code for the client and server, there are function calls
	to SimulateLag() which are commented out. Uncomment these and build the appropriate
	project and now the program should be lagging behind.
	
	- You can find these comments in the Example.cpp file for the client, and the EntryPoint.cpp
	file for the server.
