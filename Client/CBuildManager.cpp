#include <SAMP+/CRPC.h>
#include <SAMP+/OMPPlusProtocol.h>
#include <SAMP+/client/CBuildManager.h>
#include <SAMP+/client/CGraphics.h>
#include <SAMP+/client/CLog.h>
#include <SAMP+/client/COverlayLayout.h>
#include <SAMP+/client/CTargetManager.h>
#include <SAMP+/client/Network.h>
#include <SAMP+/client/Proxy/CDInput8DeviceProxy.h>
#include <SAMP+/client/Proxy/CMessageProxy.h>

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdint>

bool CBuildManager::m_active = false;
bool CBuildManager::m_menuOpen = false;
bool CBuildManager::m_cursorOwned = false;
bool CBuildManager::m_virtualCursorInitialized = false;
bool CBuildManager::m_lastLeftDown = false;
bool CBuildManager::m_lastRightDown = false;
bool CBuildManager::m_lastMiddleDown = false;
bool CBuildManager::m_lastEscapeDown = false;
bool CBuildManager::m_lastQDown = false;
bool CBuildManager::m_lastEDown = false;
float CBuildManager::m_virtualCursorX = 0.0f;
float CBuildManager::m_virtualCursorY = 0.0f;
LONG CBuildManager::m_virtualWheel = 0;
bool CBuildManager::m_mouseButtons[5] = {};
unsigned int CBuildManager::m_sessionId = 0;
unsigned int CBuildManager::m_selectedPartId = 0;
float CBuildManager::m_maxDistance = 8.0f;
int CBuildManager::m_rotationStep = 0;
bool CBuildManager::m_flipped = false;
unsigned long CBuildManager::m_nextPreviewSendAt = 0;
unsigned long CBuildManager::m_statusUntil = 0;
bool CBuildManager::m_statusSuccess = true;
unsigned long CBuildManager::m_inputGuardUntil = 0;
sBuildRemoveTarget CBuildManager::m_removeTarget = {};
std::string CBuildManager::m_title;
std::string CBuildManager::m_status;
std::vector<sBuildPart> CBuildManager::m_parts;
float CBuildManager::m_menuX = 0.0f;
float CBuildManager::m_menuY = 0.0f;
float CBuildManager::m_menuW = 0.0f;
float CBuildManager::m_menuH = 0.0f;

namespace
{
	struct sBuildStateLock
	{
		CRITICAL_SECTION section;

		sBuildStateLock()
		{
			InitializeCriticalSection(&section);
		}

		~sBuildStateLock()
		{
			DeleteCriticalSection(&section);
		}
	};

	class cScopedBuildStateLock
	{
	public:
		cScopedBuildStateLock()
		{
			EnterCriticalSection(&BuildLock().section);
		}

		~cScopedBuildStateLock()
		{
			LeaveCriticalSection(&BuildLock().section);
		}

	private:
		static sBuildStateLock& BuildLock()
		{
			static sBuildStateLock lock;
			return lock;
		}
	};

	struct sKeyboardReleaseKey
	{
		DWORD offset;
		int virtualKey;
	};

	const sKeyboardReleaseKey KeyboardReleaseKeys[] =
	{
		{ DIK_D, 'D' },
		{ DIK_A, 'A' },
		{ DIK_W, 'W' },
		{ DIK_S, 'S' },
		{ DIK_RIGHT, VK_RIGHT },
		{ DIK_LEFT, VK_LEFT },
		{ DIK_UP, VK_UP },
		{ DIK_DOWN, VK_DOWN },
		{ DIK_NUMPAD6, VK_NUMPAD6 },
		{ DIK_NUMPAD4, VK_NUMPAD4 },
		{ DIK_NUMPAD8, VK_NUMPAD8 },
		{ DIK_NUMPAD2, VK_NUMPAD2 },
		{ DIK_LMENU, VK_LMENU },
		{ DIK_RMENU, VK_RMENU },
		{ DIK_LSHIFT, VK_LSHIFT },
		{ DIK_RSHIFT, VK_RSHIFT },
		{ DIK_LCONTROL, VK_LCONTROL },
		{ DIK_RCONTROL, VK_RCONTROL },
		{ DIK_SPACE, VK_SPACE },
		{ DIK_RETURN, VK_RETURN },
		{ DIK_LBRACKET, VK_OEM_4 },
		{ DIK_RBRACKET, VK_OEM_6 }
	};

	enum eBuildPartId
	{
		BuildPartFoundation = 1,
		BuildPartWall = 2,
		BuildPartDoorFrame = 3,
		BuildPartFloor = 4,
		BuildPartRoof = 5,
		BuildPartStairs = 6,
		BuildPartDoor = 7,
		BuildPartRemove = 100
	};

	const float RemoveBadgeMinWidth = 178.0f;
	const float RemoveBadgeMaxWidth = 302.0f;
	const float RemoveBadgeHeight = 46.0f;
	const float BuildUiScreenPadding = 12.0f;
	const int BuildFoundationLiftSteps = 10;
	const float BuildFoundationFootprintHalf = 1.5f;
	const unsigned long BuildPreviewSendIntervalMs = 75;
	const uintptr_t GtaCameraBase = 0x00B6F028;
	const uintptr_t GtaCameraActiveCamOffset = 0x59;
	const uintptr_t GtaCameraCamsOffset = 0x174;
	const uintptr_t GtaCamSize = 0x238;
	const uintptr_t GtaCamFrontOffset = 0x190;
	const uintptr_t GtaCamSourceOffset = 0x19C;
	const uintptr_t GtaProcessLineOfSightAddress = 0x56BA00;
	const uintptr_t GtaFindGroundZForCoordAddress = 0x569660;
	const float BuildAimGroundNormalMinZ = 0.45f;
	const float BuildAimHighSurfaceThreshold = 1.0f;
	const float BuildFoundationFootprintProbeInset = 0.15f;
	const float BuildFoundationAimLockRadius = 1.75f;
	const float BuildFoundationAimLockZTolerance = 0.35f;

	struct sGtaVector
	{
		float x;
		float y;
		float z;
	};

	struct sGtaColPoint
	{
		sGtaVector point;
		float depth;
		sGtaVector normal;
		unsigned char data[0x10];
	};

	enum eBuildAimSurfaceState : unsigned char
	{
		BuildAimSurfaceNone = 0,
		BuildAimSurfaceGround = 1,
		BuildAimSurfaceBlockedNonGround = 2
	};

	struct sBuildAimHit
	{
		bool hasHit;
		float x;
		float y;
		float z;
		eBuildAimSurfaceState surfaceState;
		float footprintMinGroundZ;
		float footprintMaxGroundZ;
	};

	struct sBuildFoundationAimLock
	{
		bool valid;
		float x;
		float y;
		float z;
		float footprintMinGroundZ;
		float footprintMaxGroundZ;
	};

	sBuildFoundationAimLock gFoundationAimLock = {};

