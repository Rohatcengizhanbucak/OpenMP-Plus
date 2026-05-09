#include <SAMP+/client/CKeyBinds.h>

#include <algorithm>
#include <cstdio>
#include <set>
#include <mutex>

#include <SAMP+/CRPC.h>
#include <SAMP+/client/Client.h>
#include <SAMP+/client/CLocalPlayer.h>
#include <SAMP+/client/CLog.h>
#include <SAMP+/client/Network.h>
#include <SAMP+/client/CSampClient.h>

std::map<unsigned short, sKeyBind> CKeyBinds::m_binds;
std::map<std::string, sKeyCaptureLease> CKeyBinds::m_captures;
std::map<unsigned short, bool> CKeyBinds::m_keyStates;
unsigned long CKeyBinds::m_captureSequence = 0;

namespace
{
	const size_t MAX_KEY_ACTION_LENGTH = 31;
	const unsigned short DEFAULT_CAPTURE_TTL_MS = 350;
	const unsigned short MAX_CAPTURE_TTL_MS = 5000;
	std::mutex g_keyBindMutex;
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

	std::lock_guard<std::mutex> guard(g_keyBindMutex);
	m_binds[key] = bind;
	m_keyStates[key] = bind.wasDown;
	CLog::Write("Bound key: key=%u mask=%u action=%s", key, eventMask, bind.action.c_str());
}

void CKeyBinds::Unbind(unsigned short key)
{
	std::lock_guard<std::mutex> guard(g_keyBindMutex);
	m_binds.erase(key);
	CLog::Write("Unbound key: key=%u", key);
}

void CKeyBinds::Clear()
{
	{
		std::lock_guard<std::mutex> guard(g_keyBindMutex);
		m_binds.clear();
		m_captures.clear();
		m_keyStates.clear();
		CLog::Write("Cleared key binds");
	}
	UpdateGameActionBlocks();
}

void CKeyBinds::BeginCapture(unsigned short key, unsigned char eventMask, unsigned char flags, short priority, unsigned short ttlMs, const std::string& action)
{
	if (key == 0 || key > 255)
		return;

	eventMask &= KEY_EVENT_BOTH;
	if (!eventMask)
		return;

	if (ttlMs == 0)
		ttlMs = DEFAULT_CAPTURE_TTL_MS;
	if (ttlMs > MAX_CAPTURE_TTL_MS)
		ttlMs = MAX_CAPTURE_TTL_MS;

	sKeyCaptureLease capture;
	capture.key = key;
	capture.eventMask = eventMask;
	capture.flags = flags;
	capture.priority = priority;
	capture.expiresAt = GetTickCount() + ttlMs;
	capture.action = action.substr(0, MAX_KEY_ACTION_LENGTH);

	{
		std::lock_guard<std::mutex> guard(g_keyBindMutex);
		capture.sequence = ++m_captureSequence;
		m_captures[MakeCaptureId(key, capture.action)] = capture;

		if (m_keyStates.find(key) == m_keyStates.end())
			m_keyStates[key] = IsKeyDown(key);
	}

	UpdateGameActionBlocks();
}

void CKeyBinds::EndCapture(unsigned short key, const std::string& action)
{
	{
		std::lock_guard<std::mutex> guard(g_keyBindMutex);
		m_captures.erase(MakeCaptureId(key, action.substr(0, MAX_KEY_ACTION_LENGTH)));
	}
	UpdateGameActionBlocks();
}

void CKeyBinds::ClearCaptures()
{
	{
		std::lock_guard<std::mutex> guard(g_keyBindMutex);
		m_captures.clear();
		CLog::Write("Cleared key capture leases");
	}
	UpdateGameActionBlocks();
}

