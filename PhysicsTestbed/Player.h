#pragma once
#include "Maths.h"
#include "LineRenderer.h"

#include "CustomIdentifiers.h"


struct PlayerMoveStruct
{
	int firstByte = (int)CustomIdentifier::PLAYER_MOVE;

	float m_xPos = 0;
	float m_yPos = 0;

	int m_id = -1;

};

class Peer;
class Player
{
public:

	Player(int id, char* name, glm::vec3 colour, bool isPlayer = false);

	bool m_isPlayer = false;

	float m_xPos = 0;
	float m_yPos = 0;

	float m_radius = 0.25f; // idk seems like a good radius.

	int m_id = 0;

	char m_name[25];

	glm::vec3 m_colour;

	void Update(GLFWwindow* window, float deltaTime, Peer& peer);
	void Draw(LineRenderer& renderer);

	void Move(GLFWwindow* window, float deltaTime, Peer& peer);




};