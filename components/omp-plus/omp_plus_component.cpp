#include "omp_plus_component.hpp"

#include <cstring>

OMPPlusComponent* OMPPlusComponent::instance_ = nullptr;

namespace
{
	constexpr size_t MaxTargetOptions = 8;
	constexpr size_t MaxTargetRows = 12;
	constexpr size_t MaxTargetTitleLength = 48;
	constexpr size_t MaxTargetLabelLength = 96;
	constexpr size_t MaxTargetIconLength = 24;
	constexpr size_t MaxTargetDescriptionLength = 128;
	constexpr size_t MaxBuildParts = 32;
	constexpr size_t MaxBuildTitleLength = 48;
	constexpr size_t MaxBuildNameLength = 48;
	constexpr size_t MaxBuildCategoryLength = 32;
	constexpr size_t MaxBuildCostLength = 48;
	constexpr size_t MaxBuildMessageLength = 96;
	constexpr uint16_t DefaultTargetTtlMs = 500;
	constexpr uint16_t MaxTargetTtlMs = 5000;
	constexpr float DefaultBuildMaxDistance = 8.0f;
	constexpr float MaxBuildMaxDistance = 30.0f;
	constexpr int BuildPlaceCooldownMs = 250;

	std::string BoundString(const std::string& value, size_t maxLength)
	{
		return value.length() > maxLength ? value.substr(0, maxLength) : value;
	}

	void WriteBoundString(NetworkBitStream& stream, const std::string& value, size_t maxLength)
	{
		std::string bounded = BoundString(value, maxLength);
		stream.Write(static_cast<uint8_t>(bounded.length()));
		if (!bounded.empty())
			stream.Write(bounded.c_str(), static_cast<int>(bounded.length()));
	}

	bool IsValidTargetType(uint8_t targetType)
	{
		return targetType <= OMPPlusProtocol::TargetTypeCustom;
	}

	bool IsValidTargetLayout(uint8_t layout)
	{
		return layout <= OMPPlusProtocol::TargetLayoutCategory;
	}

	bool IsValidTargetRowType(uint8_t rowType)
	{
		return rowType <= OMPPlusProtocol::TargetRowDanger;
	}

	bool IsSelectableTargetRow(uint8_t rowType)
	{
		return rowType == OMPPlusProtocol::TargetRowAction
			|| rowType == OMPPlusProtocol::TargetRowToggle
			|| rowType == OMPPlusProtocol::TargetRowDanger;
	}
}

OMPPlusComponent* OMPPlusComponent::getInstance()
{
	if (!instance_)
		instance_ = new OMPPlusComponent();
	return instance_;
}

StringView OMPPlusComponent::componentName() const
{
	return "OpenMP-Plus";
}

SemanticVersion OMPPlusComponent::componentVersion() const
{
	return SemanticVersion(0, 2, 0, 0);
}

void OMPPlusComponent::onLoad(ICore* core)
{
	core_ = core;
	core_->getPlayers().getPlayerConnectDispatcher().addEventHandler(this);
	core_->addPerRPCInEventHandler<OMPPlusProtocol::RpcID>(this);
	core_->printLn("[OpenMP-Plus] Native INetwork transport loaded on RPC %d.", OMPPlusProtocol::RpcID);
}

void OMPPlusComponent::onInit(IComponentList* components)
{
	pawn_ = components->queryComponent<IPawnComponent>();
	if (pawn_)
		pawn_->getEventDispatcher().addEventHandler(this);
	else if (core_)
		core_->logLn(Warning, "[OpenMP-Plus] Pawn component not found; natives will not be registered.");
}

void OMPPlusComponent::onReady()
{
}

void OMPPlusComponent::onFree(IComponent* component)
{
	if (component == pawn_)
	{
		pawn_ = nullptr;
		scripts_.clear();
	}
}

void OMPPlusComponent::free()
{
	delete this;
}

void OMPPlusComponent::reset()
{
	for (auto& state : states_)
		state = OMPPlusPlayerState();
	for (auto& context : targetContexts_)
		context = OMPPlusTargetContext();
	for (auto& context : buildContexts_)
		context = OMPPlusBuildContext();
}

void OMPPlusComponent::onPlayerConnect(IPlayer& player)
{
	resetPlayer(player.getID());
}

void OMPPlusComponent::onPlayerDisconnect(IPlayer& player, PeerDisconnectReason)
{
	resetPlayer(player.getID());
}

void OMPPlusComponent::onAmxLoad(IPawnScript& script)
{
	extern AMX_NATIVE_INFO OMPPlusNatives[];
	script.Register(OMPPlusNatives, -1);
	scripts_.push_back(&script);
}

void OMPPlusComponent::onAmxUnload(IPawnScript& script)
{
	scripts_.erase(std::remove(scripts_.begin(), scripts_.end(), &script), scripts_.end());
}

