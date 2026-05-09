#include <SAMP+/client/CGameRakClient.h>

#include <SAMP+/client/CLog.h>
#include <SAMP+/client/Network.h>

#include <stdio.h>

namespace
{
	enum eSampVersion
	{
		SAMP_VERSION_UNKNOWN = -1,
		SAMP_VERSION_037_R1 = 0,
		SAMP_VERSION_037_R31,
		SAMP_VERSION_037_R4,
		SAMP_VERSION_03DL_R1
	};

	const DWORD SAMP_INFO_OFFSETS[] = { 0x21A0F8, 0x26E8DC, 0x26EA0C, 0x2ACA24 };
	const DWORD RAKCLIENT_INTERFACE_OFFSETS[] = { 0x3C9, 0x2C, 0x2C, 0x2C };

	CGameRakClientInterface* g_pRakClient = NULL;
	int g_iRegisteredRpc = OMPPlusProtocol::RpcID;
	DWORD g_dwNextUnavailableLog = 0;

	DWORD GetSampBase()
	{
		HMODULE hSamp = GetModuleHandleA("samp.dll");
		return reinterpret_cast<DWORD>(hSamp);
	}

	eSampVersion GetSampVersion(DWORD base)
	{
		if (!base)
			return SAMP_VERSION_UNKNOWN;

		IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
		if (dos->e_magic != IMAGE_DOS_SIGNATURE)
			return SAMP_VERSION_UNKNOWN;

		IMAGE_NT_HEADERS* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
		if (nt->Signature != IMAGE_NT_SIGNATURE)
			return SAMP_VERSION_UNKNOWN;

		switch (nt->OptionalHeader.AddressOfEntryPoint)
		{
		case 0x31DF13:
			return SAMP_VERSION_037_R1;
		case 0xCC4D0:
			return SAMP_VERSION_037_R31;
		case 0xCBCB0:
			return SAMP_VERSION_037_R4;
		case 0xFDB60:
			return SAMP_VERSION_03DL_R1;
		default:
			return SAMP_VERSION_UNKNOWN;
		}
	}

	DWORD GetSampEntryPoint(DWORD base)
	{
		if (!base)
			return 0;

		IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
		if (dos->e_magic != IMAGE_DOS_SIGNATURE)
			return 0;

		IMAGE_NT_HEADERS* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
		if (nt->Signature != IMAGE_NT_SIGNATURE)
			return 0;

		return nt->OptionalHeader.AddressOfEntryPoint;
	}

	const char* GetVersionName(eSampVersion version)
	{
		switch (version)
		{
		case SAMP_VERSION_037_R1:
			return "0.3.7-R1";
		case SAMP_VERSION_037_R31:
			return "0.3.7-R3-1";
		case SAMP_VERSION_037_R4:
			return "0.3.7-R4";
		case SAMP_VERSION_03DL_R1:
			return "0.3DL-R1";
		default:
			return "unknown";
		}
	}

	void WriteHeader(RakNet::BitStream& stream, OMPPlusProtocol::Message message)
	{
		stream.Write(OMPPlusProtocol::Magic);
		stream.Write(OMPPlusProtocol::Version);
		stream.Write(static_cast<unsigned char>(message));
	}

	void LogUnavailable(const char* reason)
	{
		DWORD now = GetTickCount();
		if (now < g_dwNextUnavailableLog)
			return;

		CLog::Write("Native RakClient transport unavailable: %s", reason);
		g_dwNextUnavailableLog = now + 5000;
	}
}

bool CGameRakClient::Initialize()
{
	if (g_pRakClient)
		return true;

	g_pRakClient = ResolveInterface();
	if (!g_pRakClient)
		return false;

	g_pRakClient->RegisterAsRemoteProcedureCall(&g_iRegisteredRpc, CGameRakClient::OnOMPPlusRPC);
	CLog::Write("Native RakClient transport registered RPC %d", OMPPlusProtocol::RpcID);
	return true;
}

bool CGameRakClient::IsInitialized()
{
	return g_pRakClient != NULL;
}

bool CGameRakClient::SendHello()
{
	RakNet::BitStream stream;
	stream.Write(OMPPlusProtocol::DefaultCapabilities);

	return SendMessage(OMPPlusProtocol::Message::Hello, 0, &stream);
}

bool CGameRakClient::SendClientRPC(unsigned short rpc, RakNet::BitStream* payload)
{
	return SendMessage(OMPPlusProtocol::Message::ClientRPC, rpc, payload);
}

void CGameRakClient::Shutdown()
{
	if (g_pRakClient)
		g_pRakClient->UnregisterAsRemoteProcedureCall(&g_iRegisteredRpc);

	g_pRakClient = NULL;
}

CGameRakClientInterface* CGameRakClient::ResolveInterface()
{
	DWORD base = GetSampBase();
	if (!base)
	{
		LogUnavailable("samp.dll module is not loaded; start through the SA-MP/open.mp launcher");
		return NULL;
	}

	eSampVersion version = GetSampVersion(base);
	if (version == SAMP_VERSION_UNKNOWN)
	{
		char reason[96] = { 0 };
		sprintf(reason, "unsupported samp.dll entry point 0x%08X", GetSampEntryPoint(base));
		LogUnavailable(reason);
		return NULL;
	}

	DWORD sampInfoPtrAddress = base + SAMP_INFO_OFFSETS[version];
	DWORD sampInfo = *reinterpret_cast<DWORD*>(sampInfoPtrAddress);
	if (!sampInfo)
	{
		LogUnavailable("samp info is not ready");
		return NULL;
	}

	CGameRakClientInterface** ppRakClient = reinterpret_cast<CGameRakClientInterface**>(sampInfo + RAKCLIENT_INTERFACE_OFFSETS[version]);
	if (!ppRakClient || !*ppRakClient)
	{
		LogUnavailable("RakClientInterface is not ready");
		return NULL;
	}

	CLog::Write("Native RakClient transport resolved for SA-MP %s", GetVersionName(version));
	return *ppRakClient;
}

void __cdecl CGameRakClient::OnOMPPlusRPC(OMPPlusRPCParameters* params)
{
	if (!params || !params->input || !params->numberOfBitsOfData)
		return;

	Network::HandleNativeRPC(params->input, params->numberOfBitsOfData);
}

bool CGameRakClient::SendMessage(OMPPlusProtocol::Message message, unsigned short rpc, RakNet::BitStream* payload)
{
	if (!g_pRakClient)
		return false;

	RakNet::BitStream stream;
	WriteHeader(stream, message);
	if (message == OMPPlusProtocol::Message::ClientRPC)
		stream.Write(rpc);
	if (payload && payload->GetNumberOfBytesUsed())
		stream.Write(reinterpret_cast<const char*>(payload->GetData()), payload->GetNumberOfBytesUsed());

	int rpcId = OMPPlusProtocol::RpcID;
	bool sent = g_pRakClient->RPC(&rpcId, &stream, HIGH_PRIORITY, RELIABLE_ORDERED, 0, false);
	if (!sent)
		CLog::Write("Native RakClient RPC send failed: message=%u rpc=%u", static_cast<unsigned int>(message), static_cast<unsigned int>(rpc));
	return sent;
}
