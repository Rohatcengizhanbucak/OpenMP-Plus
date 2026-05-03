#pragma once

#include <map>
#include <istream>
#include <string>

class CServerConfig
{
public:
	CServerConfig(const char* szFileName);
	virtual ~CServerConfig();

	void Reparse();
	bool IsLoaded() const;
	const std::string& GetSetting(const std::string& strKey) const;
	std::string GetSettingOr(const std::string& strKey, const std::string& strFallback) const;
	void SetSetting(const std::string& strKey, const std::string& strValue);
	inline std::map<std::string, std::string> GetSettings() { return m_settings; };
	inline bool HasSetting(const std::string& strKey) const { return !!m_settings.count(strKey); };
	
private:
	void ParseLegacyConfig(std::istream& isConfigFile);
	void ParseJsonConfig(const std::string& strConfig);

	std::map<std::string, std::string> m_settings;
	const char* m_szFileName;
	bool m_bLoaded;

};
