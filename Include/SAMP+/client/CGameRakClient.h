#pragma once

#include <windows.h>

#include <RakNet/BitStream.h>
#include <RakNet/PacketPriority.h>
#include <SAMP+/OMPPlusProtocol.h>

struct OMPPlusPlayerID
{
	unsigned int binaryAddress;
	unsigned short port;
};

struct OMPPlusNetworkID
{
	OMPPlusPlayerID playerId;
	unsigned short localSystemId;
};

struct OMPPlusRPCParameters
{
	unsigned char* input;
	unsigned int numberOfBitsOfData;
	OMPPlusPlayerID sender;
};

typedef void (__cdecl *OMPPlusRPCHandler)(OMPPlusRPCParameters* rpcParms);

class CGameRakClientInterface
{
public:
	virtual ~CGameRakClientInterface() {}
	virtual bool Connect(const char* host, unsigned short serverPort, unsigned short clientPort, unsigned int depreciated, int threadSleepTimer) = 0;
	virtual void Disconnect(unsigned int blockDuration, unsigned char orderingChannel = 0) = 0;
	virtual void InitializeSecurity(const char* privKeyP, const char* privKeyQ) = 0;
	virtual void SetPassword(const char* password) = 0;
	virtual bool HasPassword(void) const = 0;
	virtual bool Send(const char* data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel) = 0;
	virtual bool Send(RakNet::BitStream* bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel) = 0;
	virtual void* Receive(void) = 0;
	virtual void DeallocatePacket(void* packet) = 0;
	virtual void PingServer(void) = 0;
	virtual void PingServer(const char* host, unsigned short serverPort, unsigned short clientPort, bool onlyReplyOnAcceptingConnections) = 0;
	virtual int GetAveragePing(void) = 0;
	virtual int GetLastPing(void) const = 0;
	virtual int GetLowestPing(void) const = 0;
	virtual int GetPlayerPing(const OMPPlusPlayerID playerId) = 0;
	virtual void StartOccasionalPing(void) = 0;
	virtual void StopOccasionalPing(void) = 0;
	virtual bool IsConnected(void) const = 0;
	virtual unsigned int GetSynchronizedRandomInteger(void) const = 0;
	virtual bool GenerateCompressionLayer(unsigned int inputFrequencyTable[256], bool inputLayer) = 0;
	virtual bool DeleteCompressionLayer(bool inputLayer) = 0;
	virtual void RegisterAsRemoteProcedureCall(int* uniqueID, OMPPlusRPCHandler functionPointer) = 0;
	virtual void RegisterClassMemberRPC(int* uniqueID, void* functionPointer) = 0;
	virtual void UnregisterAsRemoteProcedureCall(int* uniqueID) = 0;
	virtual bool RPC(int* uniqueID, const char* data, unsigned int bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp) = 0;
	virtual bool RPC(int* uniqueID, RakNet::BitStream* bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp) = 0;
	virtual bool RPC_(int* uniqueID, RakNet::BitStream* bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, bool shiftTimestamp, OMPPlusNetworkID networkID) = 0;
};

class CGameRakClient
{
public:
	static bool Initialize();
	static bool IsInitialized();
	static bool SendHello();
	static bool SendClientRPC(unsigned short rpc, RakNet::BitStream* payload = NULL);
	static void Shutdown();

private:
	static CGameRakClientInterface* ResolveInterface();
	static void __cdecl OnOMPPlusRPC(OMPPlusRPCParameters* params);
	static bool SendMessage(OMPPlusProtocol::Message message, unsigned short rpc, RakNet::BitStream* payload);
};