	bool IsPhysicalKeyDown(int virtualKey)
	{
		return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
	}

	float ClampFloat(float value, float minValue, float maxValue)
	{
		return (std::max)(minValue, (std::min)(value, maxValue));
	}

	void ResetBuildFoundationAimLock()
	{
		gFoundationAimLock = {};
	}

	bool IsFiniteVector(const sGtaVector& vector)
	{
		return std::isfinite(vector.x) && std::isfinite(vector.y) && std::isfinite(vector.z);
	}

	bool TryReadMemory(uintptr_t address, void* output, size_t size)
	{
		if (!address || !output || !size)
			return false;

		MEMORY_BASIC_INFORMATION mbi = {};
		void* memory = reinterpret_cast<void*>(address);
		if (!VirtualQuery(memory, &mbi, sizeof(mbi)))
			return false;
		if (mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
			return false;

		const uintptr_t regionEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
		if (address + size > regionEnd)
			return false;

		__try
		{
			std::memcpy(output, memory, size);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
		return true;
	}

	bool TryReadGtaVector(uintptr_t address, sGtaVector& vector)
	{
		if (!TryReadMemory(address, &vector, sizeof(vector)))
			return false;
		return IsFiniteVector(vector);
	}

	bool NormalizeVector(sGtaVector& vector)
	{
		const float length = std::sqrt((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));
		if (length <= 0.0001f || !std::isfinite(length))
			return false;

		vector.x /= length;
		vector.y /= length;
		vector.z /= length;
		return true;
	}

	bool TryFindTerrainGroundZ(float x, float y, float& z)
	{
		typedef float(__cdecl* FindGroundZForCoord_t)(float, float);

		float result = 0.0f;
		__try
		{
			FindGroundZForCoord_t findGroundZ = reinterpret_cast<FindGroundZForCoord_t>(GtaFindGroundZForCoordAddress);
			result = findGroundZ(x, y);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}

		if (!std::isfinite(result) || result < -200.0f || result > 2000.0f)
			return false;

		z = result;
		return true;
	}

	bool TryComputeFoundationFootprintGroundRange(const sGtaVector& center, float& minGroundZ, float& maxGroundZ)
	{
		const float half = (std::max)(0.25f, BuildFoundationFootprintHalf - BuildFoundationFootprintProbeInset);
		const float sampleOffsets[][2] =
		{
			{ 0.0f, 0.0f },
			{ -half, -half },
			{ -half, half },
			{ half, -half },
			{ half, half },
			{ -half, 0.0f },
			{ half, 0.0f },
			{ 0.0f, -half },
			{ 0.0f, half }
		};

		bool found = false;
		for (const auto& offset : sampleOffsets)
		{
			float sampleZ = 0.0f;
			if (!TryFindTerrainGroundZ(center.x + offset[0], center.y + offset[1], sampleZ))
				continue;

			if (!found)
			{
				minGroundZ = sampleZ;
				maxGroundZ = sampleZ;
				found = true;
				continue;
			}

			minGroundZ = (std::min)(minGroundZ, sampleZ);
			maxGroundZ = (std::max)(maxGroundZ, sampleZ);
		}

		return found;
	}

	bool IsLikelyBuildFoundationSelfHit(const sBuildAimHit& hit)
	{
		if (!hit.hasHit || !gFoundationAimLock.valid)
			return false;

		const float dx = hit.x - gFoundationAimLock.x;
		const float dy = hit.y - gFoundationAimLock.y;
		const float radiusSq = BuildFoundationAimLockRadius * BuildFoundationAimLockRadius;
		if (((dx * dx) + (dy * dy)) > radiusSq)
			return false;

		return hit.z > gFoundationAimLock.z + BuildFoundationAimLockZTolerance
			|| hit.footprintMinGroundZ > gFoundationAimLock.footprintMaxGroundZ + BuildFoundationAimLockZTolerance
			|| hit.footprintMaxGroundZ > gFoundationAimLock.footprintMaxGroundZ + BuildFoundationAimLockZTolerance;
	}

	bool TryProjectFoundationAimToTerrain(const sBuildAimHit& source, sBuildAimHit& hit)
	{
		if (!source.hasHit)
			return false;

		float groundZ = 0.0f;
		if (!TryFindTerrainGroundZ(source.x, source.y, groundZ))
			return false;

		hit.hasHit = true;
		hit.x = source.x;
		hit.y = source.y;
		hit.z = groundZ;
		hit.surfaceState = BuildAimSurfaceGround;
		hit.footprintMinGroundZ = groundZ;
		hit.footprintMaxGroundZ = groundZ;

		float minGroundZ = groundZ;
		float maxGroundZ = groundZ;
		sGtaVector center = { hit.x, hit.y, hit.z };
		if (TryComputeFoundationFootprintGroundRange(center, minGroundZ, maxGroundZ))
		{
			hit.footprintMinGroundZ = minGroundZ;
			hit.footprintMaxGroundZ = maxGroundZ;
		}

		return true;
	}

	void ApplyBuildFoundationAimLock(sBuildAimHit& hit)
	{
		if (!hit.hasHit)
		{
			ResetBuildFoundationAimLock();
			return;
		}

		if (hit.footprintMinGroundZ > hit.footprintMaxGroundZ)
			std::swap(hit.footprintMinGroundZ, hit.footprintMaxGroundZ);

		if (gFoundationAimLock.valid)
		{
			const float dx = hit.x - gFoundationAimLock.x;
			const float dy = hit.y - gFoundationAimLock.y;
			const float radiusSq = BuildFoundationAimLockRadius * BuildFoundationAimLockRadius;
			if (((dx * dx) + (dy * dy)) <= radiusSq && IsLikelyBuildFoundationSelfHit(hit))
			{
				hit.hasHit = true;
				hit.x = gFoundationAimLock.x;
				hit.y = gFoundationAimLock.y;
				hit.z = gFoundationAimLock.z;
				hit.surfaceState = BuildAimSurfaceGround;
				hit.footprintMinGroundZ = gFoundationAimLock.footprintMinGroundZ;
				hit.footprintMaxGroundZ = gFoundationAimLock.footprintMaxGroundZ;
				return;
			}
		}

		if (hit.surfaceState != BuildAimSurfaceGround)
		{
			ResetBuildFoundationAimLock();
			return;
		}

		gFoundationAimLock.valid = true;
		gFoundationAimLock.x = hit.x;
		gFoundationAimLock.y = hit.y;
		gFoundationAimLock.z = hit.z;
		gFoundationAimLock.footprintMinGroundZ = hit.footprintMinGroundZ;
		gFoundationAimLock.footprintMaxGroundZ = hit.footprintMaxGroundZ;
	}

	bool TryProcessBuildAimLine(const sGtaVector& origin, const sGtaVector& direction, float distance, sBuildAimHit& hit, unsigned int selectedPartId)
	{
		typedef bool(__cdecl* ProcessLineOfSight_t)(
			const sGtaVector&,
			const sGtaVector&,
			sGtaColPoint&,
			void*&,
			bool,
			bool,
			bool,
			bool,
			bool,
			bool,
			bool,
			bool);

		sGtaVector target =
		{
			origin.x + (direction.x * distance),
			origin.y + (direction.y * distance),
			origin.z + (direction.z * distance)
		};
		if (!IsFiniteVector(target))
			return false;

		sGtaColPoint colPoint = {};
		void* entity = nullptr;
		bool lineHit = false;

		__try
		{
			ProcessLineOfSight_t processLineOfSight = reinterpret_cast<ProcessLineOfSight_t>(GtaProcessLineOfSightAddress);
			lineHit = processLineOfSight(
				origin,
				target,
				colPoint,
				entity,
				true,
				false,
				false,
				false,
				false,
				false,
				false,
				false);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}

		if (!lineHit || !IsFiniteVector(colPoint.point))
			return false;

		hit.hasHit = true;
		hit.x = colPoint.point.x;
		hit.y = colPoint.point.y;
		hit.z = colPoint.point.z;
		hit.surfaceState = BuildAimSurfaceGround;
		hit.footprintMinGroundZ = colPoint.point.z;
		hit.footprintMaxGroundZ = colPoint.point.z;

		const bool hasNormal = IsFiniteVector(colPoint.normal);
		const bool groundLikeNormal = hasNormal && colPoint.normal.z >= BuildAimGroundNormalMinZ;
		float centerGroundZ = colPoint.point.z;
		const bool hasCenterGround = TryFindTerrainGroundZ(colPoint.point.x, colPoint.point.y, centerGroundZ);
		const bool highAboveGround = hasCenterGround && colPoint.point.z > centerGroundZ + BuildAimHighSurfaceThreshold;
		float minGroundZ = colPoint.point.z;
		float maxGroundZ = colPoint.point.z;
		if (TryComputeFoundationFootprintGroundRange(colPoint.point, minGroundZ, maxGroundZ))
		{
			hit.footprintMinGroundZ = minGroundZ;
			hit.footprintMaxGroundZ = maxGroundZ;
		}
		else if (hasCenterGround)
		{
			hit.footprintMinGroundZ = centerGroundZ;
			hit.footprintMaxGroundZ = centerGroundZ;
		}

		if (!groundLikeNormal)
		{
			hit.surfaceState = BuildAimSurfaceBlockedNonGround;
		}
		else if (selectedPartId == BuildPartFoundation && highAboveGround)
		{
			hit.z = centerGroundZ;
		}
		else if (highAboveGround)
		{
			hit.surfaceState = BuildAimSurfaceBlockedNonGround;
		}

		return true;
	}

	bool TryComputeBuildAimHit(float maxDistance, sBuildAimHit& hit, unsigned int selectedPartId)
	{
		hit = {};

		unsigned char activeCam = 0;
		if (!TryReadMemory(GtaCameraBase + GtaCameraActiveCamOffset, &activeCam, sizeof(activeCam)) || activeCam >= 3)
			return false;

		const uintptr_t camBase = GtaCameraBase + GtaCameraCamsOffset + (static_cast<uintptr_t>(activeCam) * GtaCamSize);
		sGtaVector origin = {};
		sGtaVector direction = {};
		if (!TryReadGtaVector(camBase + GtaCamSourceOffset, origin) || !TryReadGtaVector(camBase + GtaCamFrontOffset, direction))
			return false;
		if (!NormalizeVector(direction))
			return false;

		sGtaVector reverse = { -direction.x, -direction.y, -direction.z };
		const float rayDistance = ClampFloat(maxDistance + 20.0f, 20.0f, 60.0f);
		const bool reverseFirst = reverse.z < direction.z;

		if (selectedPartId == BuildPartFoundation)
		{
			sBuildAimHit losHit = {};
			bool hasLosHit = false;
			if (reverseFirst)
			{
				hasLosHit = TryProcessBuildAimLine(origin, reverse, rayDistance, losHit, selectedPartId);
				if (!hasLosHit)
					hasLosHit = TryProcessBuildAimLine(origin, direction, rayDistance, losHit, selectedPartId);
			}
			else
			{
				hasLosHit = TryProcessBuildAimLine(origin, direction, rayDistance, losHit, selectedPartId);
				if (!hasLosHit)
					hasLosHit = TryProcessBuildAimLine(origin, reverse, rayDistance, losHit, selectedPartId);
			}

			if (hasLosHit && IsLikelyBuildFoundationSelfHit(losHit))
			{
				hit = losHit;
				return true;
			}

			if (hasLosHit && losHit.surfaceState == BuildAimSurfaceBlockedNonGround)
			{
				hit = losHit;
				return true;
			}

			if (hasLosHit && TryProjectFoundationAimToTerrain(losHit, hit))
			{
				return true;
			}

			if (hasLosHit)
			{
				hit = losHit;
				return true;
			}
			return false;
		}

		if (reverseFirst)
		{
			if (TryProcessBuildAimLine(origin, reverse, rayDistance, hit, selectedPartId))
				return true;
			return TryProcessBuildAimLine(origin, direction, rayDistance, hit, selectedPartId);
		}

		if (TryProcessBuildAimLine(origin, direction, rayDistance, hit, selectedPartId))
			return true;
		return TryProcessBuildAimLine(origin, reverse, rayDistance, hit, selectedPartId);
	}

	void WriteBuildAimPayload(RakNet::BitStream& bitStream, float maxDistance, unsigned int selectedPartId)
	{
		sBuildAimHit hit = {};
		const bool hasHit = TryComputeBuildAimHit(maxDistance, hit, selectedPartId);
		if (selectedPartId == BuildPartFoundation)
		{
			ApplyBuildFoundationAimLock(hit);
		}
		else
		{
			ResetBuildFoundationAimLock();
		}

		bitStream.Write(hasHit);
		bitStream.Write(hasHit ? hit.x : 0.0f);
		bitStream.Write(hasHit ? hit.y : 0.0f);
		bitStream.Write(hasHit ? hit.z : 0.0f);
		bitStream.Write(hasHit ? static_cast<unsigned char>(hit.surfaceState) : static_cast<unsigned char>(BuildAimSurfaceNone));
		bitStream.Write(hasHit ? hit.footprintMinGroundZ : 0.0f);
		bitStream.Write(hasHit ? hit.footprintMaxGroundZ : 0.0f);
	}

	float GetBuildMenuWidth(float displayWidth)
	{
		return (std::max)(280.0f, (std::min)(340.0f, displayWidth * 0.26f));
	}

	float GetBuildMenuX(float displayWidth, float menuWidth)
	{
		return ClampFloat(OverlayLayout::GetMenuX(displayWidth), BuildUiScreenPadding, displayWidth - menuWidth - BuildUiScreenPadding);
	}

	float GetBuildMenuY(float displayHeight)
	{
		return displayHeight * 0.5f;
	}

	ImVec2 GetOverlayDisplaySize()
	{
		ImGuiIO& io = ImGui::GetIO();
		float width = io.DisplaySize.x;
		float height = io.DisplaySize.y;

		if (width <= 1.0f || height <= 1.0f)
		{
			CPoint2D& resolution = CGraphics::GetScreenResolution();
			width = static_cast<float>(resolution.X());
			height = static_cast<float>(resolution.Y());
		}

		if (width <= 1.0f)
			width = 1280.0f;
		if (height <= 1.0f)
			height = 720.0f;

		return ImVec2(width, height);
	}

	ImVec2 ScaleClientPointToDisplay(HWND window, const POINT& point)
	{
		ImVec2 scaled(static_cast<float>(point.x), static_cast<float>(point.y));
		if (!window)
			return scaled;

		RECT client = {};
		if (!GetClientRect(window, &client))
			return scaled;

		const float clientWidth = static_cast<float>(client.right - client.left);
		const float clientHeight = static_cast<float>(client.bottom - client.top);
		if (clientWidth <= 1.0f || clientHeight <= 1.0f)
			return scaled;

		const ImVec2 displaySize = GetOverlayDisplaySize();
		const float displayWidth = displaySize.x;
		const float displayHeight = displaySize.y;

		scaled.x *= displayWidth / clientWidth;
		scaled.y *= displayHeight / clientHeight;
		return scaled;
	}

	bool TryReadWindowCursor(ImVec2& point)
	{
		HWND window = CMessageProxy::GetWindowHandle();
		if (!window)
			return false;

		POINT cursor = {};
		if (!GetCursorPos(&cursor) || !ScreenToClient(window, &cursor))
			return false;

		point = ScaleClientPointToDisplay(window, cursor);
		return true;
	}

	void MoveWindowCursorToDisplayPoint(const ImVec2& point)
	{
		HWND window = CMessageProxy::GetWindowHandle();
		if (!window)
			return;

		RECT client = {};
		if (!GetClientRect(window, &client))
			return;

		const float clientWidth = static_cast<float>(client.right - client.left);
		const float clientHeight = static_cast<float>(client.bottom - client.top);
		const ImVec2 displaySize = GetOverlayDisplaySize();
		const float displayWidth = displaySize.x;
		const float displayHeight = displaySize.y;
		if (clientWidth <= 1.0f || clientHeight <= 1.0f || displayWidth <= 1.0f || displayHeight <= 1.0f)
			return;

		POINT cursor = {};
		cursor.x = static_cast<LONG>(ClampFloat(point.x * clientWidth / displayWidth, 0.0f, clientWidth - 1.0f));
		cursor.y = static_cast<LONG>(ClampFloat(point.y * clientHeight / displayHeight, 0.0f, clientHeight - 1.0f));
		if (ClientToScreen(window, &cursor))
			SetCursorPos(cursor.x, cursor.y);
	}

	const char* ResultLabel(unsigned char result)
	{
		switch (result)
		{
		case OMPPlusProtocol::BuildResultSuccess:
			return "OK";
		case OMPPlusProtocol::BuildResultPreviewValid:
			return "VALID";
		case OMPPlusProtocol::BuildResultPreviewInvalid:
			return "INVALID";
		default:
			return "ERROR";
		}
	}

	bool SupportsBuildVariant(unsigned int partId)
	{
		return partId == BuildPartWall
			|| partId == BuildPartDoorFrame
			|| partId == BuildPartDoor;
	}

	bool SupportsFoundationHeight(unsigned int partId)
	{
		return partId == BuildPartFoundation;
	}

	int ClampFoundationHeightStep(int step)
	{
		return (std::max)(0, (std::min)(step, BuildFoundationLiftSteps));
	}

	int DefaultBuildRotationStep(unsigned int partId)
	{
		return SupportsFoundationHeight(partId) ? 0 : 0;
	}

	bool IsRemoveTool(unsigned int partId)
	{
		return partId == BuildPartRemove;
	}

	const char* BuildPartDisplayName(unsigned int partId)
	{
		switch (partId)
		{
		case BuildPartFoundation:
			return "Foundation";
		case BuildPartWall:
			return "Wall";
		case BuildPartDoorFrame:
			return "Door Frame";
		case BuildPartFloor:
			return "Floor/Ceiling";
		case BuildPartRoof:
			return "Roof";
		case BuildPartStairs:
			return "Stairs";
		case BuildPartDoor:
			return "Door";
		case BuildPartRemove:
			return "Remove";
		default:
			return "Build Part";
		}
	}

	const char* BuildControlHint(unsigned int partId, bool flipped)
	{
		switch (partId)
		{
		case BuildPartRemove:
			return "Remove mode  |  Aim placed part  |  LMB remove  RMB menu";
		case BuildPartFoundation:
			return "Q/E first foundation height  |  LMB place  RMB menu";
		case BuildPartWall:
		case BuildPartDoorFrame:
			return flipped
				? "Outer side  |  MMB switch side  |  LMB place  RMB menu"
				: "Inner side  |  MMB switch side  |  LMB place  RMB menu";
		case BuildPartDoor:
			return flipped
				? "Opens outward  |  MMB switch side  |  LMB place  RMB menu"
				: "Opens inward  |  MMB switch side  |  LMB place  RMB menu";
		case BuildPartStairs:
			return "Aim edge to choose direction  |  LMB place  RMB menu";
		case 0:
			return "Select a part  |  RMB close";
		default:
			return "LMB place  |  RMB menu";
		}
	}
}

void CBuildManager::HandleOpen(RakNet::BitStream& bitStream)
{
	unsigned int sessionId = 0;
	float maxDistance = 8.0f;
	std::string title;
	if (!bitStream.Read(sessionId) || !bitStream.Read(maxDistance) || !ReadBoundString(bitStream, title, 48) || sessionId == 0)
		return;

	cScopedBuildStateLock lock;
	ClearUnlocked(true);
	CTargetManager::ClearContext();

	m_active = true;
	m_menuOpen = true;
	m_sessionId = sessionId;
	m_maxDistance = maxDistance > 0.0f ? maxDistance : 8.0f;
	m_title = title.empty() ? "Build Mode" : title;
	m_selectedPartId = 0;
	m_rotationStep = 0;
	m_flipped = false;
	m_nextPreviewSendAt = 0;
	m_removeTarget = sBuildRemoveTarget();
	m_status.clear();
	m_statusUntil = 0;
	m_inputGuardUntil = GetTickCount() + 450;
	m_lastLeftDown = m_mouseButtons[0] || IsPhysicalKeyDown(VK_LBUTTON);
	m_lastRightDown = m_mouseButtons[1] || IsPhysicalKeyDown(VK_RBUTTON);
	m_lastMiddleDown = m_mouseButtons[2] || IsPhysicalKeyDown(VK_MBUTTON);
	m_lastEscapeDown = IsPhysicalKeyDown(VK_ESCAPE);
	m_lastQDown = IsPhysicalKeyDown('Q');
	m_lastEDown = IsPhysicalKeyDown('E');
	memset(m_mouseButtons, 0, sizeof(m_mouseButtons));
	m_virtualCursorInitialized = false;
	m_cursorOwned = !CGraphics::IsCursorEnabled();
	CGraphics::ToggleCursor(true);
	CDInput8DeviceProxy::RequestInputReset();
	CLog::Write("Build UI opened session=%u", sessionId);
}

void CBuildManager::HandleClose()
{
	Clear();
}

void CBuildManager::HandleClearParts()
{
	cScopedBuildStateLock lock;
	m_parts.clear();
	m_selectedPartId = 0;
}

void CBuildManager::HandleAddPart(RakNet::BitStream& bitStream)
{
	sBuildPart part = {};
	if (!bitStream.Read(part.partId) || !bitStream.Read(part.modelId))
		return;
	if (!ReadBoundString(bitStream, part.name, 48)
		|| !ReadBoundString(bitStream, part.category, 32)
		|| !ReadBoundString(bitStream, part.cost, 48))
		return;
	if (part.partId == 0 || part.name.empty())
		return;

	cScopedBuildStateLock lock;
	const auto existing = std::find_if(m_parts.begin(), m_parts.end(), [part](const sBuildPart& current)
	{
		return current.partId == part.partId;
	});
	if (existing != m_parts.end())
		*existing = part;
	else
		m_parts.push_back(part);

	// Do not auto-select the first part. Preview starts only after the player
	// deliberately chooses a part, then the client enters placement mode.
}

void CBuildManager::HandleResult(RakNet::BitStream& bitStream)
{
	unsigned char result = 0;
	std::string message;
	if (!bitStream.Read(result) || !ReadBoundString(bitStream, message, 96))
		return;

	cScopedBuildStateLock lock;
	m_statusSuccess = result == OMPPlusProtocol::BuildResultSuccess || result == OMPPlusProtocol::BuildResultPreviewValid;
	m_status = message.empty() ? ResultLabel(result) : message;
	m_statusUntil = GetTickCount() + 2200;
}

void CBuildManager::HandleRemoveTarget(RakNet::BitStream& bitStream)
{
	bool active = false;
	unsigned int partId = 0;
	float distance = 0.0f;
	std::string label;
	if (!bitStream.Read(active) || !bitStream.Read(partId) || !bitStream.Read(distance) || !ReadBoundString(bitStream, label, 48))
		return;

	cScopedBuildStateLock lock;
	m_removeTarget.active = active;
	m_removeTarget.partId = active ? partId : 0;
	m_removeTarget.distance = active ? (std::max)(0.0f, distance) : 0.0f;
	m_removeTarget.label = active ? label : std::string();
	m_removeTarget.receivedAt = GetTickCount();
}

void CBuildManager::Clear()
{
	cScopedBuildStateLock lock;
	ClearUnlocked(true);
}

void CBuildManager::ClearUnlocked(bool restoreCursor)
{
	const bool wasActive = m_active;
	m_active = false;
	m_menuOpen = false;
	m_sessionId = 0;
	m_selectedPartId = 0;
	m_maxDistance = 8.0f;
	m_rotationStep = 0;
	m_flipped = false;
	m_nextPreviewSendAt = 0;
	m_inputGuardUntil = 0;
	m_status.clear();
	m_statusUntil = 0;
	m_removeTarget = sBuildRemoveTarget();
	m_title.clear();
	m_parts.clear();
	m_virtualCursorInitialized = false;
	m_virtualWheel = 0;
	memset(m_mouseButtons, 0, sizeof(m_mouseButtons));
	m_lastLeftDown = m_lastRightDown = m_lastMiddleDown = false;
	m_lastEscapeDown = m_lastQDown = m_lastEDown = false;
	m_menuX = m_menuY = m_menuW = m_menuH = 0.0f;

	if (wasActive)
		CDInput8DeviceProxy::RequestInputReset();

	if (restoreCursor && m_cursorOwned)
	{
		CGraphics::ToggleCursor(false);
		m_cursorOwned = false;
	}
}

void CBuildManager::Process()
{
	cScopedBuildStateLock lock;
	if (!m_active)
		return;

	const bool escapeDown = IsPhysicalKeyDown(VK_ESCAPE);
	const bool rightDown = m_mouseButtons[1] || IsPhysicalKeyDown(VK_RBUTTON);
	const bool leftDown = m_mouseButtons[0] || IsPhysicalKeyDown(VK_LBUTTON);
	const bool middleDown = m_mouseButtons[2] || IsPhysicalKeyDown(VK_MBUTTON);
	const bool qDown = IsPhysicalKeyDown('Q');
	const bool eDown = IsPhysicalKeyDown('E');
	const unsigned long now = GetTickCount();
	const bool inputGuardActive = static_cast<long>(now - m_inputGuardUntil) < 0;

	if (!inputGuardActive && escapeDown && !m_lastEscapeDown)
	{
		SendCancel();
		return;
	}

	if (!inputGuardActive && rightDown && !m_lastRightDown)
	{
		if (m_menuOpen)
			SendCancel();
		else
			ReturnToMenu();
		return;
	}

	if (m_menuOpen)
	{
		m_lastEscapeDown = escapeDown;
		m_lastRightDown = rightDown;
		m_lastLeftDown = leftDown;
		m_lastMiddleDown = middleDown;
		m_lastQDown = qDown;
		m_lastEDown = eDown;
		return;
	}

	if (SupportsFoundationHeight(m_selectedPartId))
	{
		int heightStep = m_rotationStep;
		if (qDown && !m_lastQDown)
			--heightStep;
		if (eDown && !m_lastEDown)
			++heightStep;
		heightStep = ClampFoundationHeightStep(heightStep);
		if (heightStep != m_rotationStep)
		{
			m_rotationStep = heightStep;
			SendPreviewState();
		}
	}

	if (middleDown && !m_lastMiddleDown && SupportsBuildVariant(m_selectedPartId))
	{
		m_flipped = !m_flipped;
		SendPreviewState();
	}

	if (m_selectedPartId != 0 && !IsRemoveTool(m_selectedPartId) && static_cast<long>(now - m_nextPreviewSendAt) >= 0)
		SendPreviewState();

	if (leftDown && !m_lastLeftDown && m_selectedPartId != 0 && !IsMouseOverMenu())
		SendPlace();

	m_lastEscapeDown = escapeDown;
	m_lastRightDown = rightDown;
	m_lastLeftDown = leftDown;
	m_lastMiddleDown = middleDown;
	m_lastQDown = qDown;
	m_lastEDown = eDown;
}

void CBuildManager::RenderImGui()
{
	cScopedBuildStateLock lock;
	if (!m_active)
		return;

	if (!m_menuOpen)
	{
		if (IsRemoveTool(m_selectedPartId))
			RenderRemoveOverlay();
		return;
	}

	ApplyImGuiInput();

	const ImVec2 displaySize = GetOverlayDisplaySize();
	const float width = GetBuildMenuWidth(displaySize.x);
	const float x = GetBuildMenuX(displaySize.x, width);
	const float y = GetBuildMenuY(displaySize.y);

	ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always, ImVec2(0.0f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Always);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 9.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(7.0f, 6.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.01f, 0.01f, 0.012f, 0.94f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.95f, 0.95f, 0.92f, 0.45f));

	if (ImGui::Begin("##ompplus_build_menu", NULL, flags))
	{
		ImGui::TextUnformatted(m_title.empty() ? "Build Mode" : m_title.c_str());
		ImGui::Separator();

		if (m_parts.empty())
		{
			ImGui::TextWrapped("No build parts were sent by the server.");
		}
		else
		{
			std::string currentCategory;
			for (const sBuildPart& part : m_parts)
			{
				if (part.category != currentCategory)
				{
					currentCategory = part.category;
					if (!currentCategory.empty())
					{
						ImGui::Spacing();
						ImGui::TextUnformatted(currentCategory.c_str());
					}
				}

				const bool selected = part.partId == m_selectedPartId;
				const bool removeTool = IsRemoveTool(part.partId);
				ImVec4 normal = selected ? ImVec4(0.24f, 0.25f, 0.25f, 0.98f) : ImVec4(0.025f, 0.030f, 0.032f, 0.94f);
				ImVec4 hover = ImVec4(0.34f, 0.35f, 0.35f, 1.00f);
				ImVec4 active = ImVec4(0.46f, 0.47f, 0.47f, 1.00f);
				if (removeTool)
				{
					normal = selected ? ImVec4(0.84f, 0.38f, 0.10f, 0.98f) : ImVec4(0.42f, 0.18f, 0.05f, 0.96f);
					hover = ImVec4(0.94f, 0.48f, 0.14f, 1.00f);
					active = ImVec4(1.00f, 0.58f, 0.20f, 1.00f);
				}
				ImGui::PushStyleColor(ImGuiCol_Button, normal);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);

				char label[160] = {};
				if (part.cost.empty())
					std::snprintf(label, sizeof(label), "%s##buildpart%u", part.name.c_str(), part.partId);
				else
					std::snprintf(label, sizeof(label), "%s    %s##buildpart%u", part.name.c_str(), part.cost.c_str(), part.partId);

				if (ImGui::Button(label, ImVec2(-1.0f, 30.0f)))
				{
					m_selectedPartId = part.partId;
					m_rotationStep = DefaultBuildRotationStep(part.partId);
					m_flipped = false;
					m_nextPreviewSendAt = 0;
					m_removeTarget = sBuildRemoveTarget();
					m_menuOpen = false;
					if (m_cursorOwned)
					{
						CGraphics::ToggleCursor(false);
						m_cursorOwned = false;
					}
					m_lastLeftDown = m_mouseButtons[0] || IsPhysicalKeyDown(VK_LBUTTON);
					m_lastRightDown = m_mouseButtons[1] || IsPhysicalKeyDown(VK_RBUTTON);
					m_lastMiddleDown = m_mouseButtons[2] || IsPhysicalKeyDown(VK_MBUTTON);
					m_lastQDown = IsPhysicalKeyDown('Q');
					m_lastEDown = IsPhysicalKeyDown('E');
					m_virtualCursorInitialized = false;
					CDInput8DeviceProxy::RequestInputReset();
					SendSelect(part.partId);
					if (!IsRemoveTool(part.partId))
						SendPreviewState();
				}

				ImGui::PopStyleColor(3);
			}
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::TextWrapped("%s", BuildControlHint(m_selectedPartId, m_flipped));

		if (!m_status.empty() && GetTickCount() <= m_statusUntil)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, m_statusSuccess ? ImVec4(0.82f, 1.0f, 0.82f, 1.0f) : ImVec4(1.0f, 0.72f, 0.72f, 1.0f));
			ImGui::TextWrapped("%s", m_status.c_str());
			ImGui::PopStyleColor();
		}

		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 size = ImGui::GetWindowSize();
		m_menuX = pos.x;
		m_menuY = pos.y;
		m_menuW = size.x;
		m_menuH = size.y;
	}
	ImGui::End();

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(2);
}

bool CBuildManager::IsActive()
{
	return m_active;
}

bool CBuildManager::IsMenuOpen()
{
	return m_active && m_menuOpen;
}

bool CBuildManager::ShouldCaptureMouse()
{
	return m_active;
}

bool CBuildManager::ShouldPassMouseMovement()
{
	return m_active && !m_menuOpen && m_selectedPartId != 0;
}

bool CBuildManager::ShouldBlockCursorMove()
{
	return m_active && m_menuOpen;
}

bool CBuildManager::ShouldSuppressKeyboard()
{
	return m_active && m_menuOpen;
}

bool CBuildManager::ShouldNeutralizeKeyboard()
{
	return m_active && m_menuOpen;
}

bool CBuildManager::ShouldBlockGameControls()
{
	return m_active && m_menuOpen;
}

bool CBuildManager::ShouldBlockWeaponCycleControls()
{
	return m_active && !m_menuOpen && SupportsFoundationHeight(m_selectedPartId);
}

bool CBuildManager::ShouldConsumeDirectInputEvent(DWORD offset, DWORD)
{
	if (!m_active)
		return false;
	if (m_menuOpen)
		return true;
	if (SupportsFoundationHeight(m_selectedPartId) && (offset == DIK_Q || offset == DIK_E))
		return true;
	return offset == DIK_ESCAPE;
}