void CKeyBinds::Process()
{
	if (!Network::IsConnected())
	{
		UpdateGameActionBlocks();
		return;
	}

	if (!IsGameForeground() || IsTextInputActive())
	{
		SyncKeyStates();
		UpdateGameActionBlocks();
		return;
	}

	{
		std::vector<sPendingKeyState> pending;
		unsigned long now = GetTickCount();

		{
			std::lock_guard<std::mutex> guard(g_keyBindMutex);
			CleanExpiredCaptures(now);

			if (m_binds.empty() && m_captures.empty())
				return;

			std::set<unsigned short> keys;
			for (std::map<unsigned short, sKeyBind>::const_iterator it = m_binds.begin(); it != m_binds.end(); ++it)
				keys.insert(it->first);
			for (std::map<std::string, sKeyCaptureLease>::const_iterator it = m_captures.begin(); it != m_captures.end(); ++it)
				keys.insert(it->second.key);

			for (std::set<unsigned short>::const_iterator it = keys.begin(); it != keys.end(); ++it)
			{
				const unsigned short key = *it;
				bool isDown = IsKeyDown(key);
				bool wasDown = false;
				std::map<unsigned short, bool>::iterator stateIt = m_keyStates.find(key);
				if (stateIt == m_keyStates.end())
					stateIt = m_keyStates.insert(std::make_pair(key, isDown)).first;
				wasDown = stateIt->second;

				if (isDown == wasDown)
					continue;

				stateIt->second = isDown;
				unsigned char state = isDown ? KEY_STATE_DOWN : KEY_STATE_UP;
				unsigned char eventMask = 0;
				std::string action;

				sKeyCaptureLease capture;
				if (FindBestCapture(key, now, capture))
				{
					eventMask = capture.eventMask;
					action = capture.action;
				}
				else
				{
					std::map<unsigned short, sKeyBind>::const_iterator bindIt = m_binds.find(key);
					if (bindIt == m_binds.end())
						continue;
					eventMask = bindIt->second.eventMask;
					action = bindIt->second.action;
				}

				if ((isDown && (eventMask & KEY_EVENT_DOWN)) || (!isDown && (eventMask & KEY_EVENT_UP)))
				{
					sPendingKeyState pendingState;
					pendingState.key = key;
					pendingState.state = state;
					pendingState.action = action;
					pending.push_back(pendingState);
				}
			}
		}

		for (std::vector<sPendingKeyState>::const_iterator it = pending.begin(); it != pending.end(); ++it)
			SendKeyState(it->key, it->state, it->action);

		UpdateGameActionBlocks();
	}
}

void CKeyBinds::FilterKeyboardState(unsigned long stateSize, void* state)
{
	if (!state || stateSize == 0)
		return;

	std::vector<unsigned short> keys = GetConsumedKeys();
	if (keys.empty())
		return;

	unsigned char* bytes = static_cast<unsigned char*>(state);
	for (std::vector<unsigned short>::const_iterator it = keys.begin(); it != keys.end(); ++it)
	{
		const unsigned short key = *it;
		const unsigned char directInputOffset = VirtualKeyToDirectInputOffset(key);

		if (key < stateSize)
			bytes[key] = 0;
		if (directInputOffset < stateSize)
			bytes[directInputOffset] = 0;
	}
}

bool CKeyBinds::ShouldConsumeDirectInputOffset(unsigned long offset)
{
	std::vector<unsigned short> keys = GetConsumedKeys();
	for (std::vector<unsigned short>::const_iterator it = keys.begin(); it != keys.end(); ++it)
	{
		if (MatchesDirectInputOffset(*it, offset))
			return true;
	}
	return false;
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
	std::lock_guard<std::mutex> guard(g_keyBindMutex);

	for (std::map<unsigned short, sKeyBind>::iterator it = m_binds.begin(); it != m_binds.end(); ++it)
	{
		it->second.wasDown = IsKeyDown(it->second.key);
		m_keyStates[it->second.key] = it->second.wasDown;
	}

	for (std::map<std::string, sKeyCaptureLease>::const_iterator it = m_captures.begin(); it != m_captures.end(); ++it)
		m_keyStates[it->second.key] = IsKeyDown(it->second.key);
}

