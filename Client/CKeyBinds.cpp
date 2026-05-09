#include <SAMP+/client/CKeyBinds.h>

#include <algorithm>

#include <SAMP+/CRPC.h>
#include <SAMP+/client/Client.h>
#include <SAMP+/client/CLog.h>
#include <SAMP+/client/Network.h>

std::map<unsigned short, sKeyBind> CKeyBinds::m_binds;

namespace
{
	const size_t MAX_KEY_ACTION_LENGTH = 31;

	enum eSampVersion
	{
		SAMP_VERSION_UNKNOWN = -1,
		SAMP_VERSION_037_R1 = 0,
		SAMP_VERSION_037_R31,
		SAMP_VERSION_037_R4,
		SAMP_VERSION_03DL_R1
	};

	const DWORD SAMP_CHAT_INPUT_INFO_OFFSETS[] = { 0x21A0E8, 0x26E8CC, 0x26E9FC, 0x2ACA14 };
	const DWORD SAMP_INPUT_EDITBOX_OFFSET = 0x8;
	const DWORD SAMP_INPUT_BOX_OPEN_OFFSET = 0x4;

	DWORD GetSampBase()
	{
		return reinterpret_cast<DWORD>(GetModuleHandleA("samp.dll"));
	}

	bool CanRead(DWORD address, size_t size)
	{
		if (!address || !size)
			return false;

		MEMORY_BASIC_INFORMATION mbi;
		if (!VirtualQuery(reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi)))
			return false;

		if (mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
			return false;

		DWORD end = address + static_cast<DWORD>(size);
		DWORD regionEnd = reinterpret_cast<DWORD>(mbi.BaseAddress) + static_cast<DWORD>(mbi.RegionSize);
		return end >= address && end <= regionEnd;
	}

	bool ReadPointer(DWORD address, DWORD& value)
	{
		if (!CanRead(address, sizeof(DWORD)))
			return false;

		value = *reinterpret_cast<DWORD*>(address);
		return value != 0;
	}

	eSampVersion GetSampVersion(DWORD base)
	{
		if (!base || !CanRead(base, sizeof(IMAGE_DOS_HEADER)))
			return SAMP_VERSION_UNKNOWN;

		IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
		if (dos->e_magic != IMAGE_DOS_SIGNATURE)
			return SAMP_VERSION_UNKNOWN;

		DWORD ntAddress = base + dos->e_lfanew;
		if (!CanRead(ntAddress, sizeof(IMAGE_NT_HEADERS)))
			return SAMP_VERSION_UNKNOWN;

		IMAGE_NT_HEADERS* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(ntAddress);
		if (nt->Signature != IMAGE_NT_SIGNATURE)
			return SAMP_VERSION_UNKNOWN;

		switch (nt->OptionalHeader.AddressOfEntryPoint)
		{
		case 0x31DF13:
			return SAMP_VERSION_037_R1;
		case 0xCC4D0:
			return SAMP_VERSION_037_R31;
		case 0xCBCB0:
			return SAMP_VERSION_037_R4;
		case 0xFDB60:
			return SAMP_VERSION_03DL_R1;
		default:
			return SAMP_VERSION_UNKNOWN;
		}
	}
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
	DWORD base = GetSampBase();
	eSampVersion version = GetSampVersion(base);
	if (version == SAMP_VERSION_UNKNOWN)
		return false;

	DWORD inputInfo;
	if (!ReadPointer(base + SAMP_CHAT_INPUT_INFO_OFFSETS[version], inputInfo))
		return false;

	DWORD editBox;
	if (!ReadPointer(inputInfo + SAMP_INPUT_EDITBOX_OFFSET, editBox))
		return false;

	if (!CanRead(editBox + SAMP_INPUT_BOX_OPEN_OFFSET, sizeof(unsigned char)))
		return false;

	return *reinterpret_cast<unsigned char*>(editBox + SAMP_INPUT_BOX_OPEN_OFFSET) != 0;
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
