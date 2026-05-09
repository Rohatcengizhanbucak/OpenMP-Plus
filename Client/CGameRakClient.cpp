#include <SAMP+/client/CGameRakClient.h>

#include <SAMP+/client/CLog.h>
#include <SAMP+/client/Network.h>
#include <SAMP+/client/CSampClient.h>

#include <Wincrypt.h>

#include <stdio.h>
#include <string>

#ifndef PROV_RSA_AES
#define PROV_RSA_AES 24
#endif

#ifndef ALG_SID_SHA_256
#define ALG_SID_SHA_256 12
#endif

#ifndef CALG_SHA_256
#define CALG_SHA_256 (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_SHA_256)
#endif

namespace
{
	const size_t RAKCLIENT_MIN_VTABLE_METHODS = 28;

	CGameRakClientInterface* g_pRakClient = NULL;
	HMODULE g_hModule = NULL;
	int g_iRegisteredRpc = OMPPlusProtocol::RpcID;
	DWORD g_dwNextUnavailableLog = 0;
	std::string g_strClientHash;
	bool g_bClientHashResolved = false;

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

	bool BytesToHex(const unsigned char* bytes, DWORD length, std::string& output)
	{
		static const char* hex = "0123456789ABCDEF";
		if (!bytes || !length)
			return false;

		output.clear();
		output.reserve(length * 2);
		for (DWORD i = 0; i < length; ++i)
		{
			output.push_back(hex[(bytes[i] >> 4) & 0x0F]);
			output.push_back(hex[bytes[i] & 0x0F]);
		}
		return true;
	}

	std::string ComputeModuleSHA256()
	{
		if (g_bClientHashResolved)
			return g_strClientHash;

		g_bClientHashResolved = true;
		g_strClientHash.clear();

		char path[MAX_PATH] = { 0 };
		if (!g_hModule || !GetModuleFileNameA(g_hModule, path, sizeof(path)))
			return g_strClientHash;

		HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file == INVALID_HANDLE_VALUE)
			return g_strClientHash;

		HCRYPTPROV provider = 0;
		HCRYPTHASH hash = 0;
		if (!CryptAcquireContextA(&provider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
		{
			CloseHandle(file);
			return g_strClientHash;
		}

		if (!CryptCreateHash(provider, CALG_SHA_256, 0, 0, &hash))
		{
			CryptReleaseContext(provider, 0);
			CloseHandle(file);
			return g_strClientHash;
		}

		unsigned char buffer[4096];
		DWORD bytesRead = 0;
		while (ReadFile(file, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
		{
			if (!CryptHashData(hash, buffer, bytesRead, 0))
				break;
		}

		unsigned char hashBytes[32] = { 0 };
		DWORD hashLength = sizeof(hashBytes);
		if (CryptGetHashParam(hash, HP_HASHVAL, hashBytes, &hashLength, 0))
			BytesToHex(hashBytes, hashLength, g_strClientHash);

		CryptDestroyHash(hash);
		CryptReleaseContext(provider, 0);
		CloseHandle(file);

		return g_strClientHash;
	}
}

void CGameRakClient::SetModuleHandle(HMODULE module)
{
	g_hModule = module;
	g_bClientHashResolved = false;
	g_strClientHash.clear();
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
	stream.Write(OMPPlusProtocol::ClientInfoVersion);
	stream.Write(OMPPlusProtocol::ClientVersionMajor);
	stream.Write(OMPPlusProtocol::ClientVersionMinor);
	stream.Write(OMPPlusProtocol::ClientVersionPatch);
	stream.Write(static_cast<uint32_t>(OMPPlusProtocol::DefaultFeatures));
	stream.Write(false);

	std::string hash = ComputeModuleSHA256();
	if (hash.length() > OMPPlusProtocol::MaxClientHashLength)
		hash.resize(OMPPlusProtocol::MaxClientHashLength);

	stream.Write(static_cast<uint8_t>(hash.length()));
	if (!hash.empty())
		stream.Write(hash.c_str(), static_cast<int>(hash.length()));

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