bool OMPPlusComponent::onReceive(IPlayer& player, NetworkBitStream& stream)
{
	if (!acceptRate(player))
		return false;

	OMPPlusProtocol::Message message;
	uint16_t version;
	if (!readHeader(stream, message, version))
	{
		sendError(player, 1);
		return false;
	}

	if (stream.GetNumberOfUnreadBits() > OMPPlusProtocol::MaxPayloadBytes * 8)
	{
		sendError(player, 2);
		return false;
	}

	switch (message)
	{
	case OMPPlusProtocol::Message::Hello:
		return handleHello(player, stream, version);
	case OMPPlusProtocol::Message::ClientRPC:
		return handleClientRPC(player, stream);
	default:
		sendError(player, 3);
		return false;
	}
}

bool OMPPlusComponent::isUsingOMPPlus(int playerid) const
{
	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE)
		return false;
	return states_[playerid].ready;
}

bool OMPPlusComponent::sendLegacyRPC(int playerid, uint16_t rpc, NetworkBitStream* payload)
{
	IPlayer* player = getPlayer(playerid);
	if (!player || !isUsingOMPPlus(playerid))
		return false;

	NetworkBitStream stream;
	writeHeader(stream, OMPPlusProtocol::Message::ServerRPC);
	stream.Write(rpc);
	if (payload && payload->GetNumberOfBytesUsed() > 0)
		stream.Write(reinterpret_cast<const char*>(payload->GetData()), payload->GetNumberOfBytesUsed());

	return player->sendRPC(OMPPlusProtocol::RpcID, Span<uint8_t>(stream.GetData(), stream.GetNumberOfBitsUsed()), OMPPlusProtocol::Channel);
}

void OMPPlusComponent::broadcastLegacyRPC(uint16_t rpc, NetworkBitStream* payload)
{
	for (IPlayer* player : core_->getPlayers().players())
		sendLegacyRPC(player->getID(), rpc, payload);
}

OMPPlusPlayerState* OMPPlusComponent::getPlayerState(int playerid)
{
	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE)
		return nullptr;
	return &states_[playerid];
}

IPawnScript* OMPPlusComponent::getScript(AMX* amx)
{
	return pawn_ ? pawn_->getScript(amx) : nullptr;
}

std::string OMPPlusComponent::getPawnString(AMX* amx, cell address, size_t maxLength)
{
	IPawnScript* script = getScript(amx);
	if (!script)
		return std::string();

	cell* physical = nullptr;
	if (script->GetAddr(address, &physical) != AMX_ERR_NONE || !physical)
		return std::string();

	int length = 0;
	if (script->StrLen(physical, &length) != AMX_ERR_NONE || length <= 0)
		return std::string();

	length = std::min<int>(length, static_cast<int>(maxLength));
	std::string value(static_cast<size_t>(length) + 1, '\0');
	if (script->GetString(&value[0], physical, false, static_cast<size_t>(length) + 1) != AMX_ERR_NONE)
		return std::string();

	value.resize(std::strlen(value.c_str()));
	return value;
}

bool OMPPlusComponent::setPawnCell(AMX* amx, cell address, cell value)
{
	IPawnScript* script = getScript(amx);
	if (!script)
		return false;

	cell* physical = nullptr;
	if (script->GetAddr(address, &physical) != AMX_ERR_NONE || !physical)
		return false;

	*physical = value;
	return true;
}

bool OMPPlusComponent::setPawnString(AMX* amx, cell address, cell size, const std::string& value)
{
	IPawnScript* script = getScript(amx);
	if (!script || size <= 0)
		return false;

	cell* physical = nullptr;
	if (script->GetAddr(address, &physical) != AMX_ERR_NONE || !physical)
		return false;

	return script->SetString(physical, StringView(value.c_str(), value.length()), false, false, static_cast<size_t>(size)) == AMX_ERR_NONE;
}

bool OMPPlusComponent::beginTargetContext(int playerid, uint32_t targetid, const std::string& title, uint16_t ttlMs, uint32_t flags)
{
	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE || !isUsingOMPPlus(playerid) || targetid == 0)
		return false;

	if (ttlMs == 0)
		ttlMs = DefaultTargetTtlMs;
	ttlMs = std::min<uint16_t>(ttlMs, MaxTargetTtlMs);

	OMPPlusTargetContext& context = targetContexts_[playerid];
	context.active = true;
	context.advanced = false;
	context.targetId = targetid;
	context.flags = flags;
	context.targetType = OMPPlusProtocol::TargetTypeGeneric;
	context.layout = OMPPlusProtocol::TargetLayoutAuto;
	context.ttlMs = ttlMs;
	context.expiresAt = Time::now() + Milliseconds(ttlMs);
	context.title = BoundString(title, MaxTargetTitleLength);
	context.description.clear();
	context.options.clear();
	return true;
}

bool OMPPlusComponent::beginTargetContextEx(int playerid, uint32_t targetid, uint8_t targetType, const std::string& title, uint16_t ttlMs, uint32_t flags)
{
	if (!IsValidTargetType(targetType))
		targetType = OMPPlusProtocol::TargetTypeGeneric;

	if (!beginTargetContext(playerid, targetid, title, ttlMs, flags))
		return false;

	OMPPlusTargetContext& context = targetContexts_[playerid];
	context.advanced = true;
	context.targetType = targetType;
	return true;
}

bool OMPPlusComponent::setTargetLayout(int playerid, uint32_t targetid, uint8_t layout)
{
	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE || targetid == 0)
		return false;

	OMPPlusTargetContext& context = targetContexts_[playerid];
	if (!context.active || context.targetId != targetid)
		return false;

	if (!IsValidTargetLayout(layout))
		layout = OMPPlusProtocol::TargetLayoutAuto;

	context.advanced = true;
	context.layout = layout;
	return true;
}

bool OMPPlusComponent::setTargetDescription(int playerid, uint32_t targetid, const std::string& description)
{
	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE || targetid == 0)
		return false;

	OMPPlusTargetContext& context = targetContexts_[playerid];
	if (!context.active || context.targetId != targetid)
		return false;

	context.advanced = true;
	context.description = BoundString(description, MaxTargetDescriptionLength);
	return true;
}

bool OMPPlusComponent::addTargetOption(int playerid, uint32_t targetid, uint32_t optionid, const std::string& label, const std::string& icon, bool enabled)
{
	if (optionid == 0)
		return false;
	return addTargetRow(playerid, targetid, optionid, enabled ? OMPPlusProtocol::TargetRowAction : OMPPlusProtocol::TargetRowDisabled, label, icon, enabled);
}

bool OMPPlusComponent::addTargetRow(int playerid, uint32_t targetid, uint32_t optionid, uint8_t rowType, const std::string& label, const std::string& icon, bool enabled)
{
	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE || targetid == 0)
		return false;

	OMPPlusTargetContext& context = targetContexts_[playerid];
	if (!context.active || context.targetId != targetid || context.options.size() >= MaxTargetRows)
		return false;

	if (!IsValidTargetRowType(rowType))
		rowType = OMPPlusProtocol::TargetRowAction;

	if (IsSelectableTargetRow(rowType) && optionid == 0)
		return false;

	OMPPlusTargetOption option;
	option.optionId = optionid;
	option.rowType = rowType;
	option.enabled = enabled;
	option.label = BoundString(label, MaxTargetLabelLength);
	option.icon = BoundString(icon, MaxTargetIconLength);
	context.options.push_back(option);
	return true;
}

bool OMPPlusComponent::commitTargetContext(int playerid, uint32_t targetid)
{
	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE || !isUsingOMPPlus(playerid))
		return false;

	OMPPlusTargetContext& context = targetContexts_[playerid];
	if (!context.active || context.targetId != targetid || context.options.empty())
		return false;

	context.expiresAt = Time::now() + Milliseconds(context.ttlMs);
	OMPPlusPlayerState* state = getPlayerState(playerid);
	const bool useV2 = context.advanced && state && (state->capabilities & OMPPlusProtocol::CapabilityTargetUIV2);

	NetworkBitStream stream;
	stream.Write(context.targetId);
	stream.Write(context.ttlMs);
	stream.Write(useV2 ? (context.flags | OMPPlusProtocol::TargetFlagPayloadV2) : (context.flags & ~OMPPlusProtocol::TargetFlagPayloadV2));

	if (useV2)
	{
		stream.Write(context.targetType);
		stream.Write(context.layout);
		WriteBoundString(stream, context.title, MaxTargetTitleLength);
		WriteBoundString(stream, context.description, MaxTargetDescriptionLength);
		stream.Write(static_cast<uint8_t>(context.options.size()));

		for (const OMPPlusTargetOption& option : context.options)
		{
			stream.Write(option.optionId);
			stream.Write(option.rowType);
			stream.Write(option.enabled);
			WriteBoundString(stream, option.label, MaxTargetLabelLength);
			WriteBoundString(stream, option.icon, MaxTargetIconLength);
		}

		return sendLegacyRPC(playerid, OMPPlusProtocol::TARGET_SET_CONTEXT, &stream);
	}

	WriteBoundString(stream, context.title, MaxTargetTitleLength);

	std::vector<OMPPlusTargetOption> legacyOptions;
	for (const OMPPlusTargetOption& option : context.options)
	{
		if (legacyOptions.size() >= MaxTargetOptions)
			break;
		if (option.rowType == OMPPlusProtocol::TargetRowDivider)
			continue;
		if (option.optionId == 0)
			continue;
		legacyOptions.push_back(option);
	}

	if (legacyOptions.empty())
		return false;

	stream.Write(static_cast<uint8_t>(legacyOptions.size()));
	for (const OMPPlusTargetOption& option : legacyOptions)
	{
		stream.Write(option.optionId);
		stream.Write(option.enabled && IsSelectableTargetRow(option.rowType));
		WriteBoundString(stream, option.label, 48);
		WriteBoundString(stream, option.icon, MaxTargetIconLength);
	}

	return sendLegacyRPC(playerid, OMPPlusProtocol::TARGET_SET_CONTEXT, &stream);
}

bool OMPPlusComponent::clearTargetContext(int playerid)
{
	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE)
		return false;

	targetContexts_[playerid] = OMPPlusTargetContext();
	return sendLegacyRPC(playerid, OMPPlusProtocol::TARGET_CLEAR_CONTEXT) != 0;
}

bool OMPPlusComponent::openBuild(int playerid, uint32_t sessionid, const std::string& title, float maxDistance)
{
	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE || !isUsingOMPPlus(playerid) || sessionid == 0)
		return false;

	if (maxDistance <= 0.0f)
		maxDistance = DefaultBuildMaxDistance;
	maxDistance = std::min<float>(maxDistance, MaxBuildMaxDistance);

	OMPPlusBuildContext& context = buildContexts_[playerid];
	context.active = true;
	context.sessionId = sessionid;
	context.maxDistance = maxDistance;
	context.title = BoundString(title, MaxBuildTitleLength);
	context.lastPlaceAt = TimePoint::min();

	NetworkBitStream stream;
	stream.Write(context.sessionId);
	stream.Write(context.maxDistance);
	WriteBoundString(stream, context.title, MaxBuildTitleLength);
	return sendLegacyRPC(playerid, OMPPlusProtocol::BUILD_OPEN, &stream);
}

bool OMPPlusComponent::closeBuild(int playerid)
{
	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE)
		return false;

	buildContexts_[playerid] = OMPPlusBuildContext();
	return sendLegacyRPC(playerid, OMPPlusProtocol::BUILD_CLOSE) != 0;
}

bool OMPPlusComponent::clearBuildParts(int playerid)
{
	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE || !isUsingOMPPlus(playerid))
		return false;

	OMPPlusBuildContext& context = buildContexts_[playerid];
	if (!context.active)
		return false;

	context.parts.clear();
	return sendLegacyRPC(playerid, OMPPlusProtocol::BUILD_CLEAR_PARTS) != 0;
}

bool OMPPlusComponent::addBuildPart(int playerid, uint32_t partid, int32_t modelid, const std::string& name, const std::string& category, const std::string& cost)
{
	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE || !isUsingOMPPlus(playerid) || partid == 0)
		return false;

	OMPPlusBuildContext& context = buildContexts_[playerid];
	if (!context.active || context.parts.size() >= MaxBuildParts)
		return false;

	const auto duplicate = std::find_if(context.parts.begin(), context.parts.end(), [partid](const OMPPlusBuildPart& part)
	{
		return part.partId == partid;
	});
	if (duplicate != context.parts.end())
		return false;

	OMPPlusBuildPart part;
	part.partId = partid;
	part.modelId = modelid;
	part.name = BoundString(name, MaxBuildNameLength);
	part.category = BoundString(category, MaxBuildCategoryLength);
	part.cost = BoundString(cost, MaxBuildCostLength);
	context.parts.push_back(part);

	NetworkBitStream stream;
	stream.Write(part.partId);
	stream.Write(part.modelId);
	WriteBoundString(stream, part.name, MaxBuildNameLength);
	WriteBoundString(stream, part.category, MaxBuildCategoryLength);
	WriteBoundString(stream, part.cost, MaxBuildCostLength);
	return sendLegacyRPC(playerid, OMPPlusProtocol::BUILD_ADD_PART, &stream);
}

bool OMPPlusComponent::sendBuildResult(int playerid, uint8_t result, const std::string& message)
{
	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE || !isUsingOMPPlus(playerid))
		return false;

	NetworkBitStream stream;
	stream.Write(result);
	WriteBoundString(stream, message, MaxBuildMessageLength);
	return sendLegacyRPC(playerid, OMPPlusProtocol::BUILD_RESULT, &stream);
}

void OMPPlusComponent::resetPlayer(int playerid)
{
	if (playerid >= 0 && playerid < PLAYER_POOL_SIZE)
	{
		states_[playerid] = OMPPlusPlayerState();
		targetContexts_[playerid] = OMPPlusTargetContext();
		buildContexts_[playerid] = OMPPlusBuildContext();
	}
}

bool OMPPlusComponent::readHeader(NetworkBitStream& stream, OMPPlusProtocol::Message& message, uint16_t& version)
{
	const int start = stream.GetReadOffset();
	const int totalBits = stream.GetNumberOfBitsUsed();

	auto readAtOffset = [&](int bitOffset) -> bool
	{
		if (bitOffset < 0 || bitOffset + 56 > totalBits)
			return false;

		stream.SetReadOffset(bitOffset);
		uint32_t magic;
		uint8_t type;
		if (!stream.Read(magic) || magic != OMPPlusProtocol::Magic)
			return false;
		if (!stream.Read(version) || version != OMPPlusProtocol::Version)
			return false;
		if (!stream.Read(type))
			return false;
		message = static_cast<OMPPlusProtocol::Message>(type);
		return true;
	};

	for (int byteOffset = 0; byteOffset <= 8; ++byteOffset)
	{
		if (readAtOffset(start + byteOffset * 8))
			return true;
	}

	stream.SetReadOffset(start);
	return false;
}

void OMPPlusComponent::writeHeader(NetworkBitStream& stream, OMPPlusProtocol::Message message)
{
	stream.Write(OMPPlusProtocol::Magic);
	stream.Write(OMPPlusProtocol::Version);
	stream.Write(static_cast<uint8_t>(message));
}

bool OMPPlusComponent::acceptRate(IPlayer& player)
{
	OMPPlusPlayerState* state = getPlayerState(player.getID());
	if (!state)
		return false;

	TimePoint now = Time::now();
	if (state->rateWindow == TimePoint::min() || now - state->rateWindow >= Seconds(1))
	{
		state->rateWindow = now;
		state->messagesInWindow = 0;
	}

	if (++state->messagesInWindow > OMPPlusProtocol::MaxMessagesPerSecond)
		return false;

	return true;
}

bool OMPPlusComponent::handleHello(IPlayer& player, NetworkBitStream& stream, uint16_t version)
{
	uint32_t capabilities = 0;
	if (!stream.Read(capabilities))
	{
		sendError(player, 4);
		return false;
	}

	OMPPlusPlayerState* state = getPlayerState(player.getID());
	if (!state)
		return false;

	const bool wasReady = state->ready;
	state->ready = true;
	state->protocolVersion = version;
	state->capabilities = capabilities;
	state->clientInfoVersion = 0;
	state->clientVersionMajor = 0;
	state->clientVersionMinor = 0;
	state->clientVersionPatch = 0;
	state->featureFlags = deriveLegacyFeatures(capabilities);
	state->launcherVerified = false;
	state->clientHash.clear();

	if (stream.GetNumberOfUnreadBits() >= 8)
	{
		uint8_t infoVersion = 0;
		if (stream.Read(infoVersion))
		{
			state->clientInfoVersion = infoVersion;

			if (infoVersion >= 1 && stream.GetNumberOfUnreadBits() >= 16 * 3 + 32 + 1 + 8)
			{
				uint16_t major = 0;
				uint16_t minor = 0;
				uint16_t patch = 0;
				uint32_t featureFlags = 0;
				bool launcherVerified = false;
				uint8_t hashLength = 0;

				if (stream.Read(major)
					&& stream.Read(minor)
					&& stream.Read(patch)
					&& stream.Read(featureFlags)
					&& stream.Read(launcherVerified)
					&& stream.Read(hashLength)
					&& hashLength <= OMPPlusProtocol::MaxClientHashLength
					&& stream.GetNumberOfUnreadBits() >= static_cast<int>(hashLength) * 8)
				{
					char hash[OMPPlusProtocol::MaxClientHashLength + 1] = {};
					if (!hashLength || stream.Read(hash, hashLength))
					{
						state->clientVersionMajor = major;
						state->clientVersionMinor = minor;
						state->clientVersionPatch = patch;
						state->featureFlags = featureFlags;
						state->launcherVerified = launcherVerified;
						state->clientHash.assign(hash, hashLength);
					}
				}
			}
		}
	}

	if (core_)
	{
		core_->printLn(
			"[OpenMP-Plus] Native HELLO from player %d, protocol=%u, client=%u.%u.%u, features=%u, verified=%s, hash=%.*s.",
			player.getID(),
			static_cast<unsigned>(version),
			static_cast<unsigned>(state->clientVersionMajor),
			static_cast<unsigned>(state->clientVersionMinor),
			static_cast<unsigned>(state->clientVersionPatch),
			static_cast<unsigned>(state->featureFlags),
			state->launcherVerified ? "true" : "false",
			static_cast<int>(std::min<size_t>(12, state->clientHash.length())),
			state->clientHash.c_str());
	}

	sendHelloAck(player);

	if (!wasReady)
	{
		callPublic("OnPlayerOMPPlusReady", DefaultReturnValue_True, player.getID());
		callPublic("OnPlayerSAMPPJoin", DefaultReturnValue_True, player.getID(), 1);
	}

	return false;
}

