#include <SAMP+/svr/Server.h>

#include <cstdlib>

namespace SAMPServer
{
	static CServerConfig* pServerConfig;

	void Initialize(const char* szConfigFileName)
	{
		pServerConfig = new CServerConfig(szConfigFileName);

		if (!pServerConfig->IsLoaded() && std::string(szConfigFileName) == "config.json")
		{
			delete pServerConfig;
			pServerConfig = new CServerConfig("server.cfg");
		}
	}

	CServerConfig* GetConfig()
	{
		return pServerConfig;
	}

	unsigned short getMaxPlayers()
	{
		std::string value = pServerConfig->GetSettingOr("max_players", pServerConfig->GetSettingOr("maxplayers", "50"));
		return (unsigned short)atoi(value.c_str());
	}

	std::string GetListeningAddress()
	{
		return pServerConfig->GetSettingOr("network.bind", pServerConfig->GetSettingOr("bind", ""));
	}

	unsigned short GetListeningPort()
	{
		std::string value = pServerConfig->GetSettingOr("network.port", pServerConfig->GetSettingOr("port", "7777"));
		return (unsigned short)atoi(value.c_str());
	}

}
