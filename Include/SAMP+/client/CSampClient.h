#pragma once

#include <windows.h>
#include <stddef.h>

namespace SampClient
{
	enum Version
	{
		VERSION_UNKNOWN = -1,
		VERSION_037_R1 = 0,
		VERSION_037_R31,
		VERSION_037_R4,
		VERSION_03DL_R1
	};

	struct Layout
	{
		Version version;
		const char* name;
		DWORD entryPoint;
		DWORD sampInfoOffset;
		DWORD rakClientInterfaceOffset;
		DWORD chatInputInfoOffset;
	};

	DWORD GetBase();
	DWORD GetEntryPoint(DWORD base);
	Version GetVersion(DWORD base);
	const char* GetVersionName(Version version);
	bool GetLayout(Version version, Layout& layout);
	bool CanRead(DWORD address, size_t size);
	bool ReadPointer(DWORD address, DWORD& value);
	bool IsExecutableAddress(DWORD address);
	bool ValidateVTableObject(DWORD objectAddress, size_t methodCount);
	bool ResolveSampInfo(DWORD& sampInfo);
	bool IsChatInputActive();
}