void CKeyBinds::SendKeyState(unsigned short key, unsigned char state, const std::string& action)
{
	RakNet::BitStream bitStream;
	size_t boundedActionLength = action.length();
	if (boundedActionLength > MAX_KEY_ACTION_LENGTH)
		boundedActionLength = MAX_KEY_ACTION_LENGTH;
	unsigned char actionLength = static_cast<unsigned char>(boundedActionLength);

	bitStream.Write(key);
	bitStream.Write(state);
	bitStream.Write(actionLength);

	if (actionLength)
		bitStream.Write(action.c_str(), actionLength);

	CLog::Write("Key state changed: key=%u state=%u action=%s", key, state, action.c_str());
	Network::SendRPC(eRPC::ON_KEY_STATE_CHANGE, &bitStream);
}

void CKeyBinds::CleanExpiredCaptures(unsigned long now)
{
	for (std::map<std::string, sKeyCaptureLease>::iterator it = m_captures.begin(); it != m_captures.end();)
	{
		if (CaptureExpired(it->second, now))
			it = m_captures.erase(it);
		else
			++it;
	}
}

bool CKeyBinds::CaptureExpired(const sKeyCaptureLease& capture, unsigned long now)
{
	return static_cast<long>(now - capture.expiresAt) >= 0;
}

bool CKeyBinds::FindBestCapture(unsigned short key, unsigned long now, sKeyCaptureLease& capture)
{
	bool found = false;
	for (std::map<std::string, sKeyCaptureLease>::const_iterator it = m_captures.begin(); it != m_captures.end(); ++it)
	{
		const sKeyCaptureLease& candidate = it->second;
		if (candidate.key != key || CaptureExpired(candidate, now))
			continue;

		if (!found || candidate.priority > capture.priority || (candidate.priority == capture.priority && candidate.sequence > capture.sequence))
		{
			capture = candidate;
			found = true;
		}
	}
	return found;
}

std::string CKeyBinds::MakeCaptureId(unsigned short key, const std::string& action)
{
	char buffer[16] = {};
	sprintf_s(buffer, "%u:", key);
	return std::string(buffer) + action;
}

unsigned char CKeyBinds::VirtualKeyToDirectInputOffset(unsigned short key)
{
	return static_cast<unsigned char>(MapVirtualKeyA(static_cast<unsigned int>(key), MAPVK_VK_TO_VSC) & 0xFF);
}

bool CKeyBinds::MatchesDirectInputOffset(unsigned short key, unsigned long offset)
{
	return offset == key || offset == VirtualKeyToDirectInputOffset(key);
}

std::vector<unsigned short> CKeyBinds::GetConsumedKeys()
{
	std::vector<unsigned short> keys;

	if (!IsGameForeground() || IsTextInputActive())
		return keys;

	unsigned long now = GetTickCount();
	std::lock_guard<std::mutex> guard(g_keyBindMutex);
	CleanExpiredCaptures(now);

	for (std::map<std::string, sKeyCaptureLease>::const_iterator it = m_captures.begin(); it != m_captures.end(); ++it)
	{
		const sKeyCaptureLease& capture = it->second;
		if ((capture.flags & KEY_CAPTURE_CONSUME_GAME_INPUT) == 0)
			continue;
		if (std::find(keys.begin(), keys.end(), capture.key) == keys.end())
			keys.push_back(capture.key);
	}

	return keys;
}

bool CKeyBinds::HasActiveCaptureFlag(unsigned char flag)
{
	if (!IsGameForeground() || IsTextInputActive())
		return false;

	unsigned long now = GetTickCount();
	std::lock_guard<std::mutex> guard(g_keyBindMutex);
	CleanExpiredCaptures(now);

	for (std::map<std::string, sKeyCaptureLease>::const_iterator it = m_captures.begin(); it != m_captures.end(); ++it)
	{
		if ((it->second.flags & flag) != 0)
			return true;
	}

	return false;
}

void CKeyBinds::UpdateGameActionBlocks()
{
	const bool blockSwitchWeapon = HasActiveCaptureFlag(KEY_CAPTURE_BLOCK_SWITCH_WEAPON);
	CLocalPlayer::SetActionTemporarilyBlocked(CLocalPlayer::SWITCH_WEAPON, blockSwitchWeapon);
}
