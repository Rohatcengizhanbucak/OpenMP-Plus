#pragma once

#include <map>
#include <string>
#include <vector>

struct sKeyBind
{
	unsigned short key;
	unsigned char eventMask;
	bool wasDown;
	std::string action;
};

struct sKeyCaptureLease
{
	unsigned short key;
	unsigned char eventMask;
	unsigned char flags;
	short priority;
	unsigned long expiresAt;
	unsigned long sequence;
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

	enum eKeyCaptureFlags : unsigned char
	{
		KEY_CAPTURE_CONSUME_GAME_INPUT = 1,
		KEY_CAPTURE_BLOCK_SWITCH_WEAPON = 2
	};

	static void Bind(unsigned short key, unsigned char eventMask, const std::string& action);
	static void Unbind(unsigned short key);
	static void Clear();
	static void BeginCapture(unsigned short key, unsigned char eventMask, unsigned char flags, short priority, unsigned short ttlMs, const std::string& action);
	static void EndCapture(unsigned short key, const std::string& action);
	static void ClearCaptures();
	static void Process();
	static void FilterKeyboardState(unsigned long stateSize, void* state);
	static bool ShouldConsumeDirectInputOffset(unsigned long offset);
	static bool IsTextInputActive();

private:
	struct sPendingKeyState
	{
		unsigned short key;
		unsigned char state;
		std::string action;
	};

	static bool IsGameForeground();
	static bool IsKeyDown(unsigned short key);
	static void SyncKeyStates();
	static void SendKeyState(unsigned short key, unsigned char state, const std::string& action);
	static void CleanExpiredCaptures(unsigned long now);
	static bool CaptureExpired(const sKeyCaptureLease& capture, unsigned long now);
	static bool FindBestCapture(unsigned short key, unsigned long now, sKeyCaptureLease& capture);
	static std::string MakeCaptureId(unsigned short key, const std::string& action);
	static unsigned char VirtualKeyToDirectInputOffset(unsigned short key);
	static bool MatchesDirectInputOffset(unsigned short key, unsigned long offset);
	static std::vector<unsigned short> GetConsumedKeys();
	static bool HasActiveCaptureFlag(unsigned char flag);
	static void UpdateGameActionBlocks();

	static std::map<unsigned short, sKeyBind> m_binds;
	static std::map<std::string, sKeyCaptureLease> m_captures;
	static std::map<unsigned short, bool> m_keyStates;
	static unsigned long m_captureSequence;
};
