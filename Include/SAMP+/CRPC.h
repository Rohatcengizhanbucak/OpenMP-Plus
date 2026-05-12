#pragma once

#include <map>
#include <RakNet/BitStream.h>
#include <SAMP+/OMPPlusProtocol.h>

#define RPC_CALLBACK void
#define RPC_ARGS RakNet::BitStream& bsData, int iExtra

typedef RPC_CALLBACK (*RPCFunct_t)(RPC_ARGS);

enum eRPC : unsigned short
{
#define OMPPLUS_LEGACY_RPC_ENUM(name) name = OMPPlusProtocol::name,
	OMPPLUS_LEGACY_RPC_LIST(OMPPLUS_LEGACY_RPC_ENUM)
#undef OMPPLUS_LEGACY_RPC_ENUM
};

class CRPC
{
public:
	static void Add(unsigned short usRPCId, RPCFunct_t func);
	static void Process(unsigned short usRPCId, RakNet::BitStream& bsData, int iExtra = NULL);

private:
	static std::map<unsigned short, RPCFunct_t> m_functions;

};
