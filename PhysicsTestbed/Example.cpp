#include "Example.h"

#include <vector>

#include "Player.h"

#include "Peer.h"
#include "MessageIdentifiers.h"
#include "Packet.h"
// testing player
#include "CustomIdentifiers.h"


struct PlayerCreateStruct
{
	int firstByte = (int)CustomIdentifier::PLAYER_CREATE;

	float m_xPos = 0;
	float m_yPos = 0;

	int m_id = -1;

	char name[25];
	
};


Example::Example() : Testbed()
{
	testPeer = new Peer();
	testPeer->StartPeer();

	char input[25];
	std::cin >> input;
	strcpy_s(name, input);

	//Your initialisation code goes here!
	testPeer->Connect("192.168.1.124", 25565);
	//m_allPlayers.push_back(Player(testPeer->GetId(), glm::vec3(0, 1, 0), true));
}

void Example::Update()
{
	//This call ensures that your mouse position and aspect ratio are maintained as correct.
	Testbed::Update();

	// if we have successfully connected but we don't have a player, we will make a player. this is really dodgy and not how I would do it for a real game but I think it works fine for this little demo.
	if (testPeer->GetId() != -1 && m_myPlayer == nullptr)
	{
		m_myPlayer = new Player(testPeer->GetId(), name, glm::vec3(0, 1, 0), true); // =================================== WARNING ================================= this player is on the heap and we are not deleting it!

		Packet playerCreationPacket;
		PlayerCreateStruct playerCreateS;
		playerCreateS.m_id = testPeer->GetId();
		strcpy_s(playerCreateS.name, name);

		playerCreationPacket.Serialize(playerCreateS.firstByte, playerCreateS.m_xPos, playerCreateS.m_yPos, playerCreateS.m_id, playerCreateS.name);
		testPeer->UDPSend(playerCreationPacket);

		std::cout << "our player created: " << playerCreateS.m_id << " name: " << name << std::endl;
	}


	// packet checking update.
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
		default:
			std::cout << "Received weird message" << std::endl;
			testPeer->FlushCurrentPacket();
			break;
		}
	}




	for (int i = 0; i < m_allPlayers.size(); i++)
	{
		m_allPlayers[i].Update(window, deltaTime, *testPeer);
	}
	if (m_myPlayer != nullptr)
	{
		m_myPlayer->Update(window, deltaTime, *testPeer);
	}

	//Your physics (or whatever) code goes here!


}

void Example::Render()
{
	for (int i = 0; i < m_allPlayers.size(); i++)
	{
		m_allPlayers[i].Draw(lines);
	}
	if (m_myPlayer != nullptr)
	{
		m_myPlayer->Draw(lines);
	}

	//Example code that draws a blue square.
	//std::vector<glm::vec2> points;
	//points.push_back({ 1.5f, 1.5f });
	//points.push_back({ 1.5f, 2.5f });
	//points.push_back({ 2.5f, 2.5f });
	//points.push_back({ 2.5f, 1.5f });
	//
	//lines.DrawRawShape((float*)points.data(), (unsigned int) points.size(), glm::vec3(0, 0, 1));


	//Example code that draws a coloured circle at the mouse position, whose colour depends on which buttons are down.
	if (leftButtonDown)
	{
		//lines.DrawCircle(cursorPos, 0.2f, { 1, 0, 0 });
	}
	else if (rightButtonDown)
	{
		//lines.DrawCircle(cursorPos, 0.2f, { 0, 1, 0 });
	}
	else
	{
		//lines.DrawCircle(cursorPos, 0.2f, { 0, 0, 1 });
	}





	//Your drawing code goes here!




	//This call puts all the lines you've set up on screen - don't delete it or things won't work.
	Testbed::Render();
}