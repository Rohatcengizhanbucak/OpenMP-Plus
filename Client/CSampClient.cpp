#include <SAMP+/client/CSampClient.h>

namespace
{
	const SampClient::Layout kLayouts[] =
	{
		{ SampClient::VERSION_037_R1, "0.3.7-R1", 0x31DF13, 0x21A0F8, 0x3C9, 0x21A0E8 },
		{ SampClient::VERSION_037_R31, "0.3.7-R3-1", 0xCC4D0, 0x26E8DC, 0x2C, 0x26E8CC },
		{ SampClient::VERSION_037_R4, "0.3.7-R4", 0xCBCB0, 0x26EA0C, 0x2C, 0x26E9FC },
		{ SampClient::VERSION_03DL_R1, "0.3DL-R1", 0xFDB60, 0x2ACA24, 0x2C, 0x2ACA14 }
	};

	bool IsReadableProtection(DWORD protect)
	{
		if (protect & (PAGE_NOACCESS | PAGE_GUARD))
			return false;

		switch (protect & 0xFF)
		{
		case PAGE_READONLY:
		case PAGE_READWRITE:
		case PAGE_WRITECOPY:
		case PAGE_EXECUTE_READ:
		case PAGE_EXECUTE_READWRITE:
		case PAGE_EXECUTE_WRITECOPY:
			return true;
		default:
			return false;
		}
	}

	bool IsExecutableProtection(DWORD protect)
	{
		if (protect & (PAGE_NOACCESS | PAGE_GUARD))
			return false;

		switch (protect & 0xFF)
		{
		case PAGE_EXECUTE:
		case PAGE_EXECUTE_READ:
		case PAGE_EXECUTE_READWRITE:
		case PAGE_EXECUTE_WRITECOPY:
			return true;
		default:
			return false;
		}
	}
}

namespace SampClient
{
	DWORD GetBase()
	{
		return reinterpret_cast<DWORD>(GetModuleHandleA("samp.dll"));
	}

	bool CanRead(DWORD address, size_t size)
	{
		if (!address || !size)
			return false;

		DWORD cursor = address;
		size_t remaining = size;
		while (remaining > 0)
		{
			MEMORY_BASIC_INFORMATION mbi;
			if (!VirtualQuery(reinterpret_cast<LPCVOID>(cursor), &mbi, sizeof(mbi)))
				return false;

			if (mbi.State != MEM_COMMIT || !IsReadableProtection(mbi.Protect))
				return false;

			DWORD regionEnd = reinterpret_cast<DWORD>(mbi.BaseAddress) + static_cast<DWORD>(mbi.RegionSize);
			if (regionEnd <= cursor)
				return false;

			size_t readable = static_cast<size_t>(regionEnd - cursor);
			if (readable >= remaining)
				return true;

			cursor += static_cast<DWORD>(readable);
			remaining -= readable;
		}

		return true;
	}

	bool ReadPointer(DWORD address, DWORD& value)
	{
		if (!CanRead(address, sizeof(DWORD)))
			return false;

		value = *reinterpret_cast<DWORD*>(address);
		return value != 0;
	}

	bool IsExecutableAddress(DWORD address)
	{
		if (!address)
			return false;

		MEMORY_BASIC_INFORMATION mbi;
		if (!VirtualQuery(reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi)))
			return false;

		return mbi.State == MEM_COMMIT && IsExecutableProtection(mbi.Protect);
	}

	bool ValidateVTableObject(DWORD objectAddress, size_t methodCount)
	{
		DWORD vtable = 0;
		if (!ReadPointer(objectAddress, vtable))
			return false;

		if (!CanRead(vtable, methodCount * sizeof(DWORD)))
			return false;

		for (size_t i = 0; i < methodCount; ++i)
		{
			DWORD method = 0;
			if (!ReadPointer(vtable + static_cast<DWORD>(i * sizeof(DWORD)), method))
				return false;
			if (!IsExecutableAddress(method))
				return false;
		}

		return true;
	}

	DWORD GetEntryPoint(DWORD base)
	{
		if (!base || !CanRead(base, sizeof(IMAGE_DOS_HEADER)))
			return 0;

		IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
		if (dos->e_magic != IMAGE_DOS_SIGNATURE)
			return 0;

		DWORD ntAddress = base + dos->e_lfanew;
		if (!CanRead(ntAddress, sizeof(IMAGE_NT_HEADERS)))
			return 0;

		IMAGE_NT_HEADERS* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(ntAddress);
		if (nt->Signature != IMAGE_NT_SIGNATURE)
			return 0;

		return nt->OptionalHeader.AddressOfEntryPoint;
	}

	Version GetVersion(DWORD base)
	{
		DWORD entryPoint = GetEntryPoint(base);
		for (size_t i = 0; i < sizeof(kLayouts) / sizeof(kLayouts[0]); ++i)
		{
			if (kLayouts[i].entryPoint == entryPoint)
				return kLayouts[i].version;
		}

		return VERSION_UNKNOWN;
	}

	const char* GetVersionName(Version version)
	{
		Layout layout;
		return GetLayout(version, layout) ? layout.name : "unknown";
	}

	bool GetLayout(Version version, Layout& layout)
	{
		for (size_t i = 0; i < sizeof(kLayouts) / sizeof(kLayouts[0]); ++i)
		{
			if (kLayouts[i].version == version)
			{
				layout = kLayouts[i];
				return true;
			}
		}

		return false;
	}

	bool ResolveSampInfo(DWORD& sampInfo)
	{
		DWORD base = GetBase();
		Version version = GetVersion(base);
		Layout layout;
		if (!GetLayout(version, layout))
			return false;

		return ReadPointer(base + layout.sampInfoOffset, sampInfo);
	}

	bool IsChatInputActive()
	{
		DWORD base = GetBase();
		Version version = GetVersion(base);
		Layout layout;
		if (!GetLayout(version, layout))
			return false;

		DWORD inputInfo = 0;
		if (!ReadPointer(base + layout.chatInputInfoOffset, inputInfo))
			return false;

		DWORD editBox = 0;
		if (!ReadPointer(inputInfo + 0x8, editBox))
			return false;

		if (!CanRead(editBox + 0x4, sizeof(unsigned char)))
			return false;

		return *reinterpret_cast<unsigned char*>(editBox + 0x4) != 0;
	}
}
