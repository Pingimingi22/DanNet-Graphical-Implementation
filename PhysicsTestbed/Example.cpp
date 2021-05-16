#include "Example.h"

#include <vector>

//#include "Player.h"

//#include "Peer.h"
#include "MessageIdentifiers.h"
#include "Packet.h"
// testing player
#include "CustomIdentifiers.h"
#include <iostream>
#include <fstream>

//#include "PacketPriorities.h"

#include "CorePackets.h" // Here so user's can do stuff with the ClientTimeout struct.



// ------------ Struct to help with packet serialization ------------ //
struct PlayerCreateStruct
{
	int firstByte = (int)CustomIdentifier::PLAYER_CREATE;

	float m_xPos = 0;
	float m_yPos = 0;

	int m_id = -1;

	char name[25];
};
// ------------------------------------------------------------------ //

Example::Example() : Testbed()
{
	// ========================== Creation of peers ========================== //
	// With DanNet you want to create a peer. A peer can be a server or client.
	// Since we have left the peer constructor empty, it's going to make our peer a client. It will automatically assign an ephemeral port and will bind to the most
	// appropriate ip address the kernel can find.

	testPeer = new Peer();
	testPeer->StartPeer();


	// ========================== Lag simulation ========================== //
	// 	DanNet supports very basic lag simulation. All lag simulation does it stall
	// 	the time for your packets to send out. SimulateLag() takes in a double which
	// 	is the milliseconds you want to lag your packets by.


	//testPeer->SimulateLag(true, 250); // ---------------------------> Umcomment this code to stall the rate at which you send packets at by 250 milliseconds.

	



	// In this little demo, client's can have names which you can see on the server when a client connects.
	strcpy_s(name, "testname");

	// ----------------------- File reader so you can specify a server ipaddress to connect to with client.cfg ----------------------- //
	std::ifstream clientConfig;
	clientConfig.open("client.cfg");
	if (!clientConfig.good())
	{
		std::cout << "Please create a client.cfg file and place it next to the executable." << std::endl;
		this->Shutdown();
	}
	// If the file read worked, let's try to connect.
	else
	{
		std::string fileIP;
		std::getline(clientConfig, fileIP);
		clientConfig.close();
		// ------------------------------------------------------------------------------------------------------------------------------- //


		// After we've got the ipaddress and since we've hard coded the servers port to 25565, we can now connect.
		testPeer->Connect(fileIP.c_str(), 25565);
	}
}




// ------------------------ In this Update() function, there are examples of sending and receiving of packets. ------------------------ //

// ========================== Receiving packets with DanNet ========================== //
	// To receive packets with DanNet you create a Packet pointer and set it nullptr. In your
	// game loop, you constantly set your packet pointer to Peer.UDPReceivePacket() which will
	// return the next available packet for you to use.

	// You can call GetPacketIndentifier() on packets to see what type of message they are.
	// [IMPORTANT] Make sure to call FlushCurrentPacket() on your peer after dealing with a packet to remove it from the packet queue.



// ========================== Sending packets with DanNet ========================== //
	// To send packets with DanNet you first create a Packet. In the Packet constructor you
	// specifiy whether you want a reliable or unreliable UDP packet.
	// 
	// After creating the packet, you serialize what data you want into the packet with 
	// the .Serialize() function. Be careful with how you order what you serialize!
	// 
	// After serializing your data in a packet you can then send it with either Send(),
	// SendTo() or SendToAll(). 


