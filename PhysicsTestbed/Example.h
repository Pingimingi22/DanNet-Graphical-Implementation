#pragma once
#include "Testbed.h"


#include <vector>

#include "Maths.h"


#include "Player.h"
#include "Peer.h"
#include <iostream>


class Example : public Testbed
{
public:

	Example();
	~Example() 
	{ 
		std::cout << "Deallocated m_myPlayer." << std::endl;
		delete m_myPlayer; 
	};

	void Update();

	void Render();

	
	std::vector<Player> m_allPlayers;
	Player* m_myPlayer = nullptr;

	Peer* testPeer = nullptr;

	char name[25];
};