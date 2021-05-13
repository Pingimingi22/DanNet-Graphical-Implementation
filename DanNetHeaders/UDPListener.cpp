#include "UDPListener.h"
#include <iostream>

#include "Packet.h"

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"

#include "Peer.h"

#include "CorePackets.h"
#include <mutex>

// DELETE THIS LATER.
struct TestStruct
{
	int hello = 0;
	int goodbye = 0;
	int test1 = 0;
	int test2 = 0;
	int test3 = 0;
};

UDPListener::UDPListener(Peer* attachedPeer, std::string portNumber, std::string ipAddress)
{
	m_attachedPeer = attachedPeer;

	sockaddr_in hostAddress;
	memset(&hostAddress, 0, sizeof(sockaddr_in));

	hostAddress.sin_family = AF_INET;

	// --------------- Whether they pass in a port or not kind of defines whether they want a client or a server. ------------------ //
	// If they pass a port in, that means they want their specific application to be listenining on a special "known" port. Which is what servers usually do.
	// However, if they don't set a port, it probably means they want to listen in general to any incoming messages, like what a client would do.
	// ----------------------------------------------------------------------------------------------------------------------------- //

	try
	{
		USHORT port = std::stoi(portNumber);
		hostAddress.sin_port = htons(std::stoi(portNumber.c_str()));
	}
	catch (...)
	{
		// this means the passed in port wasn't able to convert. For now we'll just make it so the socket picks an ephemeral port to listen on, thereby making it a client.
		// also we wont bother setting the port to a wildcard thing because I think if I just leave that part of the sockaddr_in struct empty, the kernel will automatically chose an emphemeral port.

		// ok i think i need to say 0 for the port to get an ephemeral port.
		hostAddress.sin_port = 0;
	}


	// ----------------------- If they've passed in an approriate IPv4 address, we try convert it. ----------------------- //
	// Also, they probably wont ever wont to pass in an IP address as that limits the socket to what it can listen to.
	// Usually you want the kernal to chose an appropriate IP address for the socket.
		
	int ipConversionResult = inet_pton(AF_INET, ipAddress.c_str(), &hostAddress.sin_addr.S_un.S_addr);                    // The reason we don't have an else statement initialising the variable is
	if (ipConversionResult == 0 || ipConversionResult == -1)															  // because inet_pton() has an out return param that does it for us.
	{
		std::cout << "UDPListener received an address that was not able to be converted." << std::endl;
		hostAddress.sin_addr.S_un.S_addr = INADDR_ANY;
	}


	// ---------------------- Creating the host socket. ---------------------- // 
	m_hostSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_hostSocket == INVALID_SOCKET)
	{
		std::cout << "Error on UDPListener socket()." << std::endl;
	}
	// ------------------------- Binding the socket. ------------------------- //
	int result = bind(m_hostSocket, (sockaddr*)&hostAddress, sizeof(hostAddress));
	if (result == -1)
	{
		std::cout << "Error on UDPListener bind()" << std::endl;
	}

	
	//memset(&m_tempClientSize, 0, sizeof(sockaddr)); =============================== NOTE THIS MAY HAVE BEEN IMPORTANT =============================== 

	
	// ------------------------ Zeroing out and setting up the FD sets. ------------------------ // 
	FD_ZERO(&m_master);
	FD_ZERO(&m_readReady);

	FD_SET(m_hostSocket, &m_master);


	DisplaySettings();


	
}

void UDPListener::Start()
{
	m_isRunning = true;
}

void UDPListener::Close()
{
	m_isRunning = false;
	closesocket(m_hostSocket);
	std::cout << "Closed hostSocket on a UDPListener." << std::endl;
}

