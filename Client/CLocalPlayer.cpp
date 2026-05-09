#include <SAMP+/client/CLocalPlayer.h>
#include <SAMP+/client/CRPCCallback.h>
#include <SAMP+/client/Network.h>
#include <SAMP+/client/CGame.h>

DWORD CLocalPlayer::m_actionMemory[ePlayerAction::COUNT - 1] =
{
	0xB7CE20,
	0xB73571,
	0xB73572,
	0xB73573,
	0xB73574,
	0xB73575,
	0xB73576
};

bool CLocalPlayer::m_actionStateInitialized = false;
bool CLocalPlayer::m_actionDesiredDisabled[ePlayerAction::COUNT - 1] = {};
bool CLocalPlayer::m_actionTemporaryDisabled[ePlayerAction::COUNT - 1] = {};

void CLocalPlayer::EnsureActionStateInitialized()
{
	if (m_actionStateInitialized)
		return;

	for (unsigned char i = 1; i < ePlayerAction::COUNT; ++i)
		m_actionDesiredDisabled[i - 1] = (*(BYTE*)m_actionMemory[i - 1]) != 0;

	m_actionStateInitialized = true;
}

void CLocalPlayer::WriteEffectiveActionState(unsigned char action)
{
	if (action == ePlayerAction::ALL || action >= ePlayerAction::COUNT)
		return;

	const unsigned char index = action - 1;
	const bool disabled = m_actionDesiredDisabled[index] || m_actionTemporaryDisabled[index];
	CMem::PutSingle<BYTE>(m_actionMemory[index], disabled ? 1 : 0);
}

void CLocalPlayer::SetActionEnabled(unsigned char action, bool bEnabled)
{
	if (action >= ePlayerAction::COUNT)
		return;

	EnsureActionStateInitialized();

	if (action == ePlayerAction::ALL)
	{
		for (unsigned char i = 1; i < ePlayerAction::COUNT; ++i)
			SetActionEnabled(i, bEnabled);

	}
	else
	{
		m_actionDesiredDisabled[action - 1] = !bEnabled;
		WriteEffectiveActionState(action);
	}

}

void CLocalPlayer::SetActionTemporarilyBlocked(unsigned char action, bool blocked)
{
	if (action >= ePlayerAction::COUNT)
		return;

	EnsureActionStateInitialized();

	if (action == ePlayerAction::ALL)
	{
		for (unsigned char i = 1; i < ePlayerAction::COUNT; ++i)
			SetActionTemporarilyBlocked(i, blocked);
	}
	else
	{
		const unsigned char index = action - 1;
		if (m_actionTemporaryDisabled[index] == blocked)
			return;

		m_actionTemporaryDisabled[index] = blocked;
		WriteEffectiveActionState(action);
	}
}

// TODO: make cancellable
void CLocalPlayer::OnDriveByShot()
{
	Network::SendRPC(eRPC::ON_DRIVE_BY_SHOT);
}

void CLocalPlayer::OnStuntBonus(sStuntDetails* pStuntDetails)
{
	RakNet::BitStream bitStream;
	bitStream.Write(*pStuntDetails);

	Network::SendRPC(eRPC::ON_STUNT_BONUS, &bitStream);
}

void CLocalPlayer::SetClipAmmo(BYTE bSlot, int bAmmo)
{
	if (bSlot < 50) {
		//DWORD address = ((*(int*)0xB6F5F0) + 0x588) + (0x28 * bSlot) + 0x8;
		//CMem::PutSingle<int>(address, bAmmo);
		CGame::ClipAmmo[bSlot] = bAmmo;
	}
}


void CLocalPlayer::SetNoReload(bool toggle) {
	if (toggle) 
	{
		CMem::PutSingle<BYTE>(0x07428AB, 0x90);
		CMem::PutSingle<BYTE>(0x07428AC, 0x90);
	}
	else
	{
		CMem::PutSingle<BYTE>(0x07428AB, 0x85); // test eax,eax
		CMem::PutSingle<BYTE>(0x07428AC, 0xC0);
	}
}

void CLocalPlayer::ToggleInfiniteRun(bool toggle)
{
	CMem::PutSingle<BYTE>(0xB7CEE4, toggle);
}
