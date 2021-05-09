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

int main()
{
	std::vector<Player> allPlayers;

	Peer testPeer(true, 25565);
	testPeer.StartPeer();

	bool isRunning = true;




	// testing serialization.
	Packet testPacket = Packet((int)PacketPriority::RELIABLE_UDP);
	int testNum = 95;
	testPacket.Serialize(testNum);

	int thingWeDontCareAbout = 0;
	GUID guidWeDontCareAbout;
	int deserializeTestNum = 0;

	testPacket.InternalHeaderDeserialize(thingWeDontCareAbout, guidWeDontCareAbout);
	testPacket.Deserialize(deserializeTestNum);
	//-----------------------------------







	Packet* incomingPacket = nullptr;
	while (isRunning)
	{
		incomingPacket = testPeer.UDPReceivePacket();
		if (incomingPacket != nullptr)
		{
			//std::cout << "Packet identifier: " << (int)incomingPacket->GetPacketIdentifier() << std::endl;
			//std::cout << "Hello world." << std::endl;
			//std::this_thread::sleep_for(std::chrono::milliseconds(100));
			MessageIdentifier testIdentifier = incomingPacket->GetPacketIdentifier();
			switch (incomingPacket->GetPacketIdentifier())
			{
			case (MessageIdentifier)CustomIdentifier::PLAYER_CREATE:
			{
				PlayerCreateStruct playerCreateS;
				incomingPacket->Deserialize(playerCreateS.firstByte, playerCreateS.m_xPos, playerCreateS.m_yPos, playerCreateS.m_id, playerCreateS.name);

				Packet playerCreatePacket((int)PacketPriority::RELIABLE_UDP);
				playerCreatePacket.Serialize(playerCreateS.firstByte, playerCreateS.m_xPos, playerCreateS.m_yPos, playerCreateS.m_id, playerCreateS.name);

				//PlayerCreateStruct testdelete;
				//playerCreatePacket.Deserialize(testdelete.firstByte, testdelete.m_xPos, testdelete.m_yPos, testdelete.m_id, testdelete.name);

				testPeer.UDPSendToAll(playerCreatePacket);


				// telling the newly created player about all the other player's who joined before him.

				ClientStruct client;
				client = testPeer.GetClient(playerCreateS.m_id);

				for (int i = 0; i < allPlayers.size(); i++)
				{
					
					PlayerCreateStruct anotherPlayerCreateS;
					anotherPlayerCreateS.m_id = allPlayers[i].m_id;
			
					strcpy_s(anotherPlayerCreateS.name, allPlayers[i].name);
					Packet testPacket((int)PacketPriority::RELIABLE_UDP);
					testPacket.Serialize(anotherPlayerCreateS.firstByte, anotherPlayerCreateS.m_xPos, anotherPlayerCreateS.m_yPos, anotherPlayerCreateS.m_id, anotherPlayerCreateS.name);
					testPeer.UDPSendTo(testPacket, client.m_ipAddress, client.m_port);
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

				if (playerMoveS.m_xPos == 0)
				{
					//assert(playerMoveS.m_xPos != 0);
					std::cout << "test" << std::endl;
				}

				for (int i = 0; i < allPlayers.size(); i++)
				{
					if (allPlayers[i].m_id == playerMoveS.m_id) // finding that specific player and updating their position.
					{
						allPlayers[i].m_xPos = playerMoveS.m_xPos;
						allPlayers[i].m_yPos = playerMoveS.m_yPos;
						Packet playerMovePacket((int)PacketPriority::UNRELIABLE_UDP);
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
	}
	

	return 0;
}