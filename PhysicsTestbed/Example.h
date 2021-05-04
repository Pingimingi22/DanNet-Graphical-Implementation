#pragma once

#include "Testbed.h"

#include <vector>

#include "Maths.h"

#include "Player.h"

#include "Peer.h"



class Example : public Testbed
{
public:

	Example();

	void Update();

	void Render();

	
	std::vector<Player> m_allPlayers;
	Player* m_myPlayer = nullptr;

	Peer* testPeer = nullptr;

	char name[25];
};