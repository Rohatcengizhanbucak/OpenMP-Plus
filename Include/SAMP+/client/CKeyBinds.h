#pragma once

#include <map>
#include <string>

struct sKeyBind
{
	unsigned short key;
	unsigned char eventMask;
	bool wasDown;
	std::string action;
};

class CKeyBinds
{
public:
	enum eKeyState : unsigned char
	{
		KEY_STATE_UP = 0,
		KEY_STATE_DOWN = 1
	};

	enum eKeyEventMask : unsigned char
	{
		KEY_EVENT_DOWN = 1,
		KEY_EVENT_UP = 2,
		KEY_EVENT_BOTH = KEY_EVENT_DOWN | KEY_EVENT_UP
	};

	static void Bind(unsigned short key, unsigned char eventMask, const std::string& action);
	static void Unbind(unsigned short key);
	static void Clear();
	static void Process();

private:
	static bool IsGameForeground();
	static bool IsKeyDown(unsigned short key);
	static void SendKeyState(const sKeyBind& bind, unsigned char state);

	static std::map<unsigned short, sKeyBind> m_binds;
};