void UDPListener::Update()
{
	
	// Probably not the optimal place to have this since this is the "listener" but I think it'll work fine. maybe.
	m_attachedPeer->UpdateReliableSends();

	

	m_readReady = m_master;
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	if (select(m_hostSocket, &m_readReady, NULL, NULL, &tv) == -1)
	{
		std::cerr << "select() error." << std::endl;
	}
	
	if (FD_ISSET(m_hostSocket, &m_readReady))
	{
		//std::cout << "Update test" << std::endl;

		char recvBuffer[256];
		Packet* incomingPacket = new Packet();


		// temporary cache of incoming client address.
		sockaddr_in incomingClientAddress;
		int incomingClientSize;
		incomingClientSize = sizeof(sockaddr_in); 

		int result = recvfrom(m_hostSocket, &incomingPacket->m_allBytes[0], 256, 0, (sockaddr*)&incomingClientAddress, &incomingClientSize);
		if (result > 0)
		{
			//std::cout << "Received message." << std::endl;
			//std::string receivedString = recvBuffer;
			//
			//std::stringstream ss;
			//ss.write(&incomingPacket.m_allBytes[0], 1024);
			//cereal::BinaryInputArchive iarchive(ss);
			//
			//TestStruct testReadingIn;
			//iarchive(testReadingIn.hello, testReadingIn.goodbye);
			//
			//int hi = 5;
			//float byte = 3;
			//char alpha = 'a';
			//int idk1 = 2;
			//int idk2 = 1; 
			//float fin = 0;
			//incomingPacket.Deserialize(hi, byte, alpha, idk1, idk2, fin);
			
			//std::cout << "Received a message." << std::endl;
			//
			//TestStruct testingReadingIn;
			//incomingPacket->Deserialize(testingReadingIn.hello, testingReadingIn.goodbye, testingReadingIn.test1, testingReadingIn.test2, testingReadingIn.test3); // mwhahaha my multiple param any type verdaic function ! >:)
			//std::cout << testingReadingIn.goodbye;

			// We check every incoming packet's first byte. If they are sending a CorePacket we deal with it here so the user doesn't have to.
			// Commented this out because we no longer are going to be using the incoming packet as a thing to process. what we want to do instead is process the top of the m_packetQueue which is down below.
			//switch ((int)incomingPacket->GetPacketIdentifier())
			//{
			//case (int)MessageIdentifier::CONNECT:
			//	m_attachedPeer->AddClient(incomingClientAddress);
			//	std::cout << "Received connect packet. Attempting to add client." << std::endl;
			//	//return;
			//	break;
			//case (int)MessageIdentifier::ACK_CONNECT:
			//{
			//	ACKConnection AC;
			//	incomingPacket->Deserialize(AC.firstByte, AC.clientID, AC.port);
			//	m_attachedPeer->m_ID = AC.clientID;
			//	std::cout << "Server has acknowledged our connection. Our client ID is: " << m_attachedPeer->m_ID << "." << std::endl;
			//	break;
			//}
			//default:
			//	break;
			//}


			// Now that we've added super secret internal header's to each packet, we have to remove them before we do anything else.
			// =============================== REMOVING SUPER SERCRET INTERNAL HEADERS =============================== //
			int incomingPriority;
			GUID incomingGuid;

			incomingPacket->InternalHeaderDeserialize(incomingPriority, incomingGuid);

			incomingPacket->m_priority = (PacketPriority)incomingPriority;

			if ((PacketPriority)incomingPriority == PacketPriority::RELIABLE_UDP) // we need to send an acknowledgement back to whoever sent us this.
			{
				if (m_attachedPeer->m_packetQueue.size() <= 1) // Only want to acknowledge packet's if we can actually store them.
				{
					Packet ackPacket = Packet((int)PacketPriority::UNRELIABLE_UDP, incomingGuid); // the acknowledgement doesn't have to be reliable because if the client doesn't receive it, 
					                                                                         // it will ask for another one anyway.
					int ackPackIdentifier = (int)MessageIdentifier::RELIABLE_UDP_ACK;
					ackPacket.Serialize(ackPackIdentifier);
					char incomingIPString[15]; 
					inet_ntop(AF_INET, &incomingClientAddress.sin_addr.S_un.S_addr, &incomingIPString[0], 25);
					unsigned short incomingPort = ntohs(incomingClientAddress.sin_port);
					SendTo(ackPacket, incomingIPString, incomingPort);
				}
			}
			// ======================================================================================================= //


			
			//MessageIdentifier testingIdentifier = incomingPacket->GetPacketIdentifier();
			//std::cout << std::endl;
			//std::cout << "Received a packet with an identifier of: " << (int)incomingPacket->GetPacketIdentifier() << std::endl;
			//std::cout << std::endl;

			if (m_attachedPeer->m_currentPacket != nullptr)
			{
				// ==================================== NOTE ======================================
				// this was a cool idea being able to clear current packets with incoming packets but it causes issues and i plan on replacing it with a packet queue system.
				// ================================================================================

				//std::lock_guard<std::mutex> guard(*m_attachedPeer->m_packetMutex);
				//delete m_attachedPeer->m_currentPacket;
				//m_attachedPeer->m_currentPacket = nullptr;                // This is the only place I'm freeing up the memory of m_currentPacket. So there wont be that bad of a memory leak since every time we receive
			}															  // a new packet, it will delete the old one.

			
			//m_attachedPeer->m_currentPacket = incomingPacket; // ------------------------> Telling the attached peer that we have received a packet. The user can do what they like with it.
			//if(m_attachedPeer->m_packetQueue.size() <= 1)
			//	m_attachedPeer->m_packetQueue.push_back(incomingPacket);

			switch (/*m_attachedPeer->m_packetQueue[0]*/incomingPacket->GetPacketIdentifier())
			{
				// hopefully only server's only ever send RELIABLE_UDP_ACK's. this way we can ensure we are dealing with the client in this case.
			case MessageIdentifier::RELIABLE_UDP_ACK: // this is for when the client receives an ACK and wants to verify if the ack's sequence number matches any of it's reliable packet sequence numbers.
			{
				if (m_attachedPeer->m_packetQueue.size() <= 1)
				{
					bool guidMatches = true;
					for (int i = 0; i < m_attachedPeer->m_reliablePackets.size(); i++)
					{
						if (m_attachedPeer->m_reliablePackets[i].m_guid.Data1 == incomingGuid.Data1 &&
							m_attachedPeer->m_reliablePackets[i].m_guid.Data2 == incomingGuid.Data2 &&
							m_attachedPeer->m_reliablePackets[i].m_guid.Data3 == incomingGuid.Data3)
						{
							// now to check the hexedecimal part of the guid.
							for (int j = 0; j < 8; j++)
							{
								if (m_attachedPeer->m_reliablePackets[i].m_guid.Data4[j] != incomingGuid.Data4[j])
								{
									guidMatches = false;
									break;
								}
							}
							if (guidMatches)
							{
								// The incoming guid matches one of the client's reliable packet guid's.

								// I guess we can clear the reliable packet from the client since we know the server received the message.
								//m_attachedPeer->m_reliablePackets[i];
								int printIdentifier = (int)m_attachedPeer->m_reliablePackets[i].GetPacketIdentifier();
								std::cout << std::endl;
								std::cout << "======================================" << std::endl;
								std::cout << "Reliable UDP packet with identifier of " << printIdentifier << " was acknowledged and GUID of ." << m_attachedPeer->m_reliablePackets[i].m_guid.Data1 << ", " << m_attachedPeer->m_reliablePackets[i].m_guid.Data2 << ", " << m_attachedPeer->m_reliablePackets[i].m_guid.Data3;
								for (int x = 0; x < 8; x++)
								{
									std::cout << "," << m_attachedPeer->m_reliablePackets[i].m_guid.Data4[x];
								}
								std::cout << std::endl;
								std::cout << "======================================" << std::endl;


								std::lock_guard<std::mutex> guard(*m_attachedPeer->m_reliablePacketMutex.get()); // Maybe we are reading in and erasing at the same time and that might be what is causing issues.
								m_attachedPeer->m_reliablePackets.erase(m_attachedPeer->m_reliablePackets.begin() + i); // guess this is why containers start at 0, so you can do this cool trick.


								// Removing it from the packet queue so user's don't have to deal with this type of packet.
								//delete m_attachedPeer->m_packetQueue[0];
								//m_attachedPeer->m_packetQueue.erase(m_attachedPeer->m_packetQueue.begin());

								// =================================== WARNING =================================== //
								// Maybe we shouldn't delete packets from the queue here, since the message identifier checks delete packets we don't want to accidentally try to delete something twice.
							}

						}
						//else
						//{
						//	std::cout << "Received a reliable UDP ACK with identifier of " << m_but the GUID matches no known GUID!" << std::endl;
						//}
					}
					if (!guidMatches) // this means no guid matched the ack which is a pretty bad error.
					{
						std::cout << "Received a reliable UDP ACK that didn't match any known GUID!" << std::endl;
					}
				}
				break;
			}
			case MessageIdentifier::CONNECT:
				if (m_attachedPeer->m_packetQueue.size() <= 1)
				{
					std::cout << std::endl;
					std::cout << "=============================== NEW CLIENT ATTEMPTING TO CONNECT =============================== " << std::endl;
					m_attachedPeer->AddClient(incomingClientAddress);
					//std::cout << "Received connect packet. Attempting to add client." << std::endl;
					//delete m_attachedPeer->m_packetQueue[0];
					//m_attachedPeer->m_packetQueue.erase(m_attachedPeer->m_packetQueue.begin());
					//return;
				}
				break;
			case MessageIdentifier::ACK_CONNECT:
			{
				if (m_attachedPeer->m_packetQueue.size() <= 1)
				{
					ACKConnection AC;
					incomingPacket->Deserialize(AC.firstByte, AC.clientID, AC.port);
					m_attachedPeer->m_ID = AC.clientID;
					std::cout << "Server has acknowledged our connection. Our client ID is: " << m_attachedPeer->m_ID << "." << std::endl;
					//delete m_attachedPeer->m_packetQueue[0];
					//m_attachedPeer->m_packetQueue.erase(m_attachedPeer->m_packetQueue.begin());
				}
				break;
			}
			default:
				if (m_attachedPeer->m_packetQueue.size() <= 1)
				{
					m_attachedPeer->m_packetQueue.push_back(incomingPacket);
					MessageIdentifier testingIdentifier = incomingPacket->GetPacketIdentifier();
					std::cout << std::endl;
					std::cout << "Received a non core packet with an identifier of: " << (int)incomingPacket->GetPacketIdentifier() << std::endl;
					std::cout << std::endl;
				}
				break;
			}


			//incomingPacket.Write(1024);
			//
			//MessageIdentifier packetIdentifier = incomingPacket.GetPacketIdentifier();
			//std::cout << "Message packet identifier: " << packetIdentifier << std::endl;
		}
		else if (result == -1)
		{
			// ==================================== NOTE ======================================
			// this was a cool idea being able to clear current packets with incoming packets but it causes issues and i plan on replacing it with a packet queue system.
			// ================================================================================
			delete incomingPacket;
			m_attachedPeer->m_currentPacket = nullptr; // have to set that to nullptr so that way it flushes out the old packet.
			//
			//std::cerr << "UDPListener recvfrom() error." << std::endl;

		}
		else if (result == 0)
		{
			// ==================================== NOTE ======================================
			// this was a cool idea being able to clear current packets with incoming packets but it causes issues and i plan on replacing it with a packet queue system.
			// ================================================================================
			delete incomingPacket;
			m_attachedPeer->m_currentPacket = nullptr;
			//
			//// i'm not exactly sure what 0 means but i do know that it means we havn't received any bytes.
		}
	}
	else
	{
		// if we havn't received anything.
		//m_attachedPeer->m_currentPacket = nullptr;


		//std::cout << "UDPListener timeout." << std::endl;
	}

}