void CBuildManager::FilterKeyboardState(DWORD size, LPVOID state)
{
	if (!m_active || !state)
		return;

	if (m_menuOpen)
	{
		std::memset(state, 0, size);
		return;
	}

	if (size >= 256)
	{
		BYTE* keyboard = reinterpret_cast<BYTE*>(state);
		keyboard[DIK_ESCAPE] = 0;
		if (SupportsFoundationHeight(m_selectedPartId))
		{
			keyboard[DIK_Q] = 0;
			keyboard[DIK_E] = 0;
		}
	}
}

void CBuildManager::FilterMouseState(DWORD size, LPVOID state)
{
	if (!m_active || !state)
		return;

	if (size >= sizeof(DIMOUSESTATE2))
	{
		DIMOUSESTATE2* mouse = reinterpret_cast<DIMOUSESTATE2*>(state);
		if (m_menuOpen)
		{
			mouse->lX = 0;
			mouse->lY = 0;
		}
		std::memset(mouse->rgbButtons, 0, sizeof(mouse->rgbButtons));
		mouse->lZ = 0;
	}
	else if (size >= sizeof(DIMOUSESTATE))
	{
		DIMOUSESTATE* mouse = reinterpret_cast<DIMOUSESTATE*>(state);
		if (m_menuOpen)
		{
			mouse->lX = 0;
			mouse->lY = 0;
		}
		std::memset(mouse->rgbButtons, 0, sizeof(mouse->rgbButtons));
		mouse->lZ = 0;
	}
}

DWORD CBuildManager::GetKeyboardReleaseOffsets(DWORD* offsets, DWORD capacity)
{
	if (!m_active || !offsets || !capacity)
		return 0;

	if (m_menuOpen)
	{
		const DWORD available = static_cast<DWORD>(sizeof(KeyboardReleaseKeys) / sizeof(KeyboardReleaseKeys[0]));
		const DWORD count = capacity < available ? capacity : available;
		for (DWORD i = 0; i < count; ++i)
			offsets[i] = KeyboardReleaseKeys[i].offset;
		return count;
	}

	DWORD count = 0;
	offsets[count++] = DIK_ESCAPE;
	if (SupportsFoundationHeight(m_selectedPartId))
	{
		if (count < capacity)
			offsets[count++] = DIK_Q;
		if (count < capacity)
			offsets[count++] = DIK_E;
	}
	return count;
}

void CBuildManager::AddMouseDelta(LONG x, LONG y, LONG wheel)
{
	if (!m_active)
		return;

	if (!m_virtualCursorInitialized)
		InitializeVirtualCursor();

	m_virtualCursorX += static_cast<float>(x);
	m_virtualCursorY += static_cast<float>(y);
	m_virtualWheel += wheel;
}

void CBuildManager::SetWindowMousePosition(LONG x, LONG y)
{
	if (!m_active)
		return;

	POINT point = { x, y };
	const ImVec2 scaled = ScaleClientPointToDisplay(CMessageProxy::GetWindowHandle(), point);
	m_virtualCursorX = scaled.x;
	m_virtualCursorY = scaled.y;
	m_virtualCursorInitialized = true;
}

