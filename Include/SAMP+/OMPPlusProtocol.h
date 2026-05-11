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

	enum LegacyRPC : uint16_t
	{
		TOGGLE_HUD_COMPONENT,
		SET_RADIO_STATION,
		SET_WAVE_HEIGHT,
		TOGGLE_PAUSE_MENU,
		SET_HUD_COMPONENT_COLOUR,
		TOGGLE_ACTION,
		SET_CLIP_AMMO,
		SET_NO_RELOAD,
		SET_BLUR_INTENSITY,
		TOGGLE_DRIVE_ON_WATER,
		SET_GAME_SPEED,
		TOGGLE_PLAYER_FROZEN,
		SET_PLAYER_ANIMS,
		TOGGLE_SWITCH_RELOAD,
		TOGGLE_INFINITE_RUN,
		SET_AIRCRAFT_HEIGHT,
		SET_JETPACK_HEIGHT,
		SET_CHECKPOINT_EX,
		SET_RACE_CHECKPOINT_EX,
		SET_CHECKPOINT_COLOUR,
		SET_RACE_CHECKPOINT_COLOUR,
		TOGGLE_VEHICLE_BLIPS,
		TOGGLE_INFINITE_OXYGEN,
		TOGGLE_WATER_BUOYANCY,
		TOGGLE_UNDERWATER_EFFECT,
		TOGGLE_NIGHTVISION,
		TOGGLE_THERMALVISION,
		SET_KEY_BIND,
		UNBIND_KEY,
		CLEAR_KEY_BINDS,

		ON_PAUSE_MENU_TOGGLE,
		ON_PAUSE_MENU_CHANGE,
		ON_DRIVE_BY_SHOT,
		ON_STUNT_BONUS,
		ON_RESOLUTION_CHANGE,
		ON_MOUSE_CLICK,
		ON_RADIO_CHANGE,
		ON_DRINK_SPRUNK,
		ON_KEY_STATE_CHANGE,

		SET_KEY_CAPTURE,
		CLEAR_KEY_CAPTURE,
		CLEAR_KEY_CAPTURES,

		TARGET_SET_CONTEXT,
		TARGET_CLEAR_CONTEXT,
		ON_TARGET_SELECT,
		ON_TARGET_MODE,

		BUILD_OPEN,
		BUILD_CLOSE,
		BUILD_CLEAR_PARTS,
		BUILD_ADD_PART,
		BUILD_RESULT,
		ON_BUILD_SELECT,
		ON_BUILD_PLACE,
		ON_BUILD_CANCEL,
		ON_BUILD_PREVIEW
	};
}