uint32_t OMPPlusComponent::deriveLegacyFeatures(uint32_t capabilities) const
{
	uint32_t features = 0;
	if (capabilities & OMPPlusProtocol::CapabilityNativeTransport)
		features |= OMPPlusProtocol::FeatureHUD | OMPPlusProtocol::FeatureKeybind;
	if (capabilities & OMPPlusProtocol::CapabilityKeyCapture)
		features |= OMPPlusProtocol::FeatureKeyCapture;
	if (capabilities & (OMPPlusProtocol::CapabilityTargetUI | OMPPlusProtocol::CapabilityTargetUIV2))
		features |= OMPPlusProtocol::FeatureTarget | OMPPlusProtocol::FeatureUI;
	if (capabilities & OMPPlusProtocol::CapabilityBuildUI)
		features |= OMPPlusProtocol::FeatureBuild | OMPPlusProtocol::FeatureUI;
	return features;
}

bool OMPPlusComponent::handleClientRPC(IPlayer& player, NetworkBitStream& stream)
{
	if (!isUsingOMPPlus(player.getID()))
		return false;

	uint16_t rpc = 0;
	if (!stream.Read(rpc))
		return false;

	processClientRPC(player, rpc, stream);
	return false;
}

void OMPPlusComponent::processClientRPC(IPlayer& player, uint16_t rpc, NetworkBitStream& stream)
{
	const int playerid = player.getID();
	OMPPlusPlayerState* state = getPlayerState(playerid);
	if (!state)
		return;

	switch (rpc)
	{
	case OMPPlusProtocol::ON_PAUSE_MENU_TOGGLE:
	{
		bool opened;
		if (stream.Read(opened))
		{
			state->inPauseMenu = opened;
			callPublic(opened ? "OnPlayerOpenPauseMenu" : "OnPlayerClosePauseMenu", DefaultReturnValue_True, playerid);
		}
		break;
	}
	case OMPPlusProtocol::ON_PAUSE_MENU_CHANGE:
	{
		uint8_t from;
		uint8_t to;
		if (stream.Read(from) && stream.Read(to))
			callPublic("OnPlayerEnterPauseSubmenu", DefaultReturnValue_True, playerid, static_cast<int>(from), static_cast<int>(to));
		break;
	}
	case OMPPlusProtocol::ON_DRIVE_BY_SHOT:
		callPublic("OnDriverDriveByShot", DefaultReturnValue_True, playerid);
		break;
	case OMPPlusProtocol::ON_STUNT_BONUS:
	{
		uint32_t money;
		int detailsRaw[6];
		uint8_t type;
		if (stream.Read(money) && stream.Read(reinterpret_cast<char*>(detailsRaw), sizeof(detailsRaw)) && stream.Read(type))
		{
			StaticArray<int, 6> details;
			std::copy(detailsRaw, detailsRaw + 6, details.begin());
			callPublic("OnPlayerStunt", DefaultReturnValue_True, playerid, static_cast<int>(type), static_cast<int>(money), details);
		}
		break;
	}
	case OMPPlusProtocol::ON_RESOLUTION_CHANGE:
	{
		uint16_t x;
		uint16_t y;
		if (stream.Read(x) && stream.Read(y))
		{
			state->resolutionX = x;
			state->resolutionY = y;
			callPublic("OnPlayerResolutionChange", DefaultReturnValue_True, playerid, static_cast<int>(x), static_cast<int>(y));
		}
		break;
	}
	case OMPPlusProtocol::ON_MOUSE_CLICK:
	{
		uint8_t type;
		uint8_t x;
		uint8_t y;
		if (stream.Read(type) && stream.Read(x) && stream.Read(y))
			callPublic("OnPlayerClick", DefaultReturnValue_True, playerid, static_cast<int>(type), static_cast<int>(x), static_cast<int>(y));
		break;
	}
	case OMPPlusProtocol::ON_RADIO_CHANGE:
	{
		uint8_t radio;
		if (stream.Read(radio))
		{
			state->radio = radio;
			int vehicleid = 0;
			if (IPlayerVehicleData* vehicleData = queryExtension<IPlayerVehicleData>(player))
			{
				if (IVehicle* vehicle = vehicleData->getVehicle())
					vehicleid = vehicle->getID();
			}
			callPublic("OnPlayerChangeRadioStation", DefaultReturnValue_True, playerid, static_cast<int>(radio), vehicleid);
		}
		break;
	}
	case OMPPlusProtocol::ON_DRINK_SPRUNK:
		callPublic("OnPlayerDrinkSprunk", DefaultReturnValue_True, playerid);
		break;
	case OMPPlusProtocol::ON_KEY_STATE_CHANGE:
	{
		uint16_t key;
		uint8_t keyState;
		uint8_t actionLength;
		if (!stream.Read(key) || !stream.Read(keyState) || !stream.Read(actionLength) || actionLength > 31)
			return;

		char action[32] = {};
		if (actionLength && !stream.Read(action, actionLength))
			return;

		callPublic("OnPlayerSAMPPKey", DefaultReturnValue_True, playerid, static_cast<int>(key), static_cast<int>(keyState), StringView(action, actionLength));
		callPublic("OnPlayerOMPPlusKey", DefaultReturnValue_True, playerid, static_cast<int>(key), static_cast<int>(keyState), StringView(action, actionLength));
		break;
	}
	case OMPPlusProtocol::ON_TARGET_SELECT:
		processTargetSelect(player, stream);
		break;
	case OMPPlusProtocol::ON_TARGET_MODE:
	{
		uint32_t targetid = 0;
		bool opened = false;
		if (stream.Read(targetid) && stream.Read(opened))
		{
			callPublic("OnPlayerSAMPPTargetMode", DefaultReturnValue_True, playerid, static_cast<int>(targetid), opened ? 1 : 0);
			callPublic("OnPlayerOMPPlusTargetMode", DefaultReturnValue_True, playerid, static_cast<int>(targetid), opened ? 1 : 0);
		}
		break;
	}
	case OMPPlusProtocol::ON_BUILD_SELECT:
		processBuildSelect(player, stream);
		break;
	case OMPPlusProtocol::ON_BUILD_PLACE:
		processBuildPlace(player, stream);
		break;
	case OMPPlusProtocol::ON_BUILD_CANCEL:
		processBuildCancel(player, stream);
		break;
	case OMPPlusProtocol::ON_BUILD_PREVIEW:
		processBuildPreview(player, stream);
		break;
	default:
		break;
	}
}

