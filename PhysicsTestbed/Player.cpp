#include "Player.h"

#include "Testbed.h"

#include "Packet.h"
#include "Peer.h"

#include "teststructheaderdelete.h"

Player::Player(int id, char* name, glm::vec3 colour, bool isPlayer)
{
	m_id = id;
	m_colour = colour;
	m_isPlayer = isPlayer;

	strcpy_s(m_name, name);
}

void Player::Update(GLFWwindow* window, float deltaTime, Peer& peer)
{
	Move(window, deltaTime, peer);

	//std::cout << "X:" << m_xPos << " Y:" << m_yPos << std::endl;
}

void Player::Draw(LineRenderer& renderer)
{
	renderer.DrawCircle(glm::vec2(m_xPos, m_yPos), m_radius);
}

void Player::Move(GLFWwindow* window, float deltaTime, Peer& peer)
{

	if (m_isPlayer)
	{
		if (glfwGetKey(window, GLFW_KEY_A)/* == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS*/)
		{
			m_xPos -= 0.5f * deltaTime;
			
			PlayerMoveStruct playerMoveS;
			playerMoveS.m_id = peer.GetId();
			playerMoveS.m_xPos = m_xPos;
			playerMoveS.m_yPos = m_yPos;


			
			

			Packet packet;
			packet.Serialize(playerMoveS.firstByte, playerMoveS.m_xPos, playerMoveS.m_yPos, playerMoveS.m_id);
			peer.UDPSend(packet);

			if (playerMoveS.m_xPos == 0)
			{
				assert(true);
				//std::cout << "error" << std::endl;
			}
	
		}
		if (glfwGetKey(window, GLFW_KEY_D)/* == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS*/)
		{
			m_xPos += 0.5f * deltaTime;

			PlayerMoveStruct playerMoveS;
			playerMoveS.m_id = peer.GetId();
			playerMoveS.m_xPos = m_xPos;
			playerMoveS.m_yPos = m_yPos;

			Packet packet;
			packet.Serialize(playerMoveS.firstByte, playerMoveS.m_xPos, playerMoveS.m_yPos, playerMoveS.m_id);
			peer.UDPSend(packet);

			if (playerMoveS.m_xPos == 0)
			{
				assert(true);
				//std::cout << "error" << std::endl;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_W)/* == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS*/)
		{
			m_yPos += 0.5f * deltaTime;

			PlayerMoveStruct playerMoveS;
			playerMoveS.m_id = peer.GetId();
			playerMoveS.m_xPos = m_xPos;
			playerMoveS.m_yPos = m_yPos;

			Packet packet;
			packet.Serialize(playerMoveS.firstByte, playerMoveS.m_xPos, playerMoveS.m_yPos, playerMoveS.m_id);
			peer.UDPSend(packet);

			if (playerMoveS.m_xPos == 0)
			{
				assert(true);
				//std::cout << "error" << std::endl;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_S)/* == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS*/)
		{
			m_yPos -= 0.5f * deltaTime;

			PlayerMoveStruct playerMoveS;
			playerMoveS.m_id = peer.GetId();
			playerMoveS.m_xPos = m_xPos;
			playerMoveS.m_yPos = m_yPos;

			Packet packet;
			packet.Serialize(playerMoveS.firstByte, playerMoveS.m_xPos, playerMoveS.m_yPos, playerMoveS.m_id);
			peer.UDPSend(packet);

			if (playerMoveS.m_xPos == 0)
			{
				assert(true);
				//std::cout << "error" << std::endl;
			}
		}
	}
}