void CBuildManager::SetMouseButton(unsigned int button, bool down)
{
	if (button >= 5)
		return;
	m_mouseButtons[button] = down;
}

bool CBuildManager::ReadBoundString(RakNet::BitStream& bitStream, std::string& value, unsigned char maxLength)
{
	unsigned char length = 0;
	if (!bitStream.Read(length) || length > maxLength)
		return false;

	char buffer[128] = {};
	if (length && !bitStream.Read(buffer, length))
		return false;

	value.assign(buffer, length);
	return true;
}

void CBuildManager::InitializeVirtualCursor()
{
	const ImVec2 displaySize = GetOverlayDisplaySize();
	const float menuWidth = GetBuildMenuWidth(displaySize.x);
	const float menuX = GetBuildMenuX(displaySize.x, menuWidth);

	m_virtualCursorX = menuX + menuWidth * 0.5f;
	m_virtualCursorY = GetBuildMenuY(displaySize.y);
	m_virtualCursorInitialized = true;
	MoveWindowCursorToDisplayPoint(ImVec2(m_virtualCursorX, m_virtualCursorY));
}

void CBuildManager::ApplyImGuiInput()
{
	ImGuiIO& io = ImGui::GetIO();

	if (!m_virtualCursorInitialized)
		InitializeVirtualCursor();

	ImVec2 windowCursor;
	if (TryReadWindowCursor(windowCursor))
	{
		m_virtualCursorX = windowCursor.x;
		m_virtualCursorY = windowCursor.y;
	}

	const float maxX = (std::max)(1.0f, io.DisplaySize.x - 1.0f);
	const float maxY = (std::max)(1.0f, io.DisplaySize.y - 1.0f);
	m_virtualCursorX = ClampFloat(m_virtualCursorX, 0.0f, maxX);
	m_virtualCursorY = ClampFloat(m_virtualCursorY, 0.0f, maxY);

	io.MousePos = ImVec2(m_virtualCursorX, m_virtualCursorY);
	io.MouseDown[0] = m_mouseButtons[0] || IsPhysicalKeyDown(VK_LBUTTON);
	io.MouseDown[1] = m_mouseButtons[1] || IsPhysicalKeyDown(VK_RBUTTON);
	io.MouseDown[2] = m_mouseButtons[2] || IsPhysicalKeyDown(VK_MBUTTON);
	for (int i = 3; i < 5; ++i)
		io.MouseDown[i] = m_mouseButtons[i];

	if (m_virtualWheel != 0)
	{
		io.MouseWheel += static_cast<float>(m_virtualWheel) / 120.0f;
		m_virtualWheel = 0;
	}
}