void Example::Update()
{
	//This call ensures that your mouse position and aspect ratio are maintained as correct.
	Testbed::Update();


	// If we have successfully connected but we don't have a player, we will make a player. this is really dodgy and not how I would do it for a real game but I think it works fine for this little demo.
	if (testPeer->GetId() != -1 && m_myPlayer == nullptr)
	{
		m_myPlayer = new Player(testPeer->GetId(), name, glm::vec3(0, 1, 0), true); // ============= WARNING ============= this player is on the heap and we are deleting when Example.cpp is destructed. //

		Packet playerCreationPacket(PacketPriority::UNRELIABLE_UDP);
		PlayerCreateStruct playerCreateS;
		playerCreateS.m_id = testPeer->GetId();
		strcpy_s(playerCreateS.name, name);

		playerCreationPacket.Serialize(playerCreateS.firstByte, playerCreateS.m_xPos, playerCreateS.m_yPos, playerCreateS.m_id, playerCreateS.name);
		testPeer->UDPSend(playerCreationPacket);

		std::cout << "our player created: " << playerCreateS.m_id << " name: " << name << std::endl;
	}


	// Packet checking update.
	Packet* incomingPacket = nullptr;
	incomingPacket = testPeer->UDPReceivePacket();
	if (incomingPacket != nullptr)
	{
		switch (incomingPacket->GetPacketIdentifier())
		{
		case (MessageIdentifier)CustomIdentifier::PLAYER_CREATE:
		{
			PlayerCreateStruct playerCreateS;
			incomingPacket->Deserialize(playerCreateS.firstByte, playerCreateS.m_xPos, playerCreateS.m_yPos, playerCreateS.m_id, playerCreateS.name);
			if (playerCreateS.m_id == testPeer->GetId()) // if this an echo of our creation, ignore it.
			{
				testPeer->FlushCurrentPacket();
				break;
			}
			else // otherwise, a new player has joined, so we have to construct them graphically on our side.
			{
				std::cout << "other player created: " << playerCreateS.m_id << " name: " << playerCreateS.name << std::endl;
				m_allPlayers.push_back(Player(playerCreateS.m_id, playerCreateS.name, glm::vec3(1, 0, 0)));
			}
			testPeer->FlushCurrentPacket();
			break;
		}	
		case (MessageIdentifier)CustomIdentifier::PLAYER_MOVE:
		{
			PlayerMoveStruct playerMoveS;
			incomingPacket->Deserialize(playerMoveS.firstByte, playerMoveS.m_xPos, playerMoveS.m_yPos, playerMoveS.m_id);
			if (playerMoveS.m_id == testPeer->GetId()) // if we're echoing back our own movement, ignore
			{
				testPeer->FlushCurrentPacket();
				break;
			}
			else // otherwise a different player has moved.
			{
				for (int i = 0; i < m_allPlayers.size(); i++)
				{
					if (m_allPlayers[i].m_id == playerMoveS.m_id)
					{
						// we found the player to move.
						m_allPlayers[i].m_xPos = playerMoveS.m_xPos;
						m_allPlayers[i].m_yPos = playerMoveS.m_yPos;

						//assert(playerMoveS.m_xPos != 0);
						if (playerMoveS.m_xPos == 0)
						{
							std::cout << "test" << std::endl;
						}
					}
				}
			}
			testPeer->FlushCurrentPacket();
			break;
		}
		case MessageIdentifier::CLIENT_TIMEOUT:											// ================================== NOTE ================================== /
		{																				// MessageIdentifier::CLIENT_TIMEOUT is the only core message identifier that
																						// user's of DanNet have to deal with. I designed it this way to help with
																						// adding player disconnect functionality.
				
			// We've received a message from *probably* the server (unless someone is trying to use my library for peer to peer or something) that a client has timed out. We want to remove them from our
			// player's vector so we don't render them anymore.
			ClientTimeout clientTimeoutStruct;
			incomingPacket->Deserialize(clientTimeoutStruct.MessageIdentifier, clientTimeoutStruct.clientID);

			bool hasFoundPlayer = false;
			for (int i = 0; i < m_allPlayers.size(); i++)
			{
				if (m_allPlayers[i].m_id == clientTimeoutStruct.clientID)
				{
					// We've found the player to remove!
					m_allPlayers.erase(m_allPlayers.begin() + i);
					std::cout << "Removed timed out client from our player's vector." << std::endl;
					hasFoundPlayer = true;
					break;
				}
			}
			if (!hasFoundPlayer) // If we didn't find the player, that means we received a timeout message to remove a client we don't have. This is pretty bad but it could mean we already have removed
			{					 // the client.
				std::cout << "Received a timeout message for a client but we don't have that client listed as a player!" << std::endl;
				//assert(false); This isn't neccessary as apparently it's pretty common to receive timeout messages for client's we don't have especially if someone highlights text on the console and
				// freezes everything...
			}
			testPeer->FlushCurrentPacket();
			break;
		}
		default:
			std::cout << "Received weird message" << std::endl;
			testPeer->FlushCurrentPacket();
			break;
		}
	}



	// --------------- Updating player's positions ---------------

	for (int i = 0; i < m_allPlayers.size(); i++)
	{
		m_allPlayers[i].Update(window, deltaTime, *testPeer);
	}
	if (m_myPlayer != nullptr)
	{
		m_myPlayer->Update(window, deltaTime, *testPeer);
	}
}

void Example::Render()
{
	// --------------- Rendering player's ---------------

	for (int i = 0; i < m_allPlayers.size(); i++)
	{
		m_allPlayers[i].Draw(lines);
	}
	if (m_myPlayer != nullptr)
	{
		m_myPlayer->Draw(lines);
	}

	
	Testbed::Render();
}