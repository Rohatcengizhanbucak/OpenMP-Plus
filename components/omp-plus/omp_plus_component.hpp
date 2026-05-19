#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include <sdk.hpp>
#include <network.hpp>
#include <player.hpp>
#include <bitstream.hpp>
#include <Server/Components/Pawn/pawn.hpp>
#include <Server/Components/Vehicles/vehicles.hpp>

#include "omp_plus_protocol.hpp"

struct OMPPlusPlayerState
{
	bool ready = false;
	uint16_t protocolVersion = 0;
	uint32_t capabilities = 0;
	uint8_t clientInfoVersion = 0;
	uint16_t clientVersionMajor = 0;
	uint16_t clientVersionMinor = 0;
	uint16_t clientVersionPatch = 0;
	uint32_t featureFlags = 0;
	bool launcherVerified = false;
	std::string clientHash;
	uint16_t resolutionX = 0;
	uint16_t resolutionY = 0;
	uint8_t radio = 0;
	bool inPauseMenu = false;
	float aircraftHeight = 800.0f;
	float jetpackHeight = 100.0f;
	bool vehicleBlips = false;
	TimePoint rateWindow = TimePoint::min();
	int messagesInWindow = 0;
};

struct OMPPlusTargetOption
{
	uint32_t optionId = 0;
	uint8_t rowType = OMPPlusProtocol::TargetRowAction;
	bool enabled = true;
	std::string label;
	std::string icon;
};

struct OMPPlusTargetContext
{
	bool active = false;
	bool advanced = false;
	uint32_t targetId = 0;
	uint32_t flags = 0;
	uint8_t targetType = OMPPlusProtocol::TargetTypeGeneric;
	uint8_t layout = OMPPlusProtocol::TargetLayoutAuto;
	uint16_t ttlMs = 0;
	TimePoint expiresAt = TimePoint::min();
	std::string title;
	std::string description;
	std::vector<OMPPlusTargetOption> options;
};

struct OMPPlusBuildPart
{
	uint32_t partId = 0;
	int32_t modelId = 0;
	std::string name;
	std::string category;
	std::string cost;
};

struct OMPPlusBuildContext
{
	bool active = false;
	uint32_t sessionId = 0;
	float maxDistance = 8.0f;
	std::string title;
	std::vector<OMPPlusBuildPart> parts;
	TimePoint lastPlaceAt = TimePoint::min();
};

class OMPPlusComponent final
	: public IComponent
	, public PlayerConnectEventHandler
	, public PawnEventHandler
	, public SingleNetworkInEventHandler
{
public:
	PROVIDE_UID(0x6f6d70706c757331);

	static OMPPlusComponent* getInstance();
	~OMPPlusComponent();

	StringView componentName() const override;
	SemanticVersion componentVersion() const override;
	void onLoad(ICore* core) override;
	void onInit(IComponentList* components) override;
	void onReady() override;
	void onFree(IComponent* component) override;
	void free() override;
	void reset() override;

	void onPlayerConnect(IPlayer& player) override;
	void onPlayerDisconnect(IPlayer& player, PeerDisconnectReason reason) override;

	void onAmxLoad(IPawnScript& script) override;
	void onAmxUnload(IPawnScript& script) override;

	bool onReceive(IPlayer& player, NetworkBitStream& stream) override;

	bool isUsingOMPPlus(int playerid) const;
	bool sendLegacyRPC(int playerid, uint16_t rpc, NetworkBitStream* payload = nullptr);
	void broadcastLegacyRPC(uint16_t rpc, NetworkBitStream* payload = nullptr);

	OMPPlusPlayerState* getPlayerState(int playerid);
	IPawnScript* getScript(AMX* amx);
	std::string getPawnString(AMX* amx, cell address, size_t maxLength);
	bool setPawnString(AMX* amx, cell address, cell size, const std::string& value);
	bool setPawnCell(AMX* amx, cell address, cell value);
	bool beginTargetContext(int playerid, uint32_t targetid, const std::string& title, uint16_t ttlMs, uint32_t flags);
	bool beginTargetContextEx(int playerid, uint32_t targetid, uint8_t targetType, const std::string& title, uint16_t ttlMs, uint32_t flags);
	bool setTargetLayout(int playerid, uint32_t targetid, uint8_t layout);
	bool setTargetDescription(int playerid, uint32_t targetid, const std::string& description);
	bool addTargetOption(int playerid, uint32_t targetid, uint32_t optionid, const std::string& label, const std::string& icon, bool enabled);
	bool addTargetRow(int playerid, uint32_t targetid, uint32_t optionid, uint8_t rowType, const std::string& label, const std::string& icon, bool enabled);
	bool commitTargetContext(int playerid, uint32_t targetid);
	bool clearTargetContext(int playerid);
	bool openBuild(int playerid, uint32_t sessionid, const std::string& title, float maxDistance);
	bool closeBuild(int playerid);
	bool clearBuildParts(int playerid);
	bool addBuildPart(int playerid, uint32_t partid, int32_t modelid, const std::string& name, const std::string& category, const std::string& cost);
	bool sendBuildResult(int playerid, uint8_t result, const std::string& message);
	bool sendBuildRemoveTarget(int playerid, bool active, uint32_t partid, const std::string& label, float distance);
	bool openUi(int playerid, const std::string& documentid, uint8_t templateid, uint32_t flags, uint16_t capacity, const std::string& title, const std::string& body);
	bool closeUi(int playerid, const std::string& documentid);
	bool closeAllUi(int playerid);
	bool setUiData(int playerid, const std::string& documentid, const std::string& key, const std::string& value);
	bool clearInventory(int playerid, const std::string& documentid);
	bool setInventorySlot(int playerid, const std::string& documentid, uint16_t slot, uint32_t itemid, uint16_t amount, const std::string& label, const std::string& description, const std::string& icon);
	bool setInventorySlotActions(int playerid, const std::string& documentid, uint16_t slot, const std::string& actions);

	template <typename... Args>
	cell callPublic(const char* name, DefaultReturnValue defaultReturn, Args&&... args)
	{
		cell result = defaultReturn == DefaultReturnValue_True ? 1 : 0;
		for (IPawnScript* script : scripts_)
		{
			if (!script || !script->IsLoaded())
				continue;

			cell current = script->Call(name, defaultReturn, std::forward<Args>(args)...);
			if (current == 0)
				result = 0;
		}
		return result;
	}

	template <typename... Args>
	cell callPublicIfExists(const char* name, DefaultReturnValue defaultReturn, bool& called, Args&&... args)
	{
		cell result = defaultReturn == DefaultReturnValue_True ? 1 : 0;
		for (IPawnScript* script : scripts_)
		{
			if (!script || !script->IsLoaded())
				continue;

			int publicIndex = 0;
			if (script->FindPublic(name, &publicIndex) != AMX_ERR_NONE)
				continue;

			called = true;
			cell current = script->Call(name, defaultReturn, std::forward<Args>(args)...);
			if (current == 0)
				result = 0;
		}
		return result;
	}

private:
	static OMPPlusComponent* instance_;

	ICore* core_ = nullptr;
	IPawnComponent* pawn_ = nullptr;
	std::array<OMPPlusPlayerState, PLAYER_POOL_SIZE> states_;
	std::array<OMPPlusTargetContext, PLAYER_POOL_SIZE> targetContexts_;
	std::array<OMPPlusBuildContext, PLAYER_POOL_SIZE> buildContexts_;
	std::vector<IPawnScript*> scripts_;

	void resetPlayer(int playerid);
	bool readHeader(NetworkBitStream& stream, OMPPlusProtocol::Message& message, uint16_t& version);
	void writeHeader(NetworkBitStream& stream, OMPPlusProtocol::Message message);
	bool acceptRate(IPlayer& player);
	bool handleHello(IPlayer& player, NetworkBitStream& stream, uint16_t version);
	bool handleClientRPC(IPlayer& player, NetworkBitStream& stream);
	void processClientRPC(IPlayer& player, uint16_t rpc, NetworkBitStream& stream);
	void processTargetSelect(IPlayer& player, NetworkBitStream& stream);
	void processBuildSelect(IPlayer& player, NetworkBitStream& stream);
	void processBuildPlace(IPlayer& player, NetworkBitStream& stream);
	void processBuildCancel(IPlayer& player, NetworkBitStream& stream);
	void processBuildPreview(IPlayer& player, NetworkBitStream& stream);
	void processUiEvent(IPlayer& player, NetworkBitStream& stream);
	uint32_t deriveLegacyFeatures(uint32_t capabilities) const;
	void sendHelloAck(IPlayer& player);
	void sendError(IPlayer& player, uint16_t code);
	IPlayer* getPlayer(int playerid) const;
};