void const UDPListener::Receive(const char* buffer)
{
	//return nullptr;
}

void UDPListener::Send(Packet& packet)
{
	int sendResult = sendto(m_hostSocket, packet.m_allBytes, 256, 0, (sockaddr*)&m_attachedPeer->m_serverConnection, sizeof(sockaddr_in));
	if (sendResult == -1)
	{
		std::cout << "An error occured when trying to send a message." << std::endl;
	}
	else if (sendResult > 0)
	{
		std::cout << "Successfully sent out [" << sendResult << "] bytes." << std::endl;
	}
}

//void UDPListener::SendReliable(Packet packet)
//{
//}

void UDPListener::SendTo(Packet& packet, char* ipAddress, unsigned short port)
{
	sockaddr_in recipientAddress;
	recipientAddress.sin_family = AF_INET;
	inet_pton(AF_INET, &ipAddress[0], &recipientAddress.sin_addr.S_un.S_addr);
	recipientAddress.sin_port = htons(port);

	int sendResult = sendto(m_hostSocket, packet.m_allBytes, 256, 0, (sockaddr*)&recipientAddress, sizeof(sockaddr_in));

	if (sendResult == -1)
	{
		std::cout << "An error occured when trying to send the message." << std::endl;
	}
	else if (sendResult > 0)
	{
		std::cout << "Successfully sent message to. " << ipAddress << ", " << port << " Bytes sent [" << sendResult << "]." << std::endl;
	}
}

void UDPListener::DisplaySettings()
{
	sockaddr_in tempAddress;
	int tempAddressSize = sizeof(sockaddr_in);
	getsockname(m_hostSocket, (sockaddr*)&tempAddress, &tempAddressSize); // Getting the information from the host socket so that we can display it to the user.

	char thisIPString[256];
	inet_ntop(AF_INET, (sockaddr*)&tempAddress.sin_addr.S_un.S_addr, &thisIPString[0], 256);

	std::cout << "======================== SETTINGS ========================" << std::endl;
	std::cout << "IP Address: " << thisIPString << std::endl;
	std::cout << "Port Number: " << ntohs(tempAddress.sin_port) << std::endl;
	std::cout << "=================================================================" << std::endl;
}

bool UDPListener::IsRunning()
{
	if (m_isRunning)
		return true;
	return false;
}
