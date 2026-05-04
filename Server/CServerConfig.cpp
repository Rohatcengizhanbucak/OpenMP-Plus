#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

#include <SAMP+/Utility.h>
#include <SAMP+/svr/CServerConfig.h>

namespace
{
	class JsonReader
	{
	public:
		JsonReader(CServerConfig& config, const std::string& source)
			: m_config(config), m_source(source), m_pos(0)
		{
		}

		void Parse()
		{
			SkipWhitespace();
			ParseValue("");
		}

	private:
		void ParseValue(const std::string& path)
		{
			SkipWhitespace();

			if (Match('{'))
				return ParseObject(path);
			if (Match('['))
				return ParseArray();
			if (Peek() == '"')
			{
				m_config.SetSetting(path, ParseString());
				return;
			}

			m_config.SetSetting(path, ParseLiteral());
		}

		void ParseObject(const std::string& path)
		{
			SkipWhitespace();
			if (Match('}'))
				return;

			do
			{
				SkipWhitespace();
				std::string key = ParseString();
				SkipWhitespace();
				Expect(':');

				const std::string childPath = path.empty() ? key : path + "." + key;
				ParseValue(childPath);
				SkipWhitespace();
			}
			while (Match(','));

			Expect('}');
		}

		void ParseArray()
		{
			int index = 0;
			SkipWhitespace();
			if (Match(']'))
				return;

			do
			{
				std::ostringstream ignoredPath;
				ignoredPath << "__array." << index++;
				ParseValue(ignoredPath.str());
				SkipWhitespace();
			}
			while (Match(','));

			Expect(']');
		}

		std::string ParseString()
		{
			Expect('"');
			std::string value;

			while (m_pos < m_source.length())
			{
				char ch = m_source[m_pos++];

				if (ch == '"')
					return value;

				if (ch == '\\' && m_pos < m_source.length())
				{
					char escaped = m_source[m_pos++];
					switch (escaped)
					{
					case '"':
					case '\\':
					case '/':
						value.push_back(escaped);
						break;
					case 'b':
						value.push_back('\b');
						break;
					case 'f':
						value.push_back('\f');
						break;
					case 'n':
						value.push_back('\n');
						break;
					case 'r':
						value.push_back('\r');
						break;
					case 't':
						value.push_back('\t');
						break;
					default:
						value.push_back(escaped);
						break;
					}
					continue;
				}

				value.push_back(ch);
			}

			throw std::runtime_error("unterminated string");
		}

		std::string ParseLiteral()
		{
			size_t start = m_pos;

			while (m_pos < m_source.length())
			{
				char ch = m_source[m_pos];
				if (std::isspace((unsigned char)ch) || ch == ',' || ch == '}' || ch == ']')
					break;

				++m_pos;
			}

			if (start == m_pos)
				throw std::runtime_error("expected JSON value");

			return m_source.substr(start, m_pos - start);
		}

		void SkipWhitespace()
		{
			while (m_pos < m_source.length() && std::isspace((unsigned char)m_source[m_pos]))
				++m_pos;
		}

		char Peek() const
		{
			return m_pos < m_source.length() ? m_source[m_pos] : '\0';
		}

		bool Match(char expected)
		{
			if (Peek() != expected)
				return false;

			++m_pos;
			return true;
		}

		void Expect(char expected)
		{
			if (!Match(expected))
				throw std::runtime_error("unexpected JSON token");
		}

		CServerConfig& m_config;
		const std::string& m_source;
		size_t m_pos;
	};

	bool EndsWith(const std::string& value, const std::string& suffix)
	{
		return value.length() >= suffix.length()
			&& value.compare(value.length() - suffix.length(), suffix.length(), suffix) == 0;
	}
}

CServerConfig::CServerConfig(const char* szFileName)
{
	m_szFileName = szFileName;
	m_bLoaded = false;

	Reparse();
}

CServerConfig::~CServerConfig()
{

}

void CServerConfig::Reparse()
{
	m_settings.clear();
	m_bLoaded = false;

	std::string strPath = Utility::GetApplicationPath(m_szFileName);
	std::ifstream isConfigFile(strPath.c_str(), std::ios::in);

	if (!isConfigFile.is_open())
		return;

	m_bLoaded = true;

	if (EndsWith(strPath, ".json"))
	{
		std::ostringstream buffer;
		buffer << isConfigFile.rdbuf();
		ParseJsonConfig(buffer.str());
		return;
	}

	ParseLegacyConfig(isConfigFile);
}

void CServerConfig::ParseLegacyConfig(std::istream& isConfigFile)
{
	std::string strLine;

	while (std::getline(isConfigFile, strLine))
	{
		std::string::size_type iDelimiter = strLine.find_first_of(' ');
		SetSetting(
			iDelimiter != std::string::npos ? strLine.substr(0, iDelimiter) : strLine,
			iDelimiter != std::string::npos ? strLine.substr(iDelimiter + 1) : "");
	}
}

void CServerConfig::ParseJsonConfig(const std::string& strConfig)
{
	try
	{
		JsonReader reader(*this, strConfig);
		reader.Parse();
	}
	catch (const std::exception& e)
	{
		Utility::Printf("Unable to parse %s: %s", m_szFileName, e.what());
		m_settings.clear();
		m_bLoaded = false;
	}
}

void CServerConfig::SetSetting(const std::string& strKey, const std::string& strValue)
{
	if (!strKey.empty() && strKey.find("__array.") != 0)
		m_settings[strKey] = strValue;
}

bool CServerConfig::IsLoaded() const
{
	return m_bLoaded;
}

const std::string& CServerConfig::GetSetting(const std::string& strKey) const
{
	return m_settings.find(strKey)->second;
}

std::string CServerConfig::GetSettingOr(const std::string& strKey, const std::string& strFallback) const
{
	std::map<std::string, std::string>::const_iterator it = m_settings.find(strKey);
	return it != m_settings.end() ? it->second : strFallback;
}