void CBuildManager::RenderRemoveOverlay()
{
	const ImVec2 displaySize = GetOverlayDisplaySize();
	const float cx = OverlayLayout::GetTargetCenterX(displaySize.x);
	const float cy = displaySize.y * 0.5f;
	const unsigned long now = GetTickCount();
	const bool targetFresh = m_removeTarget.active && now - m_removeTarget.receivedAt <= 650;

	const char* fallbackName = BuildPartDisplayName(m_removeTarget.partId);
	std::string targetLabel = targetFresh && !m_removeTarget.label.empty() ? m_removeTarget.label : fallbackName;
	if (!targetFresh)
		targetLabel = "No build target";

	char rowAction[96] = {};
	if (targetFresh && m_removeTarget.distance > 0.01f)
		std::snprintf(rowAction, sizeof(rowAction), "LMB remove  |  %.1fm", m_removeTarget.distance);
	else
		std::snprintf(rowAction, sizeof(rowAction), "Aim placed part  |  RMB menu");

	const char* badgeMode = "REMOVE";
	const std::string badgeTitle = targetFresh ? targetLabel : "MODE";
	const ImVec2 modeText = ImGui::CalcTextSize(badgeMode);
	const ImVec2 titleText = ImGui::CalcTextSize(badgeTitle.c_str());
	const ImVec2 actionText = ImGui::CalcTextSize(rowAction);
	const float tagWidth = modeText.x + 14.0f;
	const float titleRowWidth = 10.0f + tagWidth + 8.0f + titleText.x + 12.0f;
	const float actionRowWidth = actionText.x + 20.0f;
	const float badgeWidth = ClampFloat((std::max)(titleRowWidth, actionRowWidth), RemoveBadgeMinWidth, RemoveBadgeMaxWidth);
	const float badgeX = ClampFloat(OverlayLayout::GetMenuX(displaySize.x), BuildUiScreenPadding, displaySize.x - badgeWidth - BuildUiScreenPadding);
	const float badgeY = ClampFloat(cy - RemoveBadgeHeight * 0.5f, BuildUiScreenPadding, displaySize.y - RemoveBadgeHeight - BuildUiScreenPadding);
	const ImVec2 badgeMin(badgeX, badgeY);
	const ImVec2 badgeMax(badgeMin.x + badgeWidth, badgeMin.y + RemoveBadgeHeight);

	ImDrawList* draw = ImGui::GetForegroundDrawList();
	const ImU32 bg = IM_COL32(3, 4, 4, targetFresh ? 224 : 190);
	const ImU32 bgSoft = IM_COL32(0, 0, 0, targetFresh ? 130 : 90);
	const ImU32 accent = targetFresh ? IM_COL32(255, 126, 28, 245) : IM_COL32(160, 118, 78, 150);
	const ImU32 accentSoft = targetFresh ? IM_COL32(255, 126, 28, 74) : IM_COL32(130, 100, 74, 48);
	const ImU32 text = targetFresh ? IM_COL32(245, 240, 230, 245) : IM_COL32(196, 188, 176, 185);
	const ImU32 textSoft = targetFresh ? IM_COL32(235, 226, 212, 225) : IM_COL32(168, 158, 146, 170);

	draw->AddRectFilled(ImVec2(badgeMin.x + 3.0f, badgeMin.y + 3.0f), ImVec2(badgeMax.x + 3.0f, badgeMax.y + 3.0f), bgSoft, 6.0f);
	draw->AddRectFilled(badgeMin, badgeMax, bg, 6.0f);
	draw->AddRect(badgeMin, badgeMax, accent, 6.0f, 0, 1.3f);
	draw->AddRectFilled(ImVec2(badgeMin.x, badgeMin.y), ImVec2(badgeMin.x + 4.0f, badgeMax.y), accent, 6.0f);

	const ImVec2 tagMin(badgeMin.x + 10.0f, badgeMin.y + 6.0f);
	const ImVec2 tagMax(tagMin.x + tagWidth, tagMin.y + 18.0f);
	draw->AddRectFilled(tagMin, tagMax, accentSoft, 4.0f);
	draw->AddText(ImVec2(tagMin.x + 7.0f, tagMin.y + 3.0f), accent, badgeMode);

	const float titleX = tagMax.x + 8.0f;
	const ImVec4 titleClip(titleX, badgeMin.y + 4.0f, badgeMax.x - 9.0f, badgeMin.y + 25.0f);
	draw->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(titleX, badgeMin.y + 9.0f), text, badgeTitle.c_str(), badgeTitle.c_str() + badgeTitle.size(), 0.0f, &titleClip);
	draw->AddRectFilled(ImVec2(badgeMin.x + 10.0f, badgeMin.y + 27.0f), ImVec2(badgeMax.x - 10.0f, badgeMin.y + 28.0f), accentSoft, 1.0f);
	draw->AddText(ImVec2(badgeMin.x + 10.0f, badgeMin.y + 30.0f), textSoft, rowAction);
}