void OMPPlusComponent::processTargetSelect(IPlayer& player, NetworkBitStream& stream)
{
	const int playerid = player.getID();
	uint32_t targetid = 0;
	uint32_t optionid = 0;
	if (!stream.Read(targetid) || !stream.Read(optionid))
		return;

	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE)
		return;

	OMPPlusTargetContext& context = targetContexts_[playerid];
	if (!context.active || context.targetId != targetid || Time::now() > context.expiresAt)
		return;

	const auto optionIt = std::find_if(context.options.begin(), context.options.end(), [optionid](const OMPPlusTargetOption& option)
	{
		return option.optionId == optionid && option.enabled && IsSelectableTargetRow(option.rowType);
	});
	if (optionIt == context.options.end())
		return;

	targetContexts_[playerid] = OMPPlusTargetContext();
	sendLegacyRPC(playerid, OMPPlusProtocol::TARGET_CLEAR_CONTEXT);

	callPublic("OnPlayerSAMPPTargetSelect", DefaultReturnValue_True, playerid, static_cast<int>(targetid), static_cast<int>(optionid));
	callPublic("OnPlayerOMPPlusTargetSelect", DefaultReturnValue_True, playerid, static_cast<int>(targetid), static_cast<int>(optionid));
}

void OMPPlusComponent::processBuildSelect(IPlayer& player, NetworkBitStream& stream)
{
	const int playerid = player.getID();
	uint32_t sessionid = 0;
	uint32_t partid = 0;
	if (!stream.Read(sessionid) || !stream.Read(partid))
		return;

	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE)
		return;

	OMPPlusBuildContext& context = buildContexts_[playerid];
	if (!context.active || context.sessionId != sessionid)
		return;

	if (partid == 0)
	{
		callPublic("OnPlayerSAMPPBuildPreview", DefaultReturnValue_True, playerid, static_cast<int>(sessionid), 0, 0, 0);
		callPublic("OnPlayerOMPPlusBuildPreview", DefaultReturnValue_True, playerid, static_cast<int>(sessionid), 0, 0, 0);
		return;
	}

	const auto partIt = std::find_if(context.parts.begin(), context.parts.end(), [partid](const OMPPlusBuildPart& part)
	{
		return part.partId == partid;
	});
	if (partIt == context.parts.end())
		return;

	callPublic("OnPlayerSAMPPBuildSelect", DefaultReturnValue_True, playerid, static_cast<int>(sessionid), static_cast<int>(partid));
	callPublic("OnPlayerOMPPlusBuildSelect", DefaultReturnValue_True, playerid, static_cast<int>(sessionid), static_cast<int>(partid));
}

