#include <SAMP+/CRPC.h>
#include <SAMP+/client/Client.h>
#include <SAMP+/client/CBuildManager.h>
#include <SAMP+/client/CKeyBinds.h>
#include <SAMP+/client/CGameRakClient.h>
#include <SAMP+/client/CRmlUiManager.h>
#include <SAMP+/client/CTargetManager.h>
#include <SAMP+/client/Network.h>
#include <SAMP+/client/CHUD.h>
#ifndef SAMPP_SAFE_CLIENT
#include <SAMP+/client/CHooks.h>
#include <SAMP+/client/CGame.h>
#endif

#include <RakNet/MessageIdentifiers.h>

#include <stdio.h>

namespace Network
{
	enum eTransportMode
	{
		TRANSPORT_DISABLED,
		TRANSPORT_SIDE_CHANNEL,
		TRANSPORT_NATIVE_RAKCLIENT
	};

	static CRakClient* pRakClient;
	static bool bInitialized;
	static bool bConnected;
	static bool bServerHasPlugin;
	static eTransportMode transportMode = TRANSPORT_DISABLED;
	static DWORD dwNextHelloAttempt;
	static DWORD dwNextNativeInitAttempt;
	static DWORD dwNextInvalidNativeLog;
	static std::string strAddress;
	static unsigned short usPort;
	static bool bSafeHUDInitialized;

	void Shutdown();

