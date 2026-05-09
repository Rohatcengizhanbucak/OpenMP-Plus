#include <SAMP+/client/CGameRakClient.h>

#include <SAMP+/client/CLog.h>
#include <SAMP+/client/Network.h>
#include <SAMP+/client/CSampClient.h>

#include <stdio.h>

namespace
{
	const size_t RAKCLIENT_MIN_VTABLE_METHODS = 28;

	CGameRakClientInterface* g_pRakClient = NULL;
	int g_iRegisteredRpc = OMPPlusProtocol::RpcID;
	DWORD g_dwNextUnavailableLog = 0;

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
	if (g_pRakClient && SampClient::ValidateVTableObject(reinterpret_cast<DWORD>(g_pRakClient), RAKCLIENT_MIN_VTABLE_METHODS))
		g_pRakClient->UnregisterAsRemoteProcedureCall(&g_iRegisteredRpc);

	g_pRakClient = NULL;
}

CGameRakClientInterface* CGameRakClient::ResolveInterface()
{
	DWORD base = SampClient::GetBase();
	if (!base)
	{
		LogUnavailable("samp.dll module is not loaded; start through the SA-MP/open.mp launcher");
		return NULL;
	}

	SampClient::Version version = SampClient::GetVersion(base);
	SampClient::Layout layout;
	if (!SampClient::GetLayout(version, layout))
	{
		char reason[96] = { 0 };
		sprintf(reason, "unsupported samp.dll entry point 0x%08X", SampClient::GetEntryPoint(base));
		LogUnavailable(reason);
		return NULL;
	}

	DWORD sampInfo = 0;
	if (!SampClient::ReadPointer(base + layout.sampInfoOffset, sampInfo))
	{
		LogUnavailable("samp info is not ready");
		return NULL;
	}

	DWORD rakClient = 0;
	if (!SampClient::ReadPointer(sampInfo + layout.rakClientInterfaceOffset, rakClient))
	{
		LogUnavailable("RakClientInterface is not ready");
		return NULL;
	}

	if (!SampClient::ValidateVTableObject(rakClient, RAKCLIENT_MIN_VTABLE_METHODS))
	{
		LogUnavailable("RakClientInterface failed vtable validation");
		return NULL;
	}

	CLog::Write("Native RakClient transport resolved for SA-MP %s", layout.name);
	return reinterpret_cast<CGameRakClientInterface*>(rakClient);
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

	if (!SampClient::ValidateVTableObject(reinterpret_cast<DWORD>(g_pRakClient), RAKCLIENT_MIN_VTABLE_METHODS))
	{
		CLog::Write("Native RakClient RPC send blocked: invalid RakClientInterface");
		g_pRakClient = NULL;
		return false;
	}

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