void OMPPlusComponent::processBuildPlace(IPlayer& player, NetworkBitStream& stream)
{
	const int playerid = player.getID();
	uint32_t sessionid = 0;
	uint32_t partid = 0;
	int16_t rotationStep = 0;
	bool flipped = false;
	if (!stream.Read(sessionid) || !stream.Read(partid) || !stream.Read(rotationStep) || !stream.Read(flipped))
		return;

	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE)
		return;

	OMPPlusBuildContext& context = buildContexts_[playerid];
	if (!context.active || context.sessionId != sessionid)
		return;

	TimePoint now = Time::now();
	if (context.lastPlaceAt != TimePoint::min() && now - context.lastPlaceAt < Milliseconds(BuildPlaceCooldownMs))
		return;

	const auto partIt = std::find_if(context.parts.begin(), context.parts.end(), [partid](const OMPPlusBuildPart& part)
	{
		return part.partId == partid;
	});
	if (partIt == context.parts.end())
		return;

	context.lastPlaceAt = now;
	callPublic("OnPlayerSAMPPBuildPlace", DefaultReturnValue_True, playerid, static_cast<int>(sessionid), static_cast<int>(partid), static_cast<int>(rotationStep), flipped ? 1 : 0);
	callPublic("OnPlayerOMPPlusBuildPlace", DefaultReturnValue_True, playerid, static_cast<int>(sessionid), static_cast<int>(partid), static_cast<int>(rotationStep), flipped ? 1 : 0);
}

void OMPPlusComponent::processBuildCancel(IPlayer& player, NetworkBitStream& stream)
{
	const int playerid = player.getID();
	uint32_t sessionid = 0;
	if (!stream.Read(sessionid))
		return;

	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE)
		return;

	OMPPlusBuildContext& context = buildContexts_[playerid];
	if (!context.active || context.sessionId != sessionid)
		return;

	callPublic("OnPlayerSAMPPBuildCancel", DefaultReturnValue_True, playerid, static_cast<int>(sessionid));
	callPublic("OnPlayerOMPPlusBuildCancel", DefaultReturnValue_True, playerid, static_cast<int>(sessionid));
	closeBuild(playerid);
}

void OMPPlusComponent::processBuildPreview(IPlayer& player, NetworkBitStream& stream)
{
	const int playerid = player.getID();
	uint32_t sessionid = 0;
	uint32_t partid = 0;
	int16_t rotationStep = 0;
	bool flipped = false;
	if (!stream.Read(sessionid) || !stream.Read(partid) || !stream.Read(rotationStep) || !stream.Read(flipped))
		return;

	if (playerid < 0 || playerid >= PLAYER_POOL_SIZE)
		return;

	OMPPlusBuildContext& context = buildContexts_[playerid];
	if (!context.active || context.sessionId != sessionid)
		return;

	if (partid == 0)
	{
		callPublic("OnPlayerSAMPPBuildPreview", DefaultReturnValue_True, playerid, static_cast<int>(sessionid), 0, 0, 0);
		callPublic("OnPlayerOMPPlusBuildPreview", DefaultReturnValue_True, playerid, static_cast<int>(sessionid), 0, 0, 0);
		return;
	}

	const auto partIt = std::find_if(context.parts.begin(), context.parts.end(), [partid](const OMPPlusBuildPart& part)
	{
		return part.partId == partid;
	});
	if (partIt == context.parts.end())
		return;

	callPublic("OnPlayerSAMPPBuildPreview", DefaultReturnValue_True, playerid, static_cast<int>(sessionid), static_cast<int>(partid), static_cast<int>(rotationStep), flipped ? 1 : 0);
	callPublic("OnPlayerOMPPlusBuildPreview", DefaultReturnValue_True, playerid, static_cast<int>(sessionid), static_cast<int>(partid), static_cast<int>(rotationStep), flipped ? 1 : 0);
}

void OMPPlusComponent::sendHelloAck(IPlayer& player)
{
	NetworkBitStream stream;
	writeHeader(stream, OMPPlusProtocol::Message::HelloAck);
	stream.Write(OMPPlusProtocol::DefaultCapabilities | OMPPlusProtocol::CapabilityTargetUI | OMPPlusProtocol::CapabilityTargetUIV2 | OMPPlusProtocol::CapabilityBuildUI);
	player.sendRPC(OMPPlusProtocol::RpcID, Span<uint8_t>(stream.GetData(), stream.GetNumberOfBitsUsed()), OMPPlusProtocol::Channel);
}

void OMPPlusComponent::sendError(IPlayer& player, uint16_t code)
{
	NetworkBitStream stream;
	writeHeader(stream, OMPPlusProtocol::Message::Error);
	stream.Write(code);
	player.sendRPC(OMPPlusProtocol::RpcID, Span<uint8_t>(stream.GetData(), stream.GetNumberOfBitsUsed()), OMPPlusProtocol::Channel);
}

IPlayer* OMPPlusComponent::getPlayer(int playerid) const
{
	if (!core_ || playerid < 0 || playerid >= PLAYER_POOL_SIZE)
		return nullptr;
	return core_->getPlayers().get(playerid);
}

OMPPlusComponent::~OMPPlusComponent()
{
	if (pawn_)
		pawn_->getEventDispatcher().removeEventHandler(this);
	if (core_)
	{
		core_->removePerRPCInEventHandler<OMPPlusProtocol::RpcID>(this);
		core_->getPlayers().getPlayerConnectDispatcher().removeEventHandler(this);
	}
	instance_ = nullptr;
}
