#include <SAMP+/CRPC.h>
#include <SAMP+/client/Client.h>
#include <SAMP+/client/Network.h>
#include <SAMP+/client/CHUD.h>
#ifndef SAMPP_SAFE_CLIENT
#include <SAMP+/client/CHooks.h>
#include <SAMP+/client/CGame.h>
#include <SAMP+/client/CRPCCallback.h>
#endif

#include <RakNet/MessageIdentifiers.h>

namespace Network
{
	static CRakClient* pRakClient;
	static bool bInitialized;
	static bool bConnected;
	static bool bServerHasPlugin;
	static std::string strAddress;
	static unsigned short usPort;
#ifdef SAMPP_SAFE_CLIENT
	static bool bSafeHUDInitialized;

	static void InitializeSafeHUD()
	{
		if (bSafeHUDInitialized)
			return;

		CHUD::Initialize();
		bSafeHUDInitialized = true;
		CLog::Write("Safe HUD RPC handlers initialized");
	}

	static void ProcessSafeRPC(unsigned short usRpcId, RakNet::BitStream& bitStream)
	{
		switch (usRpcId)
		{
			case eRPC::TOGGLE_HUD_COMPONENT:
			{
				unsigned char ucComponent;
				bool bToggle;

				if (bitStream.Read(ucComponent) && bitStream.Read(bToggle))
				{
					InitializeSafeHUD();
					CLog::Write("Safe RPC ToggleHUDComponent component=%u toggle=%u", ucComponent, bToggle ? 1 : 0);
					CHUD::ToggleComponent(ucComponent, bToggle);
				}

				break;
			}
			default:
				CLog::Write("Safe RPC ignored: %u", usRpcId);
				break;
		}
	}
#endif

	void Initialize(std::string address, unsigned short port)
	{
		bInitialized = false;

		strAddress = address;
		usPort = port;

		pRakClient = new CRakClient();
		bConnected = false;
		bServerHasPlugin = false;

		if (pRakClient->Startup() != RakNet::StartupResult::RAKNET_STARTED)
			return;

		bInitialized = true;
	}

	bool IsInitialized()
	{
		return bInitialized;
	}

	void Connect()
	{
		if (IsInitialized())
			pRakClient->Connect(strAddress.c_str(), usPort, NULL);

	}

	bool IsConnected()
	{
		return bConnected;
	}

	bool ServerHasPlugin()
	{
		return bServerHasPlugin;
	}

	void Process()
	{
		if (!IsInitialized())
			return;

		RakNet::Packet* pPacket;

		while ((pPacket = pRakClient->Receive()))
		{
			if (!pPacket->length)
				return;

			RakNet::BitStream bitStream(&pPacket->data[1], pPacket->length - 1, false);

			CLog::Write("Received packet: %i, local: %i, size: %d byte(s)", pPacket->data[0], pPacket->wasGeneratedLocally, bitStream.GetNumberOfBytesUsed());

			switch (pPacket->data[0])
			{
				case ePacketType::PACKET_PLAYER_REGISTERED:
				{
					bConnected = true;
					bServerHasPlugin = true;
#ifndef SAMPP_SAFE_CLIENT
					CRPCCallback::Initialize();
#else
					CLog::Write("Safe side-channel registered; limited HUD RPC mode enabled");
#endif

					break;
				}
				case ePacketType::PACKET_RPC:
				{
					unsigned short usRpcId;

					if (bitStream.Read<unsigned short>(usRpcId))
#ifndef SAMPP_SAFE_CLIENT
						CRPC::Process(usRpcId, bitStream);
#else
						ProcessSafeRPC(usRpcId, bitStream);
#endif

					break;
				}
				case ePacketType::PACKET_CONNECTION_REJECTED:
				case ePacketType::PACKET_PLAYER_PROPER_DISCONNECT:
				{
					bServerHasPlugin = false;

					break;
				}
				case ID_DISCONNECTION_NOTIFICATION:
				case ID_CONNECTION_LOST:
				{
					bConnected = false;

					if (ServerHasPlugin())
						Connect();

					break;
				}
				default:
					break;

			}

			pRakClient->DeallocatePacket(pPacket);
			CLog::bytesReceived += bitStream.GetNumberOfBytesUsed();
		}
		
	}

	unsigned int Send(Network::ePacketType packetType, RakNet::BitStream* pBitStream, PacketPriority priority, PacketReliability reliability, char cOrderingChannel)
	{
		if (!IsConnected())
			return 0;

		CLog::Write("Sent packet: %i, size: %i byte(s)", packetType, pBitStream->GetNumberOfBytesUsed());
		CLog::bytesSent += pBitStream->GetNumberOfBytesUsed();

		return pRakClient->Send(packetType, *pRakClient->GetRemoteAddress(), pBitStream, priority, reliability, cOrderingChannel);
	}

	unsigned int SendRPC(unsigned short usRPCId, RakNet::BitStream* pBitStream, PacketPriority priority, PacketReliability reliability, char cOrderingChannel)
	{
		if (!IsConnected())
			return 0;

		CLog::Write("Sent packet: %i, size: %i byte(s)", usRPCId, pBitStream->GetNumberOfBytesUsed());
		CLog::bytesSent += pBitStream->GetNumberOfBytesUsed();

		return pRakClient->SendRPC(usRPCId, *pRakClient->GetRemoteAddress(), pBitStream, priority, reliability, cOrderingChannel);
	}

	CRakClient* GetRakClient()
	{
		return pRakClient;
	}
}