	static bool ReadSafeString(RakNet::BitStream& bitStream, std::string& value)
	{
		unsigned char length;
		if (!bitStream.Read(length) || length > 31)
			return false;

		char buffer[32] = { 0 };
		if (length && !bitStream.Read(buffer, length))
			return false;

		value.assign(buffer, length);
		return true;
	}

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
			case eRPC::SET_KEY_BIND:
			{
				unsigned short key;
				unsigned char eventMask;
				std::string action;

				if (bitStream.Read(key) && bitStream.Read(eventMask) && ReadSafeString(bitStream, action))
					CKeyBinds::Bind(key, eventMask, action);

				break;
			}
			case eRPC::UNBIND_KEY:
			{
				unsigned short key;

				if (bitStream.Read(key))
					CKeyBinds::Unbind(key);

				break;
			}
			case eRPC::CLEAR_KEY_BINDS:
			{
				CKeyBinds::Clear();
				break;
			}
			case eRPC::SET_KEY_CAPTURE:
			{
				unsigned short key;
				unsigned char eventMask;
				unsigned char flags;
				short priority;
				unsigned short ttlMs;
				std::string action;

				if (bitStream.Read(key)
					&& bitStream.Read(eventMask)
					&& bitStream.Read(flags)
					&& bitStream.Read(priority)
					&& bitStream.Read(ttlMs)
					&& ReadSafeString(bitStream, action))
				{
					CKeyBinds::BeginCapture(key, eventMask, flags, priority, ttlMs, action);
				}
				break;
			}
			case eRPC::CLEAR_KEY_CAPTURE:
			{
				unsigned short key;
				std::string action;

				if (bitStream.Read(key) && ReadSafeString(bitStream, action))
					CKeyBinds::EndCapture(key, action);

				break;
			}
			case eRPC::CLEAR_KEY_CAPTURES:
			{
				CKeyBinds::ClearCaptures();
				break;
			}
			case eRPC::TARGET_SET_CONTEXT:
			{
				CTargetManager::HandleSetContext(bitStream);
				break;
			}
			case eRPC::TARGET_CLEAR_CONTEXT:
			{
				CTargetManager::ClearContext();
				break;
			}
			case eRPC::BUILD_OPEN:
			{
				CBuildManager::HandleOpen(bitStream);
				break;
			}
			case eRPC::BUILD_CLOSE:
			{
				CBuildManager::HandleClose();
				break;
			}
			case eRPC::BUILD_CLEAR_PARTS:
			{
				CBuildManager::HandleClearParts();
				break;
			}
			case eRPC::BUILD_ADD_PART:
			{
				CBuildManager::HandleAddPart(bitStream);
				break;
			}
			case eRPC::BUILD_RESULT:
			{
				CBuildManager::HandleResult(bitStream);
				break;
			}
			case eRPC::BUILD_REMOVE_TARGET:
			{
				CBuildManager::HandleRemoveTarget(bitStream);
				break;
			}
			case eRPC::UI_OPEN:
			{
				CRmlUiManager::HandleOpen(bitStream);
				break;
			}
			case eRPC::UI_CLOSE:
			{
				CRmlUiManager::HandleClose(bitStream);
				break;
			}
			case eRPC::UI_CLOSE_ALL:
			{
				CRmlUiManager::HandleCloseAll();
				break;
			}
			case eRPC::UI_SET_DATA:
			{
				CRmlUiManager::HandleSetData(bitStream);
				break;
			}
			case eRPC::UI_INVENTORY_CLEAR:
			{
				CRmlUiManager::HandleInventoryClear(bitStream);
				break;
			}
			case eRPC::UI_INVENTORY_SET_SLOT:
			{
				CRmlUiManager::HandleInventorySetSlot(bitStream);
				break;
			}
			default:
				CLog::Write("Safe RPC ignored: %u", usRpcId);
				break;
		}
	}

	void Initialize(std::string address, unsigned short port)
	{
		Shutdown();
		bInitialized = false;
		transportMode = TRANSPORT_SIDE_CHANNEL;

		strAddress = address;
		usPort = port;

		pRakClient = new CRakClient();
		bConnected = false;
		bServerHasPlugin = false;

		if (pRakClient->Startup() != RakNet::StartupResult::RAKNET_STARTED)
			return;

		bInitialized = true;
	}

	void InitializeNative()
	{
		Shutdown();
		bInitialized = false;
		bConnected = false;
		bServerHasPlugin = false;
		pRakClient = NULL;
		transportMode = TRANSPORT_NATIVE_RAKCLIENT;
		dwNextHelloAttempt = 0;
		dwNextNativeInitAttempt = 0;

		if (!CGameRakClient::Initialize())
		{
			dwNextNativeInitAttempt = GetTickCount() + 1000;
			CLog::Write("Native RakClient transport pending; waiting for samp.dll network state");
			return;
		}

		bInitialized = true;
	}

	bool IsInitialized()
	{
		return bInitialized;
	}

	void Connect()
	{
		if (transportMode == TRANSPORT_NATIVE_RAKCLIENT)
		{
			if (IsInitialized())
			{
				CGameRakClient::SendHello();
				dwNextHelloAttempt = GetTickCount() + 2000;
			}
			else
			{
				dwNextNativeInitAttempt = GetTickCount();
			}
			return;
		}

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
		if (transportMode == TRANSPORT_NATIVE_RAKCLIENT)
		{
			DWORD now = GetTickCount();
			if (!IsInitialized())
			{
				if (now >= dwNextNativeInitAttempt)
				{
					if (CGameRakClient::Initialize())
					{
						bInitialized = true;
						CGameRakClient::SendHello();
						dwNextHelloAttempt = now + 2000;
					}
					else
					{
						dwNextNativeInitAttempt = now + 1000;
					}
				}
				return;
			}

			if (!bConnected && now >= dwNextHelloAttempt)
			{
				CGameRakClient::SendHello();
				dwNextHelloAttempt = now + 2000;
			}
			return;
		}

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
					CLog::Write("Safe side-channel registered; limited HUD RPC mode enabled");

					break;
				}
				case ePacketType::PACKET_RPC:
				{
					unsigned short usRpcId;

					if (bitStream.Read<unsigned short>(usRpcId))
						ProcessSafeRPC(usRpcId, bitStream);

					break;
				}
				case ePacketType::PACKET_CONNECTION_REJECTED:
				case ePacketType::PACKET_PLAYER_PROPER_DISCONNECT:
				{
					bServerHasPlugin = false;
					CKeyBinds::Clear();
					CTargetManager::ClearContext();
					CBuildManager::Clear();
					CRmlUiManager::Clear();

					break;
				}
				case ID_DISCONNECTION_NOTIFICATION:
				case ID_CONNECTION_LOST:
				{
					bConnected = false;
					CKeyBinds::Clear();
					CTargetManager::ClearContext();
					CBuildManager::Clear();
					CRmlUiManager::Clear();

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
		if (transportMode == TRANSPORT_NATIVE_RAKCLIENT)
			return 0;

		if (!IsConnected())
			return 0;

		CLog::Write("Sent packet: %i, size: %i byte(s)", packetType, pBitStream ? pBitStream->GetNumberOfBytesUsed() : 0);
		CLog::bytesSent += pBitStream ? pBitStream->GetNumberOfBytesUsed() : 0;

		return pRakClient->Send(packetType, *pRakClient->GetRemoteAddress(), pBitStream, priority, reliability, cOrderingChannel);
	}

	unsigned int SendRPC(unsigned short usRPCId, RakNet::BitStream* pBitStream, PacketPriority priority, PacketReliability reliability, char cOrderingChannel)
	{
		if (transportMode == TRANSPORT_NATIVE_RAKCLIENT)
		{
			if (!IsConnected())
				return 0;

			CLog::Write("Sent native RPC: %i, size: %i byte(s)", usRPCId, pBitStream ? pBitStream->GetNumberOfBytesUsed() : 0);
			CLog::bytesSent += pBitStream ? pBitStream->GetNumberOfBytesUsed() : 0;
			return CGameRakClient::SendClientRPC(usRPCId, pBitStream) ? 1 : 0;
		}

		if (!IsConnected())
			return 0;

		CLog::Write("Sent packet: %i, size: %i byte(s)", usRPCId, pBitStream ? pBitStream->GetNumberOfBytesUsed() : 0);
		CLog::bytesSent += pBitStream ? pBitStream->GetNumberOfBytesUsed() : 0;

		return pRakClient->SendRPC(usRPCId, *pRakClient->GetRemoteAddress(), pBitStream, priority, reliability, cOrderingChannel);
	}

	void HandleNativeRPC(unsigned char* data, unsigned int numberOfBits)
	{
		if (!data || !numberOfBits)
			return;

		const unsigned int byteCount = BITS_TO_BYTES(numberOfBits);
		unsigned int headerOffset = 0;
		bool foundHeader = false;
		uint32_t magic;
		uint16_t version;
		unsigned char message;

		for (unsigned int offset = 0; offset < byteCount && offset <= 8; ++offset)
		{
			RakNet::BitStream probe(data + offset, byteCount - offset, false);
			if (probe.Read(magic) && magic == OMPPlusProtocol::Magic
				&& probe.Read(version) && version == OMPPlusProtocol::Version
				&& probe.Read(message))
			{
				headerOffset = offset;
				foundHeader = true;
				break;
			}
		}

		if (!foundHeader)
		{
			DWORD now = GetTickCount();
			if (now >= dwNextInvalidNativeLog)
			{
				char hex[64] = { 0 };
				char* cursor = hex;
				const unsigned int preview = byteCount < 12 ? byteCount : 12;
				for (unsigned int i = 0; i < preview; ++i)
					cursor += sprintf(cursor, "%02X ", data[i]);
				CLog::Write("Invalid native OMP+ RPC received: bits=%u bytes=%u head=%s", numberOfBits, byteCount, hex);
				dwNextInvalidNativeLog = now + 5000;
			}
			return;
		}

		RakNet::BitStream bitStream(data + headerOffset, byteCount - headerOffset, false);
		if (!bitStream.Read(magic) || magic != OMPPlusProtocol::Magic
			|| !bitStream.Read(version) || version != OMPPlusProtocol::Version
			|| !bitStream.Read(message))
		{
			CLog::Write("Invalid native OMP+ RPC received");
			return;
		}

		switch (static_cast<OMPPlusProtocol::Message>(message))
		{
			case OMPPlusProtocol::Message::HelloAck:
			{
				uint32_t capabilities;
				bConnected = true;
				bServerHasPlugin = true;
				if (bitStream.Read(capabilities))
					CLog::Write("Native OMP+ transport ready, server capabilities=%u", capabilities);
				CLog::Write("Native OMP+ safe RPC mode enabled");
				break;
			}
			case OMPPlusProtocol::Message::ServerRPC:
			{
				unsigned short usRpcId;
				if (!bitStream.Read(usRpcId))
					return;

				ProcessSafeRPC(usRpcId, bitStream);
				break;
			}
			case OMPPlusProtocol::Message::Error:
			{
				uint16_t code;
				if (bitStream.Read(code))
					CLog::Write("Native OMP+ server error: %u", code);
				break;
			}
			default:
				break;
		}
	}

	CRakClient* GetRakClient()
	{
		return pRakClient;
	}

	void Shutdown()
	{
		CKeyBinds::Clear();
		CTargetManager::ClearContext();
		CBuildManager::Clear();
		CRmlUiManager::Clear();

		if (transportMode == TRANSPORT_NATIVE_RAKCLIENT)
			CGameRakClient::Shutdown();
		else if (pRakClient)
		{
			pRakClient->Shutdown(100);
			delete pRakClient;
			pRakClient = NULL;
		}

		bInitialized = false;
		bConnected = false;
		bServerHasPlugin = false;
		bSafeHUDInitialized = false;
		transportMode = TRANSPORT_DISABLED;
		dwNextHelloAttempt = 0;
		dwNextNativeInitAttempt = 0;
	}
}