bool CBuildManager::IsMouseOverMenu()
{
	if (m_menuW <= 0.0f || m_menuH <= 0.0f)
		return true;
	return m_virtualCursorX >= m_menuX && m_virtualCursorX <= m_menuX + m_menuW
		&& m_virtualCursorY >= m_menuY && m_virtualCursorY <= m_menuY + m_menuH;
}

void CBuildManager::ReturnToMenu()
{
	if (!m_active)
		return;

	SendClearPreview();

	m_menuOpen = true;
	m_selectedPartId = 0;
	m_rotationStep = 0;
	m_flipped = false;
	m_nextPreviewSendAt = 0;
	m_status.clear();
	m_statusUntil = 0;
	m_inputGuardUntil = GetTickCount() + 350;
	m_lastLeftDown = m_mouseButtons[0] || IsPhysicalKeyDown(VK_LBUTTON);
	m_lastRightDown = m_mouseButtons[1] || IsPhysicalKeyDown(VK_RBUTTON);
	m_lastMiddleDown = m_mouseButtons[2] || IsPhysicalKeyDown(VK_MBUTTON);
	m_lastEscapeDown = IsPhysicalKeyDown(VK_ESCAPE);
	m_lastQDown = IsPhysicalKeyDown('Q');
	m_lastEDown = IsPhysicalKeyDown('E');
	memset(m_mouseButtons, 0, sizeof(m_mouseButtons));
	m_virtualCursorInitialized = false;

	if (!CGraphics::IsCursorEnabled())
	{
		m_cursorOwned = true;
		CGraphics::ToggleCursor(true);
	}
	else
	{
		m_cursorOwned = false;
	}

	CDInput8DeviceProxy::RequestInputReset();
}

void CBuildManager::SendSelect(unsigned int partId)
{
	if (!Network::IsConnected() || !m_active || m_sessionId == 0 || partId == 0)
		return;

	if (partId == BuildPartFoundation)
		ResetBuildFoundationAimLock();

	RakNet::BitStream bitStream;
	bitStream.Write(m_sessionId);
	bitStream.Write(partId);
	Network::SendRPC(eRPC::ON_BUILD_SELECT, &bitStream);
}

void CBuildManager::SendClearPreview()
{
	if (!Network::IsConnected() || !m_active || m_sessionId == 0)
		return;

	RakNet::BitStream bitStream;
	bitStream.Write(m_sessionId);
	bitStream.Write(0u);
	bitStream.Write(static_cast<short>(0));
	bitStream.Write(false);
	WriteBuildAimPayload(bitStream, m_maxDistance, 0);
	Network::SendRPC(eRPC::ON_BUILD_PREVIEW, &bitStream);
}

void CBuildManager::SendPreviewState()
{
	if (!Network::IsConnected() || !m_active || m_sessionId == 0 || m_selectedPartId == 0)
		return;

	RakNet::BitStream bitStream;
	bitStream.Write(m_sessionId);
	bitStream.Write(m_selectedPartId);
	bitStream.Write(static_cast<short>(m_rotationStep));
	bitStream.Write(m_flipped);
	WriteBuildAimPayload(bitStream, m_maxDistance, m_selectedPartId);
	Network::SendRPC(eRPC::ON_BUILD_PREVIEW, &bitStream);
	m_nextPreviewSendAt = GetTickCount() + BuildPreviewSendIntervalMs;
}

void CBuildManager::SendPlace()
{
	if (!Network::IsConnected() || !m_active || m_sessionId == 0 || m_selectedPartId == 0)
		return;

	RakNet::BitStream bitStream;
	bitStream.Write(m_sessionId);
	bitStream.Write(m_selectedPartId);
	bitStream.Write(static_cast<short>(m_rotationStep));
	bitStream.Write(m_flipped);
	WriteBuildAimPayload(bitStream, m_maxDistance, m_selectedPartId);
	Network::SendRPC(eRPC::ON_BUILD_PLACE, &bitStream);
}

void CBuildManager::SendCancel()
{
	if (Network::IsConnected() && m_active && m_sessionId != 0)
	{
		RakNet::BitStream bitStream;
		bitStream.Write(m_sessionId);
		Network::SendRPC(eRPC::ON_BUILD_CANCEL, &bitStream);
	}

	ClearUnlocked(true);
}
