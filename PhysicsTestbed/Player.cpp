#include "Testbed.h"
#include "Player.h"

#include "DanNet.h"

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
}

void Player::Draw(LineRenderer& renderer)
{
	renderer.DrawCircle(glm::vec2(m_xPos, m_yPos), m_radius, m_colour);
}

/// <summary>
/// Move() is a polling function that is called in the player's Update() function. It sends movement packets to the server.
/// </summary>
/// <param name="window">Reference to glfw window.</param>
/// <param name="deltaTime">Deltatime of program</param>
/// <param name="peer">User's local peer.</param>
void Player::Move(GLFWwindow* window, float deltaTime, Peer& peer)
{

	if (m_isPlayer)
	{
		if (glfwGetKey(window, GLFW_KEY_A))
		{
			m_xPos -= 0.5f * deltaTime;
			
			PlayerMoveStruct playerMoveS;
			playerMoveS.m_id = peer.GetId();
			playerMoveS.m_xPos = m_xPos;
			playerMoveS.m_yPos = m_yPos;


			Packet packet(PacketPriority::UNRELIABLE_UDP);
			packet.Serialize(playerMoveS.firstByte, playerMoveS.m_xPos, playerMoveS.m_yPos, playerMoveS.m_id);
			peer.UDPSend(packet);

		}
		if (glfwGetKey(window, GLFW_KEY_D))
		{
			m_xPos += 0.5f * deltaTime;

			PlayerMoveStruct playerMoveS;
			playerMoveS.m_id = peer.GetId();
			playerMoveS.m_xPos = m_xPos;
			playerMoveS.m_yPos = m_yPos;

			Packet packet(PacketPriority::UNRELIABLE_UDP);
			packet.Serialize(playerMoveS.firstByte, playerMoveS.m_xPos, playerMoveS.m_yPos, playerMoveS.m_id);
			peer.UDPSend(packet);

		}
		if (glfwGetKey(window, GLFW_KEY_W))
		{
			m_yPos += 0.5f * deltaTime;

			PlayerMoveStruct playerMoveS;
			playerMoveS.m_id = peer.GetId();
			playerMoveS.m_xPos = m_xPos;
			playerMoveS.m_yPos = m_yPos;

			Packet packet(PacketPriority::UNRELIABLE_UDP);
			packet.Serialize(playerMoveS.firstByte, playerMoveS.m_xPos, playerMoveS.m_yPos, playerMoveS.m_id);
			peer.UDPSend(packet);

		}
		if (glfwGetKey(window, GLFW_KEY_S))
		{
			m_yPos -= 0.5f * deltaTime;

			PlayerMoveStruct playerMoveS;
			playerMoveS.m_id = peer.GetId();
			playerMoveS.m_xPos = m_xPos;
			playerMoveS.m_yPos = m_yPos;

			
			Packet packet(PacketPriority::UNRELIABLE_UDP);
			packet.Serialize(playerMoveS.firstByte, playerMoveS.m_xPos, playerMoveS.m_yPos, playerMoveS.m_id);
			peer.UDPSend(packet);

		}
	}
}
