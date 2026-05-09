#include <SAMP+/client/CRakClient.h>

CRakClient::CRakClient()
{
	m_pPeer = new RakNet::RakPeer();
	m_pSocketDescriptor = new RakNet::SocketDescriptor();
	m_pSystemAddress = NULL;
	m_bConnected = false;
}

CRakClient::~CRakClient()
{
	Shutdown(0);
	delete m_pPeer;
	delete m_pSocketDescriptor;
	delete m_pSystemAddress;
}

RakNet::StartupResult CRakClient::Startup()
{
	return m_pPeer->Startup(1, m_pSocketDescriptor, 1, THREAD_PRIORITY_NORMAL);
}

void CRakClient::Shutdown(int iBlockDuration)
{
	Disconnect();

	if (m_pPeer)
		m_pPeer->Shutdown(iBlockDuration);
}

RakNet::ConnectionAttemptResult CRakClient::Connect(const char* szAddress, unsigned short usPort, const char* szPassword)
{
	Disconnect();
	delete m_pSystemAddress;

	m_pSystemAddress = new RakNet::SystemAddress(szAddress, usPort);

	return m_pPeer->Connect(szAddress, usPort, szPassword, szPassword ? strlen(szPassword) : NULL);
}

RakNet::SystemAddress* CRakClient::GetRemoteAddress()
{
	return m_pSystemAddress;
}

unsigned int CRakClient::Send(Network::ePacketType packetType, const RakNet::SystemAddress& systemAddress, RakNet::BitStream* pBitStream, PacketPriority priority, PacketReliability reliability, char cOrderingChannel)
{
	RakNet::BitStream bitStream;
	bitStream.Write((unsigned char)packetType);
	if (pBitStream)
		bitStream.Write((char*)pBitStream->GetData(), pBitStream->GetNumberOfBytesUsed());

	return m_pPeer->Send(&bitStream, priority, reliability, cOrderingChannel, systemAddress, false);
}

unsigned int CRakClient::SendRPC(unsigned short usRPCId, const RakNet::SystemAddress& systemAddress, RakNet::BitStream* pBitStream, PacketPriority priority, PacketReliability reliability, char cOrderingChannel)
{
	RakNet::BitStream bitStream;
	bitStream.Write(usRPCId);
	if (pBitStream)
		bitStream.Write((char*)pBitStream->GetData(), pBitStream->GetNumberOfBytesUsed());

	return Send(Network::ePacketType::PACKET_RPC, systemAddress, &bitStream, priority, reliability, cOrderingChannel);
}

void CRakClient::Disconnect()
{
	if (IsConnected())
	{
		m_pPeer->CloseConnection(*m_pSystemAddress, true);
		delete m_pSystemAddress;
		m_pSystemAddress = NULL;
		m_bConnected = false;
	}
}

RakNet::Packet* CRakClient::Receive()
{
	return 	m_pPeer->Receive();
}

void CRakClient::DeallocatePacket(RakNet::Packet* pPacket)
{
	return m_pPeer->DeallocatePacket(pPacket);
}

//TODO: cancel connection attempt

bool CRakClient::IsConnected()
{
	return m_bConnected;
}
