#include <iostream>
#include "Peer.h"
#include "MessageIdentifiers.h"
#include "Packet.h"

#include <assert.h>

#include <chrono>

#include "PacketPriorities.h"

enum CustomIdentifier
{
	PLAYER_CREATE = MessageIdentifier::CUSTOM_USER_ENUM,
	PLAYER_MOVE,
};

// ------------------------------------- Creating structs to help easily serialize things into packets. ------------------------------------- //
// These structs are used to serialize things into packets. A key thing with DanNet is that the order of things you serialize matter.
struct PlayerCreateStruct
{
	int firstByte = (int)CustomIdentifier::PLAYER_CREATE;

	float m_xPos = 0;
	float m_yPos = 0;

	int m_id = -1;

	char name[25];

};
struct PlayerMoveStruct
{
	int firstByte = (int)CustomIdentifier::PLAYER_MOVE;

	float m_xPos = 0;
	float m_yPos = 0;

	int m_id = -1;
};

// this player struct is a basic version of what the server needs to know about player's.
struct Player
{
	float m_xPos = 0;
	float m_yPos = 0;

	int m_id = -1;

	char name[25];

};
// ------------------------------------------------------------------------------------------------------------------------------------------ //


// --- Super basic server that can store information about player's and their positions --- //
int main()
{
	std::vector<Player> allPlayers;

	// With DanNet you want to create a peer. A peer can be a server or client.
	// Since we have passed in true for the isServer parameter and have specified a port, our peer is going to behave like a server.

	Peer testPeer(true, 25565);
	testPeer.StartPeer();

	bool isRunning = true;


	// ========================== Lag simulation ========================== //
	// 	DanNet supports very basic lag simulation. All lag simulation does it stall
	// 	the time for your packets to send out. SimulateLag() takes in a double which
	// 	is the milliseconds you want to lag your packets by.
	


	//testPeer.SimulateLag(true, 250); // ------------------------------------------- Uncomment this to make your packets send out with a delay of 250 milliseconds (I guess it would be a ping of 500).



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

	Packet* incomingPacket = nullptr;
	while (isRunning)
	{
		incomingPacket = testPeer.UDPReceivePacket();
		if (incomingPacket != nullptr)
		{
			MessageIdentifier testIdentifier = incomingPacket->GetPacketIdentifier();
			switch (incomingPacket->GetPacketIdentifier())
			{
			case (MessageIdentifier)CustomIdentifier::PLAYER_CREATE:
			{
				PlayerCreateStruct playerCreateS;
				incomingPacket->Deserialize(playerCreateS.firstByte, playerCreateS.m_xPos, playerCreateS.m_yPos, playerCreateS.m_id, playerCreateS.name);
				
				Packet playerCreatePacket(PacketPriority::RELIABLE_UDP);
				playerCreatePacket.Serialize(playerCreateS.firstByte, playerCreateS.m_xPos, playerCreateS.m_yPos, playerCreateS.m_id, playerCreateS.name);


				std::cout << std::endl;
				std::cout << "UDPSendToAll() Called. Sending information about new player to all old players." << std::endl;
				testPeer.UDPSendToAll(playerCreatePacket);


				// Telling the newly created player about all the other player's who joined before him.
				Client* client;
				client = testPeer.GetClient(playerCreateS.m_id);

				std::cout << "Sending information about all older players to the new player." << std::endl;
				for (int i = 0; i < allPlayers.size(); i++)
				{
					
					PlayerCreateStruct anotherPlayerCreateS;
					anotherPlayerCreateS.m_id = allPlayers[i].m_id;
			
					strcpy_s(anotherPlayerCreateS.name, allPlayers[i].name);
					Packet testPacket(PacketPriority::RELIABLE_UDP);
					testPacket.Serialize(anotherPlayerCreateS.firstByte, anotherPlayerCreateS.m_xPos, anotherPlayerCreateS.m_yPos, anotherPlayerCreateS.m_id, anotherPlayerCreateS.name);
					std::cout << "UDPSendTo() called." << std::endl;
					testPeer.UDPSendTo(testPacket, client->m_ipAddress, client->m_port);
					std::cout << std::endl;
					std::cout << "===== [NEW CLIENT] ===== Sent the new client information about player " << allPlayers[i].m_id << ". " << std::endl;
					std::cout << std::endl;
				}

				// adding player to server player list.
				Player newPlayer;
				newPlayer.m_id = playerCreateS.m_id;
				strcpy_s(newPlayer.name, playerCreateS.name);
				allPlayers.push_back(newPlayer);

				testPeer.FlushCurrentPacket();
				break;
		
			}
			case (MessageIdentifier)CustomIdentifier::PLAYER_MOVE:
			{
				PlayerMoveStruct playerMoveS;
				incomingPacket->Deserialize(playerMoveS.firstByte, playerMoveS.m_xPos, playerMoveS.m_yPos, playerMoveS.m_id);

				for (int i = 0; i < allPlayers.size(); i++)
				{
					if (allPlayers[i].m_id == playerMoveS.m_id) // finding that specific player and updating their position.
					{
						allPlayers[i].m_xPos = playerMoveS.m_xPos;
						allPlayers[i].m_yPos = playerMoveS.m_yPos;
						Packet playerMovePacket(PacketPriority::UNRELIABLE_UDP);
						playerMovePacket.Serialize(playerMoveS.firstByte, playerMoveS.m_xPos, playerMoveS.m_yPos, playerMoveS.m_id);
						testPeer.UDPSendToAll(playerMovePacket);
						
						break;
					}
				}
				testPeer.FlushCurrentPacket();
				break;
			}
			}
		}

		// Going through all the players and making sure our peer has a client with a matching ID. If it doesn't, this means one of the client's has disconnected so we should remove their player.
		bool foundIDMatch = false;
		for (int i = 0; i < allPlayers.size(); i++)
		{
			if (testPeer.GetClient(allPlayers[i].m_id) == nullptr)
			{
				allPlayers.erase(allPlayers.begin() + i);
			}
		}
		
	}
	

	return 0;
}