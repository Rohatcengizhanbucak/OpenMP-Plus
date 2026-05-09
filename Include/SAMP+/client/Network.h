#pragma once

#include <SAMP+/Network.h>
#include <SAMP+/client/CRakClient.h>

namespace Network
{
	void Initialize(std::string address, unsigned short port);
	void InitializeNative();
	bool IsInitialized();
	void Connect();
	bool IsConnected();
	bool ServerHasPlugin();
	void Process();
	void Shutdown();
	void HandleNativeRPC(unsigned char* data, unsigned int numberOfBits);
	unsigned int Send(Network::ePacketType packetType, RakNet::BitStream* pBitStream = NULL, PacketPriority priority = PacketPriority::HIGH_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, char cOrderingChannel = 0x7F);
	unsigned int SendRPC(unsigned short usRPCId, RakNet::BitStream* pBitStream = NULL, PacketPriority priority = PacketPriority::HIGH_PRIORITY, PacketReliability reliability = PacketReliability::RELIABLE_ORDERED, char cOrderingChannel = 0x7F);
	CRakClient* GetRakClient();
}
