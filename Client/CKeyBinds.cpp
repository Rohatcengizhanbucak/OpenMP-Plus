#include <SAMP+/client/CKeyBinds.h>

#include <algorithm>

#include <SAMP+/CRPC.h>
#include <SAMP+/client/Client.h>
#include <SAMP+/client/CLog.h>
#include <SAMP+/client/Network.h>
#include <SAMP+/client/CSampClient.h>

std::map<unsigned short, sKeyBind> CKeyBinds::m_binds;

namespace
{
	const size_t MAX_KEY_ACTION_LENGTH = 31;
}

void CKeyBinds::Bind(unsigned short key, unsigned char eventMask, const std::string& action)
{
	if (key == 0 || key > 255)
		return;

	eventMask &= KEY_EVENT_BOTH;
	if (!eventMask)
		return;

	sKeyBind bind;
	bind.key = key;
	bind.eventMask = eventMask;
	bind.wasDown = IsKeyDown(key);
	bind.action = action.substr(0, MAX_KEY_ACTION_LENGTH);

	m_binds[key] = bind;
	CLog::Write("Bound key: key=%u mask=%u action=%s", key, eventMask, bind.action.c_str());
}

void CKeyBinds::Unbind(unsigned short key)
{
	m_binds.erase(key);
	CLog::Write("Unbound key: key=%u", key);
}

void CKeyBinds::Clear()
{
	m_binds.clear();
	CLog::Write("Cleared key binds");
}

void CKeyBinds::Process()
{
	if (m_binds.empty() || !Network::IsConnected())
		return;

	if (!IsGameForeground() || IsTextInputActive())
	{
		SyncKeyStates();
		return;
	}

	for (std::map<unsigned short, sKeyBind>::iterator it = m_binds.begin(); it != m_binds.end(); ++it)
	{
		sKeyBind& bind = it->second;
		bool isDown = IsKeyDown(bind.key);

		if (isDown == bind.wasDown)
			continue;

		bind.wasDown = isDown;
		unsigned char state = isDown ? KEY_STATE_DOWN : KEY_STATE_UP;

		if ((isDown && (bind.eventMask & KEY_EVENT_DOWN)) || (!isDown && (bind.eventMask & KEY_EVENT_UP)))
			SendKeyState(bind, state);
	}
}

bool CKeyBinds::IsGameForeground()
{
	HWND hwnd = GetForegroundWindow();
	if (!hwnd)
		return false;

	DWORD processId = 0;
	GetWindowThreadProcessId(hwnd, &processId);
	return processId == GetCurrentProcessId();
}

bool CKeyBinds::IsTextInputActive()
{
	return SampClient::IsChatInputActive();
}

bool CKeyBinds::IsKeyDown(unsigned short key)
{
	return (GetAsyncKeyState((int)key) & 0x8000) != 0;
}

void CKeyBinds::SyncKeyStates()
{
	for (std::map<unsigned short, sKeyBind>::iterator it = m_binds.begin(); it != m_binds.end(); ++it)
		it->second.wasDown = IsKeyDown(it->second.key);
}

void CKeyBinds::SendKeyState(const sKeyBind& bind, unsigned char state)
{
	RakNet::BitStream bitStream;
	size_t boundedActionLength = bind.action.length();
	if (boundedActionLength > MAX_KEY_ACTION_LENGTH)
		boundedActionLength = MAX_KEY_ACTION_LENGTH;
	unsigned char actionLength = static_cast<unsigned char>(boundedActionLength);

	bitStream.Write(bind.key);
	bitStream.Write(state);
	bitStream.Write(actionLength);

	if (actionLength)
		bitStream.Write(bind.action.c_str(), actionLength);

	CLog::Write("Key state changed: key=%u state=%u action=%s", bind.key, state, bind.action.c_str());
	Network::SendRPC(eRPC::ON_KEY_STATE_CHANGE, &bitStream);
}
