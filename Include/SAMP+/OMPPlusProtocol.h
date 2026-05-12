#pragma once

#include <stdint.h>

namespace OMPPlusProtocol
{
	static const uint8_t PacketID = 0x87;
	static const int RpcID = 220;
	static const uint32_t Magic = 0x4F4D5050; // OMPP
	static const uint16_t Version = 1;
	static const uint32_t CapabilityNativeTransport = 0x00000001;
	static const uint32_t CapabilityKeyCapture = 0x00000002;
	static const uint32_t CapabilityTargetUI = 0x00000004;
	static const uint32_t CapabilityTargetUIV2 = 0x00000008;
	static const uint32_t CapabilityBuildUI = 0x00000010;
	static const uint32_t DefaultCapabilities = CapabilityNativeTransport | CapabilityKeyCapture;
	static const uint8_t ClientInfoVersion = 1;
	static const uint16_t ClientVersionMajor = 0;
	static const uint16_t ClientVersionMinor = 1;
	static const uint16_t ClientVersionPatch = 27;
	static const uint8_t MaxClientHashLength = 64;
	static const int MaxPayloadBytes = 4096;
	static const int MaxMessagesPerSecond = 60;

	enum Feature : uint32_t
	{
		FeatureHUD = 1 << 0,
		FeatureKeybind = 1 << 1,
		FeatureKeyCapture = 1 << 2,
		FeatureAudio = 1 << 3,
		FeatureEffects = 1 << 4,
		FeatureUI = 1 << 5,
		FeatureTarget = 1 << 6,
		FeatureBuild = 1 << 7,

		DefaultFeatures = FeatureHUD | FeatureKeybind | FeatureKeyCapture
	};

	enum BuildResult : uint8_t
	{
		BuildResultSuccess = 1,
		BuildResultError = 2,
		BuildResultPreviewValid = 3,
		BuildResultPreviewInvalid = 4
	};

	enum TargetFlags : uint32_t
	{
		TargetFlagHidePrompt = 1 << 0,
		TargetFlagPayloadV2 = 1u << 30
	};

	enum TargetType : uint8_t
	{
		TargetTypeGeneric = 0,
		TargetTypeVehicle = 1,
		TargetTypeHouse = 2,
		TargetTypeNPC = 3,
		TargetTypeActor = 4,
		TargetTypeObject = 5,
		TargetTypeItem = 6,
		TargetTypePlayer = 7,
		TargetTypeCustom = 8
	};

	enum TargetLayout : uint8_t
	{
		TargetLayoutAuto = 0,
		TargetLayoutCompact = 1,
		TargetLayoutStandard = 2,
		TargetLayoutDialog = 3,
		TargetLayoutWide = 4,
		TargetLayoutMinimal = 5,
		TargetLayoutCategory = 6
	};

	enum TargetRowType : uint8_t
	{
		TargetRowAction = 0,
		TargetRowInfo = 1,
		TargetRowDialog = 2,
		TargetRowDivider = 3,
		TargetRowHeader = 4,
		TargetRowDisabled = 5,
		TargetRowToggle = 6,
		TargetRowDanger = 7
	};

	enum class Message : uint8_t
	{
		Hello = 1,
		HelloAck = 2,
		ServerRPC = 3,
		ClientRPC = 4,
		Error = 5
	};

#define OMPPLUS_LEGACY_RPC_LIST(X) \
	X(TOGGLE_HUD_COMPONENT) \
	X(SET_RADIO_STATION) \
	X(SET_WAVE_HEIGHT) \
	X(TOGGLE_PAUSE_MENU) \
	X(SET_HUD_COMPONENT_COLOUR) \
	X(TOGGLE_ACTION) \
	X(SET_CLIP_AMMO) \
	X(SET_NO_RELOAD) \
	X(SET_BLUR_INTENSITY) \
	X(TOGGLE_DRIVE_ON_WATER) \
	X(SET_GAME_SPEED) \
	X(TOGGLE_PLAYER_FROZEN) \
	X(SET_PLAYER_ANIMS) \
	X(TOGGLE_SWITCH_RELOAD) \
	X(TOGGLE_INFINITE_RUN) \
	X(SET_AIRCRAFT_HEIGHT) \
	X(SET_JETPACK_HEIGHT) \
	X(SET_CHECKPOINT_EX) \
	X(SET_RACE_CHECKPOINT_EX) \
	X(SET_CHECKPOINT_COLOUR) \
	X(SET_RACE_CHECKPOINT_COLOUR) \
	X(TOGGLE_VEHICLE_BLIPS) \
	X(TOGGLE_INFINITE_OXYGEN) \
	X(TOGGLE_WATER_BUOYANCY) \
	X(TOGGLE_UNDERWATER_EFFECT) \
	X(TOGGLE_NIGHTVISION) \
	X(TOGGLE_THERMALVISION) \
	X(SET_KEY_BIND) \
	X(UNBIND_KEY) \
	X(CLEAR_KEY_BINDS) \
	X(ON_PAUSE_MENU_TOGGLE) \
	X(ON_PAUSE_MENU_CHANGE) \
	X(ON_DRIVE_BY_SHOT) \
	X(ON_STUNT_BONUS) \
	X(ON_RESOLUTION_CHANGE) \
	X(ON_MOUSE_CLICK) \
	X(ON_RADIO_CHANGE) \
	X(ON_DRINK_SPRUNK) \
	X(ON_KEY_STATE_CHANGE) \
	X(SET_KEY_CAPTURE) \
	X(CLEAR_KEY_CAPTURE) \
	X(CLEAR_KEY_CAPTURES) \
	X(TARGET_SET_CONTEXT) \
	X(TARGET_CLEAR_CONTEXT) \
	X(ON_TARGET_SELECT) \
	X(ON_TARGET_MODE) \
	X(BUILD_OPEN) \
	X(BUILD_CLOSE) \
	X(BUILD_CLEAR_PARTS) \
	X(BUILD_ADD_PART) \
	X(BUILD_RESULT) \
	X(ON_BUILD_SELECT) \
	X(ON_BUILD_PLACE) \
	X(ON_BUILD_CANCEL) \
	X(ON_BUILD_PREVIEW) \
	X(BUILD_REMOVE_TARGET)

	enum LegacyRPC : uint16_t
	{
#define OMPPLUS_LEGACY_RPC_ENUM(name) name,
		OMPPLUS_LEGACY_RPC_LIST(OMPPLUS_LEGACY_RPC_ENUM)
#undef OMPPLUS_LEGACY_RPC_ENUM
	};
}
