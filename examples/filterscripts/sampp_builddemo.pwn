#include <open.mp>
#include <sampp>

#define BUILD_DEMO_COLOUR 0x74D9FFFF
#define BUILD_DEMO_OK_COLOUR 0x9DFF86FF
#define BUILD_DEMO_WARN_COLOUR 0xFFB86CFF

#define BUILD_DEMO_MAX_OBJECTS 64
#define BUILD_DEMO_SESSION_BASE 930000
#define BUILD_DEMO_MAX_DISTANCE 12.0
#define BUILD_DEMO_FALLBACK_DISTANCE 5.0
#define BUILD_DEMO_FOUNDATION_HALF 1.5
#define BUILD_DEMO_FOUNDATION_SIZE 3.0
#define BUILD_DEMO_FOUNDATION_SNAP_RADIUS 3.75
#define BUILD_DEMO_FOUNDATION_OCCUPIED_RADIUS 0.8
#define BUILD_DEMO_FOUNDATION_Z_OFFSET -0.95
#define BUILD_DEMO_FOUNDATION_HEIGHT_STEPS 12
#define BUILD_DEMO_FOUNDATION_LIFT_STEPS 10
#define BUILD_DEMO_FOUNDATION_DEFAULT_LIFT_STEP 0
#define BUILD_DEMO_FOUNDATION_HEIGHT_STEP 0.25
#define BUILD_DEMO_FOUNDATION_LIFT_STEP 0.25
#define BUILD_DEMO_FOUNDATION_MIN_VISUAL_HEIGHT 0.05
#define BUILD_DEMO_FOUNDATION_GROUND_EMBED 0.50
#define BUILD_DEMO_FOUNDATION_TOP_CLEARANCE 0.10
#define BUILD_DEMO_FOUNDATION_SNAP_TERRAIN_EPSILON 0.05
#define BUILD_DEMO_FOUNDATION_MAX_HEIGHT 3.0
#define BUILD_DEMO_FIRST_FOUNDATION_LOCK_RADIUS 2.75
#define BUILD_DEMO_AIM_HIT_TTL_MS 300
#define BUILD_DEMO_AIM_SURFACE_NONE 0
#define BUILD_DEMO_AIM_SURFACE_GROUND 1
#define BUILD_DEMO_AIM_SURFACE_BLOCKED_NON_GROUND 2
#define BUILD_DEMO_WALL_HEIGHT 1.55
#define BUILD_DEMO_ROOF_HEIGHT 3.05
#define BUILD_DEMO_PREVIEW_MS 75
#define BUILD_DEMO_REMOVE_TARGET_MS 90
#define BUILD_DEMO_REMOVE_MAX_RAY_DISTANCE 28.0
#define BUILD_DEMO_REMOVE_MAX_PLAYER_DISTANCE 16.0
#define BUILD_DEMO_REMOVE_PRIORITY_EPSILON 0.25
#define BUILD_DEMO_EDGE_SNAP_RADIUS 2.25
#define BUILD_DEMO_CENTER_SNAP_RADIUS 2.35
#define BUILD_DEMO_REJECT_REASON_SIZE 96
#define BUILD_DEMO_WALL_YAW_OFFSET 90.0
#define BUILD_DEMO_DOORFRAME_YAW_OFFSET 90.0
#define BUILD_DEMO_DOOR_YAW_OFFSET 90.0
#define BUILD_DEMO_DOOR_Z_OFFSET BUILD_DEMO_WALL_HEIGHT
#define BUILD_DEMO_DOOR_WIDTH 1.502
#define BUILD_DEMO_DOOR_HALF_WIDTH 0.751
#define BUILD_DEMO_DOOR_HINGE_OFFSET 0.732
#define BUILD_DEMO_DOOR_OPEN_ANGLE 90.0
#define BUILD_DEMO_DOOR_MOVE_SPEED 2.5
#define BUILD_DEMO_DOOR_MOVE_MS 700
#define BUILD_DEMO_DOOR_INTERACT_MS 100
#define BUILD_DEMO_DOOR_INTERACT_DISTANCE 3.5
#define BUILD_DEMO_DOOR_INTERACT_RAY_DISTANCE 4.5
#define BUILD_DEMO_DOOR_TARGET_TTL_MS 1200
#define BUILD_DEMO_DOOR_TARGET_REPUBLISH_MS 350
#define BUILD_DEMO_DOOR_TARGET_LOST_GRACE_MS 450
#define BUILD_DEMO_DOOR_TARGET_BASE 935000
#define BUILD_DEMO_DOOR_OPTION_TOGGLE 1
#define BUILD_DEMO_STAIRS_YAW_OFFSET 0.0

#define BUILD_DEMO_SLOT_NONE 0
#define BUILD_DEMO_SLOT_EDGE 1
#define BUILD_DEMO_SLOT_TOP 2
#define BUILD_DEMO_SLOT_STAIRS 3
#define BUILD_DEMO_SLOT_DOOR 4

#define BUILD_DEMO_PIECE_NONE 0
#define BUILD_DEMO_PIECE_WALL 1
#define BUILD_DEMO_PIECE_DOORFRAME 2
#define BUILD_DEMO_PIECE_FLOOR 3
#define BUILD_DEMO_PIECE_ROOF 4
#define BUILD_DEMO_PIECE_STAIRS 5
#define BUILD_DEMO_PIECE_DOOR 6

#define BUILD_BASE_MODEL_FOUNDATION 19379
#define BUILD_MODEL_FOUNDATION -2000
#define BUILD_MODEL_FOUNDATION_PREVIEW_OK -2100
#define BUILD_MODEL_FOUNDATION_PREVIEW_BAD -2200
#define BUILD_MODEL_FOUNDATION_HIGHLIGHT -2400
#define BUILD_MODEL_FOUNDATION_HEIGHT_BASE -2007
#define BUILD_MODEL_FOUNDATION_PREVIEW_OK_HEIGHT_BASE -2107
#define BUILD_MODEL_FOUNDATION_PREVIEW_BAD_HEIGHT_BASE -2207
#define BUILD_MODEL_FOUNDATION_HIGHLIGHT_HEIGHT_BASE -2407
#define BUILD_MODEL_FOUNDATION_H0 -2007
#define BUILD_MODEL_FOUNDATION_H1 -2008
#define BUILD_MODEL_FOUNDATION_H2 -2009
#define BUILD_MODEL_FOUNDATION_H3 -2010
#define BUILD_MODEL_FOUNDATION_PREVIEW_OK_H0 -2107
#define BUILD_MODEL_FOUNDATION_PREVIEW_OK_H1 -2108
#define BUILD_MODEL_FOUNDATION_PREVIEW_OK_H2 -2109
#define BUILD_MODEL_FOUNDATION_PREVIEW_OK_H3 -2110
#define BUILD_MODEL_FOUNDATION_PREVIEW_BAD_H0 -2207
#define BUILD_MODEL_FOUNDATION_PREVIEW_BAD_H1 -2208
#define BUILD_MODEL_FOUNDATION_PREVIEW_BAD_H2 -2209
#define BUILD_MODEL_FOUNDATION_PREVIEW_BAD_H3 -2210
#define BUILD_MODEL_FOUNDATION_HIGHLIGHT_H0 -2407
#define BUILD_MODEL_FOUNDATION_HIGHLIGHT_H1 -2408
#define BUILD_MODEL_FOUNDATION_HIGHLIGHT_H2 -2409
#define BUILD_MODEL_FOUNDATION_HIGHLIGHT_H3 -2410
#define BUILD_BASE_MODEL_WALL 19380
#define BUILD_MODEL_WALL -2001
#define BUILD_MODEL_WALL_PREVIEW_OK -2101
#define BUILD_MODEL_WALL_PREVIEW_BAD -2201
#define BUILD_MODEL_WALL_HIGHLIGHT -2401
#define BUILD_BASE_MODEL_DOORFRAME 19381
#define BUILD_MODEL_DOORFRAME -2002
#define BUILD_MODEL_DOORFRAME_PREVIEW_OK -2102
#define BUILD_MODEL_DOORFRAME_PREVIEW_BAD -2202
#define BUILD_MODEL_DOORFRAME_HIGHLIGHT -2402
#define BUILD_BASE_MODEL_FLOOR 19378
#define BUILD_MODEL_FLOOR -2003
#define BUILD_MODEL_FLOOR_PREVIEW_OK -2103
#define BUILD_MODEL_FLOOR_PREVIEW_BAD -2203
#define BUILD_MODEL_FLOOR_HIGHLIGHT -2403
#define BUILD_MODEL_ROOF 19377
#define BUILD_MODEL_ROOF_PREVIEW_OK -2104
#define BUILD_MODEL_ROOF_PREVIEW_BAD -2204
#define BUILD_MODEL_ROOF_HIGHLIGHT -2404
#define BUILD_MODEL_STAIRS 19387
#define BUILD_MODEL_STAIRS_PREVIEW_OK -2105
#define BUILD_MODEL_STAIRS_PREVIEW_BAD -2205
#define BUILD_MODEL_STAIRS_HIGHLIGHT -2405
#define BUILD_BASE_MODEL_DOOR 19380
#define BUILD_MODEL_DOOR -2006
#define BUILD_MODEL_DOOR_PREVIEW_OK -2106
#define BUILD_MODEL_DOOR_PREVIEW_BAD -2206
#define BUILD_MODEL_DOOR_HIGHLIGHT -2406
#define BUILD_MODEL_PREVIEW_OK_TXD "build-preview-green.txd"
#define BUILD_MODEL_PREVIEW_BAD_TXD "build-preview-red.txd"
#define BUILD_MODEL_REMOVE_HIGHLIGHT_TXD "build-preview-orange.txd"
#define BUILD_MODEL_FOUNDATION_DFF "foundation.dff"
#define BUILD_MODEL_FOUNDATION_DFF_H0 "foundation-h0.dff"
#define BUILD_MODEL_FOUNDATION_DFF_H1 "foundation-h1.dff"
#define BUILD_MODEL_FOUNDATION_DFF_H2 "foundation-h2.dff"
#define BUILD_MODEL_FOUNDATION_DFF_H3 "foundation-h3.dff"
#define BUILD_MODEL_FOUNDATION_TXD "foundation.txd"
#define BUILD_MODEL_WALL_DFF "wall.dff"
#define BUILD_MODEL_WALL_TXD "wall.txd"
#define BUILD_MODEL_DOORFRAME_DFF "door-frame.dff"
#define BUILD_MODEL_DOORFRAME_TXD "door-frame.txd"
#define BUILD_MODEL_DOOR_DFF "door.dff"
#define BUILD_MODEL_DOOR_TXD "door.txd"
#define BUILD_MODEL_FLOOR_DFF "floor.dff"
#define BUILD_MODEL_FLOOR_TXD "floor.txd"

static gBuildSession[MAX_PLAYERS];
static bool:gBuildActive[MAX_PLAYERS];
static gBuildSelectedPart[MAX_PLAYERS];
static gBuildRotationStep[MAX_PLAYERS];
static bool:gBuildFlipped[MAX_PLAYERS];
static gBuildObjects[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static gBuildObjectPart[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static gBuildObjectFoundation[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static gBuildObjectSlotKind[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static gBuildObjectSlotIndex[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static Float:gBuildObjectX[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static Float:gBuildObjectY[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static Float:gBuildObjectZ[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static Float:gBuildObjectRX[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static Float:gBuildObjectRY[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static Float:gBuildObjectRZ[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static gBuildObjectHeightStep[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static bool:gBuildDoorOpen[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static gBuildDoorMovingUntil[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static gBuildDoorFocusedObject[MAX_PLAYERS];
static bool:gBuildDoorTargetActive[MAX_PLAYERS];
static gBuildDoorInteractNextUpdate[MAX_PLAYERS];
static gBuildDoorTargetNextPublish[MAX_PLAYERS];
static gBuildDoorTargetClearAfter[MAX_PLAYERS];
static gBuildObjectCount[MAX_PLAYERS];
static gBuildPreviewObject[MAX_PLAYERS];
static gBuildPreviewModel[MAX_PLAYERS];
static bool:gBuildPreviewValid[MAX_PLAYERS];
static gBuildPreviewNextUpdate[MAX_PLAYERS];
static Float:gBuildPreviewX[MAX_PLAYERS];
static Float:gBuildPreviewY[MAX_PLAYERS];
static Float:gBuildPreviewZ[MAX_PLAYERS];
static Float:gBuildPreviewRX[MAX_PLAYERS];
static Float:gBuildPreviewRY[MAX_PLAYERS];
static Float:gBuildPreviewRZ[MAX_PLAYERS];
static bool:gBuildPreviewPlaceable[MAX_PLAYERS];
static gBuildPreviewParentFoundation[MAX_PLAYERS];
static gBuildPreviewSlotIndex[MAX_PLAYERS];
static gBuildPreviewSlotKind[MAX_PLAYERS];
static gBuildPreviewFoundationHeightStep[MAX_PLAYERS];
static gBuildPreviewRejectReason[MAX_PLAYERS][BUILD_DEMO_REJECT_REASON_SIZE];
static bool:gBuildFoundationSnapHeightLocked[MAX_PLAYERS];
static gBuildFoundationSnapHeightParent[MAX_PLAYERS];
static gBuildFoundationSnapHeightSlot[MAX_PLAYERS];
static gBuildFoundationSnapHeightStep[MAX_PLAYERS];
static bool:gBuildFirstFoundationAimLocked[MAX_PLAYERS];
static Float:gBuildFirstFoundationAimX[MAX_PLAYERS];
static Float:gBuildFirstFoundationAimY[MAX_PLAYERS];
static Float:gBuildFirstFoundationAimZ[MAX_PLAYERS];
static Float:gBuildFirstFoundationMinGroundZ[MAX_PLAYERS];
static Float:gBuildFirstFoundationMaxGroundZ[MAX_PLAYERS];
static bool:gBuildAimHitValid[MAX_PLAYERS];
static gBuildAimHitUntil[MAX_PLAYERS];
static Float:gBuildAimHitX[MAX_PLAYERS];
static Float:gBuildAimHitY[MAX_PLAYERS];
static Float:gBuildAimHitZ[MAX_PLAYERS];
static gBuildAimSurfaceState[MAX_PLAYERS];
static Float:gBuildAimFootprintMinGroundZ[MAX_PLAYERS];
static Float:gBuildAimFootprintMaxGroundZ[MAX_PLAYERS];
static gBuildRemoveTargetNextUpdate[MAX_PLAYERS];
static gBuildRemoveFocusedObject[MAX_PLAYERS];
static gBuildRemoveHighlightObject[MAX_PLAYERS][2];
static gBuildRemoveHighlightModel[MAX_PLAYERS][2];
static bool:gBuildHasFoundation[MAX_PLAYERS];
static Float:gBuildFoundationX[MAX_PLAYERS];
static Float:gBuildFoundationY[MAX_PLAYERS];
static Float:gBuildFoundationZ[MAX_PLAYERS];
static Float:gBuildFoundationA[MAX_PLAYERS];
static gBuildFoundationCount[MAX_PLAYERS];
static bool:gBuildFoundationActive[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static Float:gBuildFoundationGridX[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static Float:gBuildFoundationGridY[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static Float:gBuildFoundationGridZ[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static Float:gBuildFoundationGridA[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static gBuildFoundationEdgePiece[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS][4];
static bool:gBuildFoundationEdgeDoor[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS][4];
static gBuildFoundationTopPiece[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
static gBuildFoundationStairsPiece[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];

stock BuildDemoSession(playerid)
{
	return BUILD_DEMO_SESSION_BASE + playerid + 1;
}

stock BuildDoorTargetId(playerid)
{
	return BUILD_DEMO_DOOR_TARGET_BASE + playerid + 1;
}

stock Float:NormalizeBuildAngle(Float:a)
{
	while (a < 0.0)
	{
		a += 360.0;
	}
	while (a >= 360.0)
	{
		a -= 360.0;
	}
	return a;
}

stock Float:GetRotationFromStep(rotationStep)
{
	return NormalizeBuildAngle(float(rotationStep % 4) * float(SAMPP_BUILD_ROTATION_STEP_DEGREES));
}

stock Float:GetBuildGridAngle(Float:a)
{
	return NormalizeBuildAngle(float(floatround(a / 90.0, floatround_round)) * 90.0);
}

stock bool:BuildDemoPartSupportsVariant(partid)
{
	switch (partid)
	{
		case SAMPP_BUILD_PART_WALL, SAMPP_BUILD_PART_DOORFRAME, SAMPP_BUILD_PART_DOOR:
		{
			return true;
		}
	}
	return false;
}

stock ClampBuildFoundationHeightStep(heightStep)
{
	if (heightStep < 0)
	{
		return 0;
	}
	if (heightStep > BUILD_DEMO_FOUNDATION_HEIGHT_STEPS)
	{
		return BUILD_DEMO_FOUNDATION_HEIGHT_STEPS;
	}
	return heightStep;
}

stock GetBuildDefaultRotationStep(partid)
{
	if (partid == SAMPP_BUILD_PART_FOUNDATION)
	{
		return BUILD_DEMO_FOUNDATION_DEFAULT_LIFT_STEP;
	}
	return 0;
}

stock ClampBuildFoundationLiftStep(liftStep)
{
	if (liftStep < 0)
	{
		return 0;
	}
	if (liftStep > BUILD_DEMO_FOUNDATION_LIFT_STEPS)
	{
		return BUILD_DEMO_FOUNDATION_LIFT_STEPS;
	}
	return liftStep;
}

stock GetBuildRotationStepForPart(partid, rotationStep)
{
	if (partid == SAMPP_BUILD_PART_FOUNDATION)
	{
		return ClampBuildFoundationLiftStep(rotationStep);
	}
	return 0;
}

stock GetBuildPlacementRotationStep(playerid, partid, rotationStep)
{
	if (partid == SAMPP_BUILD_PART_FOUNDATION && gBuildFoundationCount[playerid] > 0)
	{
		return 0;
	}
	return GetBuildRotationStepForPart(partid, rotationStep);
}

stock Float:GetBuildFoundationLiftFromStep(liftStep)
{
	return float(ClampBuildFoundationLiftStep(liftStep)) * BUILD_DEMO_FOUNDATION_LIFT_STEP;
}

stock Float:GetBuildFoundationHeightFromStep(heightStep)
{
	heightStep = ClampBuildFoundationHeightStep(heightStep);
	if (heightStep <= 0)
	{
		return BUILD_DEMO_FOUNDATION_MIN_VISUAL_HEIGHT;
	}
	return float(heightStep) * BUILD_DEMO_FOUNDATION_HEIGHT_STEP;
}

stock bool:TryGetBuildFoundationAutoHeightStep(Float:topZ, Float:groundZ, &heightStep, &Float:requiredHeight)
{
	requiredHeight = (topZ - groundZ) + BUILD_DEMO_FOUNDATION_GROUND_EMBED;
	if (requiredHeight < BUILD_DEMO_FOUNDATION_MIN_VISUAL_HEIGHT)
	{
		requiredHeight = BUILD_DEMO_FOUNDATION_MIN_VISUAL_HEIGHT;
	}

	heightStep = ClampBuildFoundationHeightStep(floatround(requiredHeight / BUILD_DEMO_FOUNDATION_HEIGHT_STEP, floatround_ceil));
	return requiredHeight <= (BUILD_DEMO_FOUNDATION_MAX_HEIGHT + 0.001);
}

stock bool:TryGetBuildFoundationAutoHeightStepFromRange(Float:topZ, Float:minGroundZ, Float:maxGroundZ, &heightStep, &Float:requiredHeight)
{
	if (minGroundZ > maxGroundZ)
	{
		new Float:tmp = minGroundZ;
		minGroundZ = maxGroundZ;
		maxGroundZ = tmp;
	}

	if (topZ < maxGroundZ + BUILD_DEMO_FOUNDATION_TOP_CLEARANCE)
	{
		requiredHeight = 0.0;
		heightStep = 0;
		return false;
	}

	return TryGetBuildFoundationAutoHeightStep(topZ, minGroundZ, heightStep, requiredHeight);
}

stock GetBuildFoundationAutoHeightStep(Float:topZ, Float:groundZ)
{
	new heightStep;
	new Float:requiredHeight;
	TryGetBuildFoundationAutoHeightStep(topZ, groundZ, heightStep, requiredHeight);
	return heightStep;
}

stock Float:GetBuildEdgePieceYaw(partid, Float:edgeA, bool:flipped)
{
	new Float:offset = 0.0;
	switch (partid)
	{
		case SAMPP_BUILD_PART_WALL:
		{
			offset = BUILD_DEMO_WALL_YAW_OFFSET;
		}
		case SAMPP_BUILD_PART_DOORFRAME:
		{
			offset = BUILD_DEMO_DOORFRAME_YAW_OFFSET;
		}
		case SAMPP_BUILD_PART_DOOR:
		{
			offset = BUILD_DEMO_DOOR_YAW_OFFSET;
		}
	}
	return NormalizeBuildAngle(edgeA + offset + (flipped ? 180.0 : 0.0));
}

stock GetForwardPoint(Float:x, Float:y, Float:a, Float:distance, &Float:outX, &Float:outY)
{
	outX = x + (floatsin(-a, degrees) * distance);
	outY = y + (floatcos(-a, degrees) * distance);
	return 1;
}

stock FitBuildDoorToFrame(Float:baseX, Float:baseY, Float:baseZ, Float:edgeA, bool:flipped, &Float:x, &Float:y, &Float:z, &Float:rz)
{
	rz = GetBuildEdgePieceYaw(SAMPP_BUILD_PART_DOOR, edgeA, flipped);
	GetForwardPoint(baseX, baseY, rz, -BUILD_DEMO_DOOR_HINGE_OFFSET, x, y);
	z = baseZ + BUILD_DEMO_DOOR_Z_OFFSET;
	return 1;
}

stock Float:GetBuildDistance2D(Float:x1, Float:y1, Float:x2, Float:y2)
{
	new Float:dx = x1 - x2;
	new Float:dy = y1 - y2;
	return floatsqroot((dx * dx) + (dy * dy));
}

stock GetBuildPartModel(partid)
{
	switch (partid)
	{
		case SAMPP_BUILD_PART_FOUNDATION: return BUILD_MODEL_FOUNDATION;
		case SAMPP_BUILD_PART_WALL: return BUILD_MODEL_WALL;
		case SAMPP_BUILD_PART_DOORFRAME: return BUILD_MODEL_DOORFRAME;
		case SAMPP_BUILD_PART_FLOOR: return BUILD_MODEL_FLOOR;
		case SAMPP_BUILD_PART_ROOF: return BUILD_MODEL_ROOF;
		case SAMPP_BUILD_PART_STAIRS: return BUILD_MODEL_STAIRS;
		case SAMPP_BUILD_PART_DOOR: return BUILD_MODEL_DOOR;
	}
	return 0;
}

stock GetBuildFoundationModel(heightStep)
{
	heightStep = ClampBuildFoundationHeightStep(heightStep);
	if (heightStep < BUILD_DEMO_FOUNDATION_HEIGHT_STEPS)
	{
		return BUILD_MODEL_FOUNDATION_HEIGHT_BASE - heightStep;
	}
	return BUILD_MODEL_FOUNDATION;
}

stock GetBuildFoundationPreviewModel(heightStep, bool:placeable)
{
	heightStep = ClampBuildFoundationHeightStep(heightStep);
	if (heightStep < BUILD_DEMO_FOUNDATION_HEIGHT_STEPS)
	{
		return placeable
			? BUILD_MODEL_FOUNDATION_PREVIEW_OK_HEIGHT_BASE - heightStep
			: BUILD_MODEL_FOUNDATION_PREVIEW_BAD_HEIGHT_BASE - heightStep;
	}
	return placeable ? BUILD_MODEL_FOUNDATION_PREVIEW_OK : BUILD_MODEL_FOUNDATION_PREVIEW_BAD;
}

stock GetBuildFoundationHighlightModel(heightStep)
{
	heightStep = ClampBuildFoundationHeightStep(heightStep);
	if (heightStep < BUILD_DEMO_FOUNDATION_HEIGHT_STEPS)
	{
		return BUILD_MODEL_FOUNDATION_HIGHLIGHT_HEIGHT_BASE - heightStep;
	}
	return BUILD_MODEL_FOUNDATION_HIGHLIGHT;
}

stock GetBuildFoundationDff(heightStep, dff[], size)
{
	heightStep = ClampBuildFoundationHeightStep(heightStep);
	if (heightStep < BUILD_DEMO_FOUNDATION_HEIGHT_STEPS)
	{
		format(dff, size, "foundation-h%d.dff", heightStep);
		return 1;
	}
	format(dff, size, BUILD_MODEL_FOUNDATION_DFF);
	return 1;
}

stock GetBuildPartModelForState(partid, rotationStep)
{
	if (partid == SAMPP_BUILD_PART_FOUNDATION)
	{
		return GetBuildFoundationModel(rotationStep);
	}
	return GetBuildPartModel(partid);
}

stock GetBuildPreviewPartModel(partid, bool:placeable)
{
	switch (partid)
	{
		case SAMPP_BUILD_PART_FOUNDATION: return placeable ? BUILD_MODEL_FOUNDATION_PREVIEW_OK : BUILD_MODEL_FOUNDATION_PREVIEW_BAD;
		case SAMPP_BUILD_PART_WALL: return placeable ? BUILD_MODEL_WALL_PREVIEW_OK : BUILD_MODEL_WALL_PREVIEW_BAD;
		case SAMPP_BUILD_PART_DOORFRAME: return placeable ? BUILD_MODEL_DOORFRAME_PREVIEW_OK : BUILD_MODEL_DOORFRAME_PREVIEW_BAD;
		case SAMPP_BUILD_PART_FLOOR: return placeable ? BUILD_MODEL_FLOOR_PREVIEW_OK : BUILD_MODEL_FLOOR_PREVIEW_BAD;
		case SAMPP_BUILD_PART_ROOF: return placeable ? BUILD_MODEL_ROOF_PREVIEW_OK : BUILD_MODEL_ROOF_PREVIEW_BAD;
		case SAMPP_BUILD_PART_STAIRS: return placeable ? BUILD_MODEL_STAIRS_PREVIEW_OK : BUILD_MODEL_STAIRS_PREVIEW_BAD;
		case SAMPP_BUILD_PART_DOOR: return placeable ? BUILD_MODEL_DOOR_PREVIEW_OK : BUILD_MODEL_DOOR_PREVIEW_BAD;
	}
	return GetBuildPartModel(partid);
}

stock GetBuildPreviewPartModelForState(partid, bool:placeable, rotationStep)
{
	if (partid == SAMPP_BUILD_PART_FOUNDATION)
	{
		return GetBuildFoundationPreviewModel(rotationStep, placeable);
	}
	return GetBuildPreviewPartModel(partid, placeable);
}

stock GetBuildRemoveHighlightPartModel(partid)
{
	switch (partid)
	{
		case SAMPP_BUILD_PART_FOUNDATION: return BUILD_MODEL_FOUNDATION_HIGHLIGHT;
		case SAMPP_BUILD_PART_WALL: return BUILD_MODEL_WALL_HIGHLIGHT;
		case SAMPP_BUILD_PART_DOORFRAME: return BUILD_MODEL_DOORFRAME_HIGHLIGHT;
		case SAMPP_BUILD_PART_FLOOR: return BUILD_MODEL_FLOOR_HIGHLIGHT;
		case SAMPP_BUILD_PART_ROOF: return BUILD_MODEL_ROOF_HIGHLIGHT;
		case SAMPP_BUILD_PART_STAIRS: return BUILD_MODEL_STAIRS_HIGHLIGHT;
		case SAMPP_BUILD_PART_DOOR: return BUILD_MODEL_DOOR_HIGHLIGHT;
	}
	return 0;
}

stock GetBuildRemoveHighlightModelForObject(playerid, objectIndex)
{
	if (objectIndex < 0 || objectIndex >= BUILD_DEMO_MAX_OBJECTS)
	{
		return 0;
	}
	if (gBuildObjectPart[playerid][objectIndex] == SAMPP_BUILD_PART_FOUNDATION)
	{
		return GetBuildFoundationHighlightModel(gBuildObjectHeightStep[playerid][objectIndex]);
	}
	return GetBuildRemoveHighlightPartModel(gBuildObjectPart[playerid][objectIndex]);
}

stock GetBuildRemoveHighlightLayerCount(partid)
{
	switch (partid)
	{
		case SAMPP_BUILD_PART_WALL, SAMPP_BUILD_PART_DOORFRAME, SAMPP_BUILD_PART_DOOR:
		{
			return 2;
		}
		case SAMPP_BUILD_PART_FLOOR, SAMPP_BUILD_PART_ROOF:
		{
			return 2;
		}
	}
	return 1;
}

stock OffsetBuildRemoveHighlight(partid, layer, Float:rz, &Float:x, &Float:y, &Float:z)
{
	switch (partid)
	{
		case SAMPP_BUILD_PART_FOUNDATION:
		{
			z += 0.035;
		}
		case SAMPP_BUILD_PART_FLOOR, SAMPP_BUILD_PART_ROOF:
		{
			z += layer == 0 ? 0.035 : -0.035;
		}
		case SAMPP_BUILD_PART_WALL, SAMPP_BUILD_PART_DOORFRAME, SAMPP_BUILD_PART_DOOR:
		{
			new Float:outX;
			new Float:outY;
			new Float:offset = layer == 0 ? 0.035 : -0.035;
			GetForwardPoint(x, y, NormalizeBuildAngle(rz - 90.0), offset, outX, outY);
			x = outX;
			y = outY;
		}
		case SAMPP_BUILD_PART_STAIRS:
		{
			z += 0.045;
		}
	}
	return 1;
}

stock Float:GetBuildRemoveAimRadius(partid)
{
	switch (partid)
	{
		case SAMPP_BUILD_PART_FOUNDATION, SAMPP_BUILD_PART_FLOOR, SAMPP_BUILD_PART_ROOF:
		{
			return 2.25;
		}
		case SAMPP_BUILD_PART_WALL, SAMPP_BUILD_PART_DOORFRAME:
		{
			return 1.45;
		}
		case SAMPP_BUILD_PART_STAIRS:
		{
			return 1.60;
		}
		case SAMPP_BUILD_PART_DOOR:
		{
			return 0.90;
		}
	}
	return 1.20;
}

stock GetBuildRemovePriority(partid)
{
	switch (partid)
	{
		case SAMPP_BUILD_PART_WALL, SAMPP_BUILD_PART_DOORFRAME, SAMPP_BUILD_PART_DOOR:
		{
			return 0;
		}
		case SAMPP_BUILD_PART_FOUNDATION, SAMPP_BUILD_PART_FLOOR, SAMPP_BUILD_PART_ROOF:
		{
			return 1;
		}
	}
	return 2;
}

stock Float:GetBuildObjectCurrentRZ(playerid, objectIndex)
{
	if (objectIndex < 0 || objectIndex >= BUILD_DEMO_MAX_OBJECTS)
	{
		return 0.0;
	}

	if (gBuildObjectPart[playerid][objectIndex] == SAMPP_BUILD_PART_DOOR && gBuildDoorOpen[playerid][objectIndex])
	{
		return NormalizeBuildAngle(gBuildObjectRZ[playerid][objectIndex] + BUILD_DEMO_DOOR_OPEN_ANGLE);
	}
	return gBuildObjectRZ[playerid][objectIndex];
}

stock GetBuildRemoveOBBHalfExtents(partid, &Float:halfX, &Float:halfY, &Float:halfZ)
{
	switch (partid)
	{
		case SAMPP_BUILD_PART_FOUNDATION:
		{
			halfX = BUILD_DEMO_FOUNDATION_HALF + 0.12;
			halfY = BUILD_DEMO_FOUNDATION_HALF + 0.12;
			halfZ = 0.18;
			return 1;
		}
		case SAMPP_BUILD_PART_FLOOR, SAMPP_BUILD_PART_ROOF:
		{
			halfX = BUILD_DEMO_FOUNDATION_HALF + 0.12;
			halfY = BUILD_DEMO_FOUNDATION_HALF + 0.12;
			halfZ = 0.22;
			return 1;
		}
		case SAMPP_BUILD_PART_WALL, SAMPP_BUILD_PART_DOORFRAME:
		{
			halfX = BUILD_DEMO_FOUNDATION_HALF + 0.28;
			halfY = 0.22;
			halfZ = 1.95;
			return 1;
		}
		case SAMPP_BUILD_PART_DOOR:
		{
			halfX = 0.90;
			halfY = 0.18;
			halfZ = 1.45;
			return 1;
		}
		case SAMPP_BUILD_PART_STAIRS:
		{
			halfX = BUILD_DEMO_FOUNDATION_HALF + 0.15;
			halfY = BUILD_DEMO_FOUNDATION_HALF + 0.15;
			halfZ = 0.95;
			return 1;
		}
	}

	return 0;
}

stock bool:UpdateBuildRaySlab(Float:origin, Float:direction, Float:minBound, Float:maxBound, &Float:tMin, &Float:tMax)
{
	if (floatabs(direction) <= 0.0001)
	{
		return origin >= minBound && origin <= maxBound;
	}

	new Float:t1 = (minBound - origin) / direction;
	new Float:t2 = (maxBound - origin) / direction;
	if (t1 > t2)
	{
		new Float:tmp = t1;
		t1 = t2;
		t2 = tmp;
	}

	if (t1 > tMin)
	{
		tMin = t1;
	}
	if (t2 < tMax)
	{
		tMax = t2;
	}

	return tMin <= tMax;
}

stock bool:RayIntersectsBuildRemoveOBB(partid, Float:objectX, Float:objectY, Float:objectZ, Float:objectRZ, Float:camX, Float:camY, Float:camZ, Float:frontX, Float:frontY, Float:frontZ, &Float:distance, heightStep = 0)
{
	new Float:minX;
	new Float:maxX;
	new Float:minY;
	new Float:maxY;
	new Float:minZ;
	new Float:maxZ;
	if (partid == SAMPP_BUILD_PART_FOUNDATION)
	{
		minX = -(BUILD_DEMO_FOUNDATION_HALF + 0.12);
		maxX = BUILD_DEMO_FOUNDATION_HALF + 0.12;
		minY = -(BUILD_DEMO_FOUNDATION_HALF + 0.12);
		maxY = BUILD_DEMO_FOUNDATION_HALF + 0.12;
		minZ = -GetBuildFoundationHeightFromStep(heightStep) - 0.08;
		maxZ = 0.20;
	}
	else if (partid == SAMPP_BUILD_PART_DOOR)
	{
		minX = -0.12;
		maxX = BUILD_DEMO_DOOR_WIDTH + 0.12;
		minY = -0.18;
		maxY = 0.18;
		minZ = -1.55;
		maxZ = 1.05;
	}
	else
	{
		new Float:halfX;
		new Float:halfY;
		new Float:halfZ;
		if (!GetBuildRemoveOBBHalfExtents(partid, halfX, halfY, halfZ))
		{
			return false;
		}
		minX = -halfX;
		maxX = halfX;
		minY = -halfY;
		maxY = halfY;
		minZ = -halfZ;
		maxZ = halfZ;
	}

	new Float:dx = camX - objectX;
	new Float:dy = camY - objectY;
	new Float:dz = camZ - objectZ;
	new Float:axisX = floatsin(-objectRZ, degrees);
	new Float:axisY = floatcos(-objectRZ, degrees);
	new Float:rightX = floatcos(-objectRZ, degrees);
	new Float:rightY = -floatsin(-objectRZ, degrees);

	new Float:localOriginX = (dx * axisX) + (dy * axisY);
	new Float:localOriginY = (dx * rightX) + (dy * rightY);
	new Float:localOriginZ = dz;
	new Float:localDirX = (frontX * axisX) + (frontY * axisY);
	new Float:localDirY = (frontX * rightX) + (frontY * rightY);
	new Float:localDirZ = frontZ;

	new Float:tMin = 0.35;
	new Float:tMax = BUILD_DEMO_REMOVE_MAX_RAY_DISTANCE;
	if (!UpdateBuildRaySlab(localOriginX, localDirX, minX, maxX, tMin, tMax)
		|| !UpdateBuildRaySlab(localOriginY, localDirY, minY, maxY, tMin, tMax)
		|| !UpdateBuildRaySlab(localOriginZ, localDirZ, minZ, maxZ, tMin, tMax))
	{
		return false;
	}

	if (tMax < 0.35 || tMin > BUILD_DEMO_REMOVE_MAX_RAY_DISTANCE)
	{
		return false;
	}

	distance = tMin >= 0.35 ? tMin : tMax;
	return distance >= 0.35 && distance <= BUILD_DEMO_REMOVE_MAX_RAY_DISTANCE;
}

stock GetBuildPartDisplayName(partid, name[], size)
{
	switch (partid)
	{
		case SAMPP_BUILD_PART_FOUNDATION: format(name, size, "Foundation");
		case SAMPP_BUILD_PART_WALL: format(name, size, "Wall");
		case SAMPP_BUILD_PART_DOORFRAME: format(name, size, "Door Frame");
		case SAMPP_BUILD_PART_FLOOR: format(name, size, "Floor/Ceiling");
		case SAMPP_BUILD_PART_ROOF: format(name, size, "Roof");
		case SAMPP_BUILD_PART_STAIRS: format(name, size, "Stairs");
		case SAMPP_BUILD_PART_DOOR: format(name, size, "Door");
		case SAMPP_BUILD_PART_REMOVE: format(name, size, "Remove Tool");
		default: format(name, size, "Build Part");
	}
	return 1;
}

stock SetBuildPreviewCandidate(playerid, bool:placeable, parentFoundation, slotIndex, slotKind, const reason[] = "")
{
	gBuildPreviewPlaceable[playerid] = placeable;
	gBuildPreviewParentFoundation[playerid] = parentFoundation;
	gBuildPreviewSlotIndex[playerid] = slotIndex;
	gBuildPreviewSlotKind[playerid] = slotKind;
	format(gBuildPreviewRejectReason[playerid], sizeof(gBuildPreviewRejectReason[]), "%s", reason);
	return 1;
}

stock ResetBuildFoundationSnapHeightLock(playerid)
{
	gBuildFoundationSnapHeightLocked[playerid] = false;
	gBuildFoundationSnapHeightParent[playerid] = -1;
	gBuildFoundationSnapHeightSlot[playerid] = -1;
	gBuildFoundationSnapHeightStep[playerid] = 0;
	return 1;
}

stock ResetBuildFirstFoundationAimLock(playerid)
{
	gBuildFirstFoundationAimLocked[playerid] = false;
	gBuildFirstFoundationAimX[playerid] = 0.0;
	gBuildFirstFoundationAimY[playerid] = 0.0;
	gBuildFirstFoundationAimZ[playerid] = 0.0;
	gBuildFirstFoundationMinGroundZ[playerid] = 0.0;
	gBuildFirstFoundationMaxGroundZ[playerid] = 0.0;
	return 1;
}

stock LockBuildFirstFoundationAim(playerid, Float:aimX, Float:aimY, Float:aimZ, Float:minGroundZ, Float:maxGroundZ)
{
	if (minGroundZ > maxGroundZ)
	{
		new Float:tmp = minGroundZ;
		minGroundZ = maxGroundZ;
		maxGroundZ = tmp;
	}

	gBuildFirstFoundationAimLocked[playerid] = true;
	gBuildFirstFoundationAimX[playerid] = aimX;
	gBuildFirstFoundationAimY[playerid] = aimY;
	gBuildFirstFoundationAimZ[playerid] = aimZ;
	gBuildFirstFoundationMinGroundZ[playerid] = minGroundZ;
	gBuildFirstFoundationMaxGroundZ[playerid] = maxGroundZ;
	return 1;
}

stock ApplyBuildFirstFoundationAimLock(playerid, &Float:aimX, &Float:aimY, &Float:aimZ, &aimSurfaceState, &Float:minGroundZ, &Float:maxGroundZ)
{
	if (gBuildFoundationCount[playerid] > 0)
	{
		ResetBuildFirstFoundationAimLock(playerid);
		return 1;
	}

	if (aimSurfaceState == BUILD_DEMO_AIM_SURFACE_BLOCKED_NON_GROUND)
	{
		if (gBuildFirstFoundationAimLocked[playerid]
			&& GetBuildDistance2D(aimX, aimY, gBuildFirstFoundationAimX[playerid], gBuildFirstFoundationAimY[playerid]) <= BUILD_DEMO_FIRST_FOUNDATION_LOCK_RADIUS)
		{
			aimX = gBuildFirstFoundationAimX[playerid];
			aimY = gBuildFirstFoundationAimY[playerid];
			aimZ = gBuildFirstFoundationAimZ[playerid];
			minGroundZ = gBuildFirstFoundationMinGroundZ[playerid];
			maxGroundZ = gBuildFirstFoundationMaxGroundZ[playerid];
			aimSurfaceState = BUILD_DEMO_AIM_SURFACE_GROUND;
		}
		return 1;
	}

	if (!gBuildFirstFoundationAimLocked[playerid])
	{
		return LockBuildFirstFoundationAim(playerid, aimX, aimY, aimZ, minGroundZ, maxGroundZ);
	}

	if (GetBuildDistance2D(aimX, aimY, gBuildFirstFoundationAimX[playerid], gBuildFirstFoundationAimY[playerid]) > BUILD_DEMO_FIRST_FOUNDATION_LOCK_RADIUS)
	{
		return LockBuildFirstFoundationAim(playerid, aimX, aimY, aimZ, minGroundZ, maxGroundZ);
	}

	aimX = gBuildFirstFoundationAimX[playerid];
	aimY = gBuildFirstFoundationAimY[playerid];
	aimZ = gBuildFirstFoundationAimZ[playerid];
	minGroundZ = gBuildFirstFoundationMinGroundZ[playerid];
	maxGroundZ = gBuildFirstFoundationMaxGroundZ[playerid];
	return 1;
}

stock ResetBuildPreviewState(playerid)
{
	gBuildSelectedPart[playerid] = 0;
	gBuildRotationStep[playerid] = 0;
	gBuildFlipped[playerid] = false;
	gBuildPreviewValid[playerid] = false;
	gBuildPreviewNextUpdate[playerid] = 0;
	gBuildPreviewModel[playerid] = 0;
	gBuildPreviewX[playerid] = 0.0;
	gBuildPreviewY[playerid] = 0.0;
	gBuildPreviewZ[playerid] = 0.0;
	gBuildPreviewRX[playerid] = 0.0;
	gBuildPreviewRY[playerid] = 0.0;
	gBuildPreviewRZ[playerid] = 0.0;
	gBuildPreviewFoundationHeightStep[playerid] = 0;
	ResetBuildFoundationSnapHeightLock(playerid);
	ResetBuildFirstFoundationAimLock(playerid);
	SetBuildAimHit(playerid, false, 0.0, 0.0, 0.0);
	gBuildRemoveTargetNextUpdate[playerid] = 0;
	gBuildRemoveFocusedObject[playerid] = -1;
	SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_NONE, "");
	return 1;
}

stock DestroyBuildRemoveHighlight(playerid)
{
	for (new layer = 0; layer < 2; layer++)
	{
		if (gBuildRemoveHighlightObject[playerid][layer] != INVALID_OBJECT_ID)
		{
			DestroyPlayerObject(playerid, gBuildRemoveHighlightObject[playerid][layer]);
		}
		gBuildRemoveHighlightObject[playerid][layer] = INVALID_OBJECT_ID;
		gBuildRemoveHighlightModel[playerid][layer] = 0;
	}
	return 1;
}

stock DestroyBuildPreview(playerid)
{
	if (gBuildPreviewObject[playerid] != INVALID_OBJECT_ID)
	{
		DestroyPlayerObject(playerid, gBuildPreviewObject[playerid]);
	}

	gBuildPreviewObject[playerid] = INVALID_OBJECT_ID;
	gBuildPreviewModel[playerid] = 0;
	gBuildPreviewValid[playerid] = false;
	return 1;
}

stock ResetBuildObjectSlot(playerid, index)
{
	gBuildObjects[playerid][index] = INVALID_OBJECT_ID;
	gBuildObjectPart[playerid][index] = SAMPP_BUILD_PART_NONE;
	gBuildObjectFoundation[playerid][index] = -1;
	gBuildObjectSlotKind[playerid][index] = BUILD_DEMO_SLOT_NONE;
	gBuildObjectSlotIndex[playerid][index] = -1;
	gBuildObjectX[playerid][index] = 0.0;
	gBuildObjectY[playerid][index] = 0.0;
	gBuildObjectZ[playerid][index] = 0.0;
	gBuildObjectRX[playerid][index] = 0.0;
	gBuildObjectRY[playerid][index] = 0.0;
	gBuildObjectRZ[playerid][index] = 0.0;
	gBuildObjectHeightStep[playerid][index] = 0;
	gBuildDoorOpen[playerid][index] = false;
	gBuildDoorMovingUntil[playerid][index] = 0;
	return 1;
}

stock ResetBuildDemoPlayer(playerid)
{
	gBuildSession[playerid] = BuildDemoSession(playerid);
	gBuildActive[playerid] = false;
	gBuildObjectCount[playerid] = 0;
	gBuildDoorFocusedObject[playerid] = -1;
	gBuildDoorTargetActive[playerid] = false;
	gBuildDoorInteractNextUpdate[playerid] = 0;
	gBuildDoorTargetNextPublish[playerid] = 0;
	gBuildDoorTargetClearAfter[playerid] = 0;
	gBuildPreviewObject[playerid] = INVALID_OBJECT_ID;
	for (new layer = 0; layer < 2; layer++)
	{
		gBuildRemoveHighlightObject[playerid][layer] = INVALID_OBJECT_ID;
		gBuildRemoveHighlightModel[playerid][layer] = 0;
	}
	gBuildHasFoundation[playerid] = false;
	gBuildFoundationX[playerid] = 0.0;
	gBuildFoundationY[playerid] = 0.0;
	gBuildFoundationZ[playerid] = 0.0;
	gBuildFoundationA[playerid] = 0.0;
	gBuildFoundationCount[playerid] = 0;
	ResetBuildPreviewState(playerid);

	for (new i = 0; i < BUILD_DEMO_MAX_OBJECTS; i++)
	{
		ResetBuildObjectSlot(playerid, i);
		gBuildFoundationActive[playerid][i] = false;
		gBuildFoundationGridX[playerid][i] = 0.0;
		gBuildFoundationGridY[playerid][i] = 0.0;
		gBuildFoundationGridZ[playerid][i] = 0.0;
		gBuildFoundationGridA[playerid][i] = 0.0;
		gBuildFoundationTopPiece[playerid][i] = BUILD_DEMO_PIECE_NONE;
		gBuildFoundationStairsPiece[playerid][i] = BUILD_DEMO_PIECE_NONE;
		for (new slot = 0; slot < 4; slot++)
		{
			gBuildFoundationEdgePiece[playerid][i][slot] = BUILD_DEMO_PIECE_NONE;
			gBuildFoundationEdgeDoor[playerid][i][slot] = false;
		}
	}
	return 1;
}

stock DestroyBuildDemoObjects(playerid)
{
	DestroyBuildPreview(playerid);
	DestroyBuildRemoveHighlight(playerid);
	EndBuildDoorInteraction(playerid);

	for (new i = 0; i < BUILD_DEMO_MAX_OBJECTS; i++)
	{
		if (gBuildObjects[playerid][i] != INVALID_OBJECT_ID)
		{
			DestroyObject(gBuildObjects[playerid][i]);
		}
		ResetBuildObjectSlot(playerid, i);
	}

	gBuildObjectCount[playerid] = 0;
	gBuildHasFoundation[playerid] = false;
	gBuildFoundationCount[playerid] = 0;
	for (new i = 0; i < BUILD_DEMO_MAX_OBJECTS; i++)
	{
		gBuildFoundationActive[playerid][i] = false;
		gBuildFoundationGridX[playerid][i] = 0.0;
		gBuildFoundationGridY[playerid][i] = 0.0;
		gBuildFoundationGridZ[playerid][i] = 0.0;
		gBuildFoundationGridA[playerid][i] = 0.0;
		gBuildFoundationTopPiece[playerid][i] = BUILD_DEMO_PIECE_NONE;
		gBuildFoundationStairsPiece[playerid][i] = BUILD_DEMO_PIECE_NONE;
		for (new slot = 0; slot < 4; slot++)
		{
			gBuildFoundationEdgePiece[playerid][i][slot] = BUILD_DEMO_PIECE_NONE;
			gBuildFoundationEdgeDoor[playerid][i][slot] = false;
		}
	}
	return 1;
}

stock bool:AddBuildDemoObject(playerid, objectid, partid, foundationIndex, slotKind, slotIndex, heightStep, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz)
{
	if (objectid == INVALID_OBJECT_ID)
	{
		SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "Could not create object. Check demo model ids.");
		return false;
	}

	if (gBuildObjectCount[playerid] >= BUILD_DEMO_MAX_OBJECTS)
	{
		DestroyObject(objectid);
		SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "Demo object limit reached.");
		return false;
	}

	for (new i = 0; i < BUILD_DEMO_MAX_OBJECTS; i++)
	{
		if (gBuildObjects[playerid][i] == INVALID_OBJECT_ID)
		{
			gBuildObjects[playerid][i] = objectid;
			gBuildObjectPart[playerid][i] = partid;
			gBuildObjectFoundation[playerid][i] = foundationIndex;
			gBuildObjectSlotKind[playerid][i] = slotKind;
			gBuildObjectSlotIndex[playerid][i] = slotIndex;
			gBuildObjectX[playerid][i] = x;
			gBuildObjectY[playerid][i] = y;
			gBuildObjectZ[playerid][i] = z;
			gBuildObjectRX[playerid][i] = rx;
			gBuildObjectRY[playerid][i] = ry;
			gBuildObjectRZ[playerid][i] = rz;
			gBuildObjectHeightStep[playerid][i] = partid == SAMPP_BUILD_PART_FOUNDATION ? ClampBuildFoundationHeightStep(heightStep) : 0;
			gBuildDoorOpen[playerid][i] = false;
			gBuildDoorMovingUntil[playerid][i] = 0;
			gBuildObjectCount[playerid]++;
			return true;
		}
	}

	DestroyObject(objectid);
	SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "Demo object slots are full.");
	return false;
}

stock bool:IsBuildFoundationSlotOccupied(playerid, Float:x, Float:y)
{
	for (new i = 0; i < BUILD_DEMO_MAX_OBJECTS; i++)
	{
		if (!gBuildFoundationActive[playerid][i])
		{
			continue;
		}

		if (GetBuildDistance2D(x, y, gBuildFoundationGridX[playerid][i], gBuildFoundationGridY[playerid][i]) <= BUILD_DEMO_FOUNDATION_OCCUPIED_RADIUS)
		{
			return true;
		}
	}
	return false;
}

stock bool:RegisterBuildFoundation(playerid, Float:x, Float:y, Float:z, Float:a, &outIndex)
{
	if (gBuildFoundationCount[playerid] >= BUILD_DEMO_MAX_OBJECTS)
	{
		return false;
	}

	new index = -1;
	for (new i = 0; i < BUILD_DEMO_MAX_OBJECTS; i++)
	{
		if (!gBuildFoundationActive[playerid][i])
		{
			index = i;
			break;
		}
	}

	if (index == -1)
	{
		return false;
	}

	gBuildFoundationActive[playerid][index] = true;
	gBuildFoundationCount[playerid]++;
	gBuildFoundationGridX[playerid][index] = x;
	gBuildFoundationGridY[playerid][index] = y;
	gBuildFoundationGridZ[playerid][index] = z;
	gBuildFoundationGridA[playerid][index] = NormalizeBuildAngle(a);
	gBuildFoundationTopPiece[playerid][index] = BUILD_DEMO_PIECE_NONE;
	gBuildFoundationStairsPiece[playerid][index] = BUILD_DEMO_PIECE_NONE;
	for (new slot = 0; slot < 4; slot++)
	{
		gBuildFoundationEdgePiece[playerid][index][slot] = BUILD_DEMO_PIECE_NONE;
		gBuildFoundationEdgeDoor[playerid][index][slot] = false;
	}

	gBuildHasFoundation[playerid] = true;
	gBuildFoundationX[playerid] = x;
	gBuildFoundationY[playerid] = y;
	gBuildFoundationZ[playerid] = z;
	gBuildFoundationA[playerid] = NormalizeBuildAngle(a);
	outIndex = index;
	return true;
}

stock bool:ComputeAimPoint(playerid, Float:zOffset, &Float:x, &Float:y, &Float:z, &Float:angle)
{
	new Float:px, Float:py, Float:pz;
	new Float:camX, Float:camY, Float:camZ;
	new Float:frontX, Float:frontY, Float:frontZ;
	if (!GetPlayerPos(playerid, px, py, pz)
		|| !GetPlayerCameraPos(playerid, camX, camY, camZ)
		|| !GetPlayerCameraFrontVector(playerid, frontX, frontY, frontZ))
	{
		return false;
	}

	GetPlayerFacingAngle(playerid, angle);

	new Float:groundZ = pz + zOffset;
	if (floatabs(frontZ) > 0.035)
	{
		new Float:t = (groundZ - camZ) / frontZ;
		if (t >= 1.5 && t <= 30.0)
		{
			x = camX + (frontX * t);
			y = camY + (frontY * t);
			z = groundZ;

			new Float:dx = x - px;
			new Float:dy = y - py;
			new Float:distance = floatsqroot((dx * dx) + (dy * dy));
			if (distance > BUILD_DEMO_MAX_DISTANCE)
			{
				new Float:scale = BUILD_DEMO_MAX_DISTANCE / distance;
				x = px + (dx * scale);
				y = py + (dy * scale);
			}
			return true;
		}
	}

	GetForwardPoint(px, py, angle, BUILD_DEMO_FALLBACK_DISTANCE, x, y);
	z = groundZ;
	return true;
}

stock SetBuildAimGroundHit(playerid, bool:hasAimHit, Float:aimX, Float:aimY, Float:aimZ, aimSurfaceState, Float:footprintMinGroundZ, Float:footprintMaxGroundZ)
{
	if (!hasAimHit)
	{
		gBuildAimHitValid[playerid] = false;
		gBuildAimHitUntil[playerid] = 0;
		gBuildAimHitX[playerid] = 0.0;
		gBuildAimHitY[playerid] = 0.0;
		gBuildAimHitZ[playerid] = 0.0;
		gBuildAimSurfaceState[playerid] = BUILD_DEMO_AIM_SURFACE_NONE;
		gBuildAimFootprintMinGroundZ[playerid] = 0.0;
		gBuildAimFootprintMaxGroundZ[playerid] = 0.0;
		return 1;
	}

	if (aimSurfaceState < BUILD_DEMO_AIM_SURFACE_NONE || aimSurfaceState > BUILD_DEMO_AIM_SURFACE_BLOCKED_NON_GROUND)
	{
		aimSurfaceState = BUILD_DEMO_AIM_SURFACE_GROUND;
	}
	if (footprintMinGroundZ > footprintMaxGroundZ)
	{
		new Float:tmp = footprintMinGroundZ;
		footprintMinGroundZ = footprintMaxGroundZ;
		footprintMaxGroundZ = tmp;
	}

	gBuildAimHitValid[playerid] = true;
	gBuildAimHitUntil[playerid] = GetTickCount() + BUILD_DEMO_AIM_HIT_TTL_MS;
	gBuildAimHitX[playerid] = aimX;
	gBuildAimHitY[playerid] = aimY;
	gBuildAimHitZ[playerid] = aimZ;
	gBuildAimSurfaceState[playerid] = aimSurfaceState;
	gBuildAimFootprintMinGroundZ[playerid] = footprintMinGroundZ;
	gBuildAimFootprintMaxGroundZ[playerid] = footprintMaxGroundZ;
	return 1;
}

stock SetBuildAimHit(playerid, bool:hasAimHit, Float:aimX, Float:aimY, Float:aimZ)
{
	return SetBuildAimGroundHit(playerid, hasAimHit, aimX, aimY, aimZ, hasAimHit ? BUILD_DEMO_AIM_SURFACE_GROUND : BUILD_DEMO_AIM_SURFACE_NONE, aimZ, aimZ);
}

stock bool:IsBuildAimHitFresh(playerid)
{
	if (!gBuildAimHitValid[playerid])
	{
		return false;
	}

	if (GetTickCount() > gBuildAimHitUntil[playerid])
	{
		gBuildAimHitValid[playerid] = false;
		gBuildAimSurfaceState[playerid] = BUILD_DEMO_AIM_SURFACE_NONE;
		return false;
	}

	return true;
}

stock bool:GetBuildAimHit(playerid, &Float:x, &Float:y, &Float:z)
{
	if (!IsBuildAimHitFresh(playerid))
	{
		return false;
	}

	x = gBuildAimHitX[playerid];
	y = gBuildAimHitY[playerid];
	z = gBuildAimHitZ[playerid];
	return true;
}

stock bool:GetBuildAimGroundRange(playerid, Float:fallbackGroundZ, &aimSurfaceState, &Float:minGroundZ, &Float:maxGroundZ)
{
	if (!IsBuildAimHitFresh(playerid))
	{
		aimSurfaceState = BUILD_DEMO_AIM_SURFACE_NONE;
		minGroundZ = fallbackGroundZ;
		maxGroundZ = fallbackGroundZ;
		return false;
	}

	aimSurfaceState = gBuildAimSurfaceState[playerid];
	minGroundZ = gBuildAimFootprintMinGroundZ[playerid];
	maxGroundZ = gBuildAimFootprintMaxGroundZ[playerid];
	if (minGroundZ > maxGroundZ)
	{
		new Float:tmp = minGroundZ;
		minGroundZ = maxGroundZ;
		maxGroundZ = tmp;
	}
	return true;
}

stock bool:ResolveBuildAimPoint(playerid, Float:zOffset, &Float:x, &Float:y, &Float:z, &Float:angle)
{
	new Float:fallbackX;
	new Float:fallbackY;
	new Float:fallbackZ;
	new bool:fallbackReady = ComputeAimPoint(playerid, zOffset, fallbackX, fallbackY, fallbackZ, angle);

	if (GetBuildAimHit(playerid, x, y, z))
	{
		if (!fallbackReady)
		{
			GetPlayerFacingAngle(playerid, angle);
		}
		return true;
	}

	if (!fallbackReady)
	{
		return false;
	}

	x = fallbackX;
	y = fallbackY;
	z = fallbackZ;
	return true;
}

stock bool:GetNearestFoundationCenterToPoint(playerid, Float:aimX, Float:aimY, &foundationIndex, &Float:centerX, &Float:centerY, &Float:centerZ, &Float:centerA, &Float:centerDistance)
{
	if (gBuildFoundationCount[playerid] <= 0)
	{
		return false;
	}

	new bestIndex = 0;
	new Float:bestDistance = 999999.0;
	for (new i = 0; i < BUILD_DEMO_MAX_OBJECTS; i++)
	{
		if (!gBuildFoundationActive[playerid][i])
		{
			continue;
		}

		new Float:distance = GetBuildDistance2D(aimX, aimY, gBuildFoundationGridX[playerid][i], gBuildFoundationGridY[playerid][i]);
		if (distance < bestDistance)
		{
			bestDistance = distance;
			bestIndex = i;
		}
	}

	foundationIndex = bestIndex;
	centerX = gBuildFoundationGridX[playerid][bestIndex];
	centerY = gBuildFoundationGridY[playerid][bestIndex];
	centerZ = gBuildFoundationGridZ[playerid][bestIndex];
	centerA = gBuildFoundationGridA[playerid][bestIndex];
	centerDistance = bestDistance;
	return true;
}

stock bool:GetNearestFoundationSnapToPoint(playerid, Float:aimX, Float:aimY, &parentIndex, &slotIndex, &occupied, &Float:snapX, &Float:snapY, &Float:snapZ, &Float:snapA, &Float:snapDistance)
{
	if (gBuildFoundationCount[playerid] <= 0)
	{
		return false;
	}

	new bestParent = -1;
	new bestSlot = -1;
	new Float:bestDistance = 999999.0;
	new Float:bestX = 0.0;
	new Float:bestY = 0.0;
	new Float:bestZ = 0.0;
	new Float:bestA = 0.0;
	new bool:bestOccupied = false;

	for (new i = 0; i < BUILD_DEMO_MAX_OBJECTS; i++)
	{
		if (!gBuildFoundationActive[playerid][i])
		{
			continue;
		}

		for (new slot = 0; slot < 4; slot++)
		{
			new Float:testA = NormalizeBuildAngle(gBuildFoundationGridA[playerid][i] + float(slot * 90));
			new Float:testX, Float:testY;
			GetForwardPoint(gBuildFoundationGridX[playerid][i], gBuildFoundationGridY[playerid][i], testA, BUILD_DEMO_FOUNDATION_SIZE, testX, testY);

			new Float:distance = GetBuildDistance2D(aimX, aimY, testX, testY);
			if (distance < bestDistance)
			{
				bestDistance = distance;
				bestParent = i;
				bestSlot = slot;
				bestX = testX;
				bestY = testY;
				bestZ = gBuildFoundationGridZ[playerid][i];
				bestA = gBuildFoundationGridA[playerid][i];
				bestOccupied = IsBuildFoundationSlotOccupied(playerid, testX, testY);
			}
		}
	}

	if (bestParent == -1 || bestDistance > BUILD_DEMO_FOUNDATION_SNAP_RADIUS)
	{
		return false;
	}

	parentIndex = bestParent;
	slotIndex = bestSlot;
	occupied = bestOccupied ? 1 : 0;
	snapX = bestX;
	snapY = bestY;
	snapZ = bestZ;
	snapA = bestA;
	snapDistance = bestDistance;
	return true;
}

stock bool:GetNearestFoundationEdgeToPoint(playerid, Float:aimX, Float:aimY, &foundationIndex, &edgeIndex, &Float:edgeX, &Float:edgeY, &Float:edgeZ, &Float:edgeA, &Float:edgeDistance)
{
	if (gBuildFoundationCount[playerid] <= 0)
	{
		return false;
	}

	new bestFoundation = 0;
	new bestEdge = 0;
	new Float:bestDistance = 999999.0;
	for (new foundation = 0; foundation < BUILD_DEMO_MAX_OBJECTS; foundation++)
	{
		if (!gBuildFoundationActive[playerid][foundation])
		{
			continue;
		}

		for (new i = 0; i < 4; i++)
		{
			new Float:testA = NormalizeBuildAngle(gBuildFoundationGridA[playerid][foundation] + float(i * 90));
			new Float:testX, Float:testY;
			GetForwardPoint(gBuildFoundationGridX[playerid][foundation], gBuildFoundationGridY[playerid][foundation], testA, BUILD_DEMO_FOUNDATION_HALF, testX, testY);

			new Float:distance = GetBuildDistance2D(aimX, aimY, testX, testY);
			if (distance < bestDistance)
			{
				bestDistance = distance;
				bestFoundation = foundation;
				bestEdge = i;
				edgeX = testX;
				edgeY = testY;
				edgeZ = gBuildFoundationGridZ[playerid][foundation];
				edgeA = testA;
			}
		}
	}

	foundationIndex = bestFoundation;
	edgeIndex = bestEdge;
	edgeDistance = bestDistance;
	return true;
}

stock bool:GetNearestFoundationEdgeOnFoundationToPoint(playerid, foundationIndex, Float:aimX, Float:aimY, &edgeIndex, &Float:edgeX, &Float:edgeY, &Float:edgeZ, &Float:edgeA, &Float:edgeDistance)
{
	if (foundationIndex < 0 || foundationIndex >= BUILD_DEMO_MAX_OBJECTS || !gBuildFoundationActive[playerid][foundationIndex])
	{
		return false;
	}

	new bestEdge = 0;
	new Float:bestDistance = 999999.0;
	for (new i = 0; i < 4; i++)
	{
		new Float:testA = NormalizeBuildAngle(gBuildFoundationGridA[playerid][foundationIndex] + float(i * 90));
		new Float:testX, Float:testY;
		GetForwardPoint(gBuildFoundationGridX[playerid][foundationIndex], gBuildFoundationGridY[playerid][foundationIndex], testA, BUILD_DEMO_FOUNDATION_HALF, testX, testY);

		new Float:distance = GetBuildDistance2D(aimX, aimY, testX, testY);
		if (distance < bestDistance)
		{
			bestDistance = distance;
			bestEdge = i;
			edgeX = testX;
			edgeY = testY;
			edgeZ = gBuildFoundationGridZ[playerid][foundationIndex];
			edgeA = testA;
		}
	}

	edgeIndex = bestEdge;
	edgeDistance = bestDistance;
	return true;
}

stock bool:ComputeBuildPreview(playerid, partid, rotationStep, bool:flipped, &modelid, &Float:x, &Float:y, &Float:z, &Float:rx, &Float:ry, &Float:rz)
{
	new stateRotationStep = GetBuildRotationStepForPart(partid, rotationStep);
	modelid = GetBuildPartModel(partid);
	if (modelid == 0)
	{
		SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_NONE, "Unknown build part.");
		return false;
	}

	gBuildPreviewFoundationHeightStep[playerid] = 0;
	rx = 0.0;
	ry = 0.0;
	rz = 0.0;
	SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_NONE, "");
	if (partid != SAMPP_BUILD_PART_FOUNDATION)
	{
		ResetBuildFoundationSnapHeightLock(playerid);
	}

	new Float:aimX, Float:aimY, Float:aimZ, Float:playerAngle;
	if (!ResolveBuildAimPoint(playerid, BUILD_DEMO_FOUNDATION_Z_OFFSET, aimX, aimY, aimZ, playerAngle))
	{
		SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_NONE, "Could not compute camera aim point.");
		return false;
	}

	switch (partid)
	{
		case SAMPP_BUILD_PART_FOUNDATION:
		{
			new aimSurfaceState;
			new Float:footprintMinGroundZ;
			new Float:footprintMaxGroundZ;
			new bool:groundRangeReliable = GetBuildAimGroundRange(playerid, aimZ, aimSurfaceState, footprintMinGroundZ, footprintMaxGroundZ);
			if (gBuildFoundationCount[playerid] == 0)
			{
				ApplyBuildFirstFoundationAimLock(playerid, aimX, aimY, aimZ, aimSurfaceState, footprintMinGroundZ, footprintMaxGroundZ);
			}
			else
			{
				ResetBuildFirstFoundationAimLock(playerid);
			}
			if (aimSurfaceState == BUILD_DEMO_AIM_SURFACE_BLOCKED_NON_GROUND)
			{
				x = aimX;
				y = aimY;
				z = footprintMaxGroundZ + BUILD_DEMO_FOUNDATION_TOP_CLEARANCE;
				rz = GetBuildGridAngle(playerAngle);
				new Float:blockedRequiredHeight;
				TryGetBuildFoundationAutoHeightStepFromRange(z, footprintMinGroundZ, footprintMaxGroundZ, gBuildPreviewFoundationHeightStep[playerid], blockedRequiredHeight);
				modelid = GetBuildFoundationModel(gBuildPreviewFoundationHeightStep[playerid]);
				SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_NONE, "Aim at terrain ground, not a wall or object.");
				return true;
			}

			if (gBuildFoundationCount[playerid] > 0)
			{
				new parentIndex;
				new slotIndex;
				new occupied;
				new Float:snapA;
				new Float:snapDistance;
				if (GetNearestFoundationSnapToPoint(playerid, aimX, aimY, parentIndex, slotIndex, occupied, x, y, z, snapA, snapDistance))
				{
					rz = NormalizeBuildAngle(snapA);
					if (occupied)
					{
						ResetBuildFoundationSnapHeightLock(playerid);
						modelid = GetBuildFoundationModel(gBuildPreviewFoundationHeightStep[playerid]);
						SetBuildPreviewCandidate(playerid, false, parentIndex, slotIndex, BUILD_DEMO_SLOT_NONE, "That foundation neighbour slot is already occupied.");
						return true;
					}

					if (gBuildFoundationSnapHeightLocked[playerid]
						&& gBuildFoundationSnapHeightParent[playerid] == parentIndex
						&& gBuildFoundationSnapHeightSlot[playerid] == slotIndex)
					{
						gBuildPreviewFoundationHeightStep[playerid] = gBuildFoundationSnapHeightStep[playerid];
						modelid = GetBuildFoundationModel(gBuildPreviewFoundationHeightStep[playerid]);
						SetBuildPreviewCandidate(playerid, true, parentIndex, slotIndex, BUILD_DEMO_SLOT_NONE, "");
						return true;
					}

					new Float:requiredHeight;
					new Float:snapMinGroundZ = footprintMinGroundZ;
					new Float:snapMaxGroundZ = footprintMaxGroundZ;
					new bool:terrainIntersects = groundRangeReliable && snapMaxGroundZ > z + BUILD_DEMO_FOUNDATION_TOP_CLEARANCE + BUILD_DEMO_FOUNDATION_SNAP_TERRAIN_EPSILON;
					if (!terrainIntersects && snapMaxGroundZ > z - BUILD_DEMO_FOUNDATION_TOP_CLEARANCE)
					{
						snapMaxGroundZ = z - BUILD_DEMO_FOUNDATION_TOP_CLEARANCE;
					}
					if (snapMinGroundZ > snapMaxGroundZ)
					{
						snapMinGroundZ = snapMaxGroundZ;
					}
					if ((z - snapMinGroundZ) + BUILD_DEMO_FOUNDATION_GROUND_EMBED > BUILD_DEMO_FOUNDATION_MAX_HEIGHT + 0.001)
					{
						snapMinGroundZ = aimZ;
					}
					new bool:heightOk = !terrainIntersects && TryGetBuildFoundationAutoHeightStepFromRange(z, snapMinGroundZ, snapMaxGroundZ, gBuildPreviewFoundationHeightStep[playerid], requiredHeight);
					modelid = GetBuildFoundationModel(gBuildPreviewFoundationHeightStep[playerid]);
					if (terrainIntersects)
					{
						ResetBuildFoundationSnapHeightLock(playerid);
						SetBuildPreviewCandidate(playerid, false, parentIndex, slotIndex, BUILD_DEMO_SLOT_NONE, "Foundation level intersects terrain here.");
					}
					else if (!heightOk)
					{
						ResetBuildFoundationSnapHeightLock(playerid);
						SetBuildPreviewCandidate(playerid, false, parentIndex, slotIndex, BUILD_DEMO_SLOT_NONE, "Ground is too far below this foundation level.");
					}
					else
					{
						gBuildFoundationSnapHeightLocked[playerid] = true;
						gBuildFoundationSnapHeightParent[playerid] = parentIndex;
						gBuildFoundationSnapHeightSlot[playerid] = slotIndex;
						gBuildFoundationSnapHeightStep[playerid] = gBuildPreviewFoundationHeightStep[playerid];
						SetBuildPreviewCandidate(playerid, true, parentIndex, slotIndex, BUILD_DEMO_SLOT_NONE, "");
					}
					return true;
				}

				ResetBuildFoundationSnapHeightLock(playerid);
				x = aimX;
				y = aimY;
				z = gBuildFoundationZ[playerid];
				rz = GetBuildGridAngle(playerAngle);
				new Float:requiredHeight;
				TryGetBuildFoundationAutoHeightStepFromRange(z, footprintMinGroundZ, footprintMaxGroundZ, gBuildPreviewFoundationHeightStep[playerid], requiredHeight);
				modelid = GetBuildFoundationModel(gBuildPreviewFoundationHeightStep[playerid]);
				SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_NONE, "Aim near an empty foundation neighbour slot.");
				return true;
			}

			ResetBuildFoundationSnapHeightLock(playerid);
			new Float:lift = GetBuildFoundationLiftFromStep(stateRotationStep);
			x = aimX;
			y = aimY;
			z = aimZ + lift;
			new Float:minTopZ = footprintMaxGroundZ + BUILD_DEMO_FOUNDATION_TOP_CLEARANCE;
			if (z < minTopZ)
			{
				z = minTopZ;
			}
			rz = GetBuildGridAngle(playerAngle);
			new Float:requiredHeight;
			new bool:heightOk = TryGetBuildFoundationAutoHeightStepFromRange(z, footprintMinGroundZ, footprintMaxGroundZ, gBuildPreviewFoundationHeightStep[playerid], requiredHeight);
			modelid = GetBuildFoundationModel(gBuildPreviewFoundationHeightStep[playerid]);
			if (!heightOk)
			{
				SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_NONE, "Ground is too far below this foundation level.");
			}
			else
			{
				SetBuildPreviewCandidate(playerid, true, -1, -1, BUILD_DEMO_SLOT_NONE, "");
			}
			return true;
		}
		case SAMPP_BUILD_PART_WALL, SAMPP_BUILD_PART_DOORFRAME:
		{
			new foundationIndex;
			new edgeIndex;
			new Float:edgeA;
			new Float:edgeDistance;
			if (!GetNearestFoundationEdgeToPoint(playerid, aimX, aimY, foundationIndex, edgeIndex, x, y, z, edgeA, edgeDistance))
			{
				x = aimX;
				y = aimY;
				z = aimZ + BUILD_DEMO_WALL_HEIGHT;
				rz = GetBuildEdgePieceYaw(partid, GetBuildGridAngle(playerAngle), flipped);
				SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_EDGE, "Wall pieces require a foundation edge.");
				return true;
			}

			z += BUILD_DEMO_WALL_HEIGHT;
			rz = GetBuildEdgePieceYaw(partid, edgeA, flipped);
			if (edgeDistance > BUILD_DEMO_EDGE_SNAP_RADIUS)
			{
				SetBuildPreviewCandidate(playerid, false, foundationIndex, edgeIndex, BUILD_DEMO_SLOT_EDGE, "Aim closer to a foundation edge.");
				return true;
			}
			if (gBuildFoundationEdgePiece[playerid][foundationIndex][edgeIndex] != BUILD_DEMO_PIECE_NONE)
			{
				SetBuildPreviewCandidate(playerid, false, foundationIndex, edgeIndex, BUILD_DEMO_SLOT_EDGE, "That foundation edge already has a wall or door frame.");
				return true;
			}
			SetBuildPreviewCandidate(playerid, true, foundationIndex, edgeIndex, BUILD_DEMO_SLOT_EDGE, "");
			return true;
		}
		case SAMPP_BUILD_PART_FLOOR, SAMPP_BUILD_PART_ROOF:
		{
			new foundationIndex;
			new Float:centerA;
			new Float:centerDistance;
			if (!GetNearestFoundationCenterToPoint(playerid, aimX, aimY, foundationIndex, x, y, z, centerA, centerDistance))
			{
				x = aimX;
				y = aimY;
				z = aimZ + BUILD_DEMO_ROOF_HEIGHT;
				rz = GetBuildGridAngle(playerAngle);
				SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_TOP, "Floors and roofs require a foundation.");
				return true;
			}

			z += BUILD_DEMO_ROOF_HEIGHT;
			rz = NormalizeBuildAngle(centerA);
			if (centerDistance > BUILD_DEMO_CENTER_SNAP_RADIUS)
			{
				SetBuildPreviewCandidate(playerid, false, foundationIndex, 0, BUILD_DEMO_SLOT_TOP, "Aim closer to the foundation center.");
				return true;
			}
			if (gBuildFoundationTopPiece[playerid][foundationIndex] != BUILD_DEMO_PIECE_NONE)
			{
				SetBuildPreviewCandidate(playerid, false, foundationIndex, 0, BUILD_DEMO_SLOT_TOP, "That foundation already has a floor or roof slot occupied.");
				return true;
			}
			SetBuildPreviewCandidate(playerid, true, foundationIndex, 0, BUILD_DEMO_SLOT_TOP, "");
			return true;
		}
		case SAMPP_BUILD_PART_STAIRS:
		{
			new foundationIndex;
			new Float:centerA;
			new Float:centerDistance;
			if (!GetNearestFoundationCenterToPoint(playerid, aimX, aimY, foundationIndex, x, y, z, centerA, centerDistance))
			{
				x = aimX;
				y = aimY;
				z = aimZ - BUILD_DEMO_FOUNDATION_Z_OFFSET;
				rz = GetBuildGridAngle(playerAngle);
				SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_STAIRS, "Stairs require a foundation.");
				return true;
			}

			new stairEdgeIndex;
			new Float:stairEdgeX;
			new Float:stairEdgeY;
			new Float:stairEdgeZ;
			new Float:stairEdgeA;
			new Float:stairEdgeDistance;
			if (!GetNearestFoundationEdgeOnFoundationToPoint(playerid, foundationIndex, aimX, aimY, stairEdgeIndex, stairEdgeX, stairEdgeY, stairEdgeZ, stairEdgeA, stairEdgeDistance))
			{
				stairEdgeA = centerA;
			}

			z += 0.1;
			rz = NormalizeBuildAngle(stairEdgeA + BUILD_DEMO_STAIRS_YAW_OFFSET);
			if (centerDistance > BUILD_DEMO_CENTER_SNAP_RADIUS)
			{
				SetBuildPreviewCandidate(playerid, false, foundationIndex, 0, BUILD_DEMO_SLOT_STAIRS, "Aim closer to the foundation center.");
				return true;
			}
			if (gBuildFoundationStairsPiece[playerid][foundationIndex] != BUILD_DEMO_PIECE_NONE)
			{
				SetBuildPreviewCandidate(playerid, false, foundationIndex, 0, BUILD_DEMO_SLOT_STAIRS, "That foundation already has stairs.");
				return true;
			}
			SetBuildPreviewCandidate(playerid, true, foundationIndex, 0, BUILD_DEMO_SLOT_STAIRS, "");
			return true;
		}
		case SAMPP_BUILD_PART_DOOR:
		{
			new foundationIndex;
			new edgeIndex;
			new Float:edgeA;
			new Float:edgeDistance;
			if (!GetNearestFoundationEdgeToPoint(playerid, aimX, aimY, foundationIndex, edgeIndex, x, y, z, edgeA, edgeDistance))
			{
				FitBuildDoorToFrame(aimX, aimY, aimZ, GetBuildGridAngle(playerAngle), flipped, x, y, z, rz);
				SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_DOOR, "Doors require a placed door frame.");
				return true;
			}

			FitBuildDoorToFrame(x, y, z, edgeA, flipped, x, y, z, rz);
			if (edgeDistance > BUILD_DEMO_EDGE_SNAP_RADIUS)
			{
				SetBuildPreviewCandidate(playerid, false, foundationIndex, edgeIndex, BUILD_DEMO_SLOT_DOOR, "Aim closer to a door frame.");
				return true;
			}
			if (gBuildFoundationEdgePiece[playerid][foundationIndex][edgeIndex] != BUILD_DEMO_PIECE_DOORFRAME)
			{
				SetBuildPreviewCandidate(playerid, false, foundationIndex, edgeIndex, BUILD_DEMO_SLOT_DOOR, "Door placement requires a door frame in this edge.");
				return true;
			}
			if (gBuildFoundationEdgeDoor[playerid][foundationIndex][edgeIndex])
			{
				SetBuildPreviewCandidate(playerid, false, foundationIndex, edgeIndex, BUILD_DEMO_SLOT_DOOR, "That door frame already has a door.");
				return true;
			}
			SetBuildPreviewCandidate(playerid, true, foundationIndex, edgeIndex, BUILD_DEMO_SLOT_DOOR, "");
			return true;
		}
	}

	return false;
}

stock RefreshBuildPreview(playerid, bool:force = false)
{
	if (!gBuildActive[playerid] || gBuildSelectedPart[playerid] == 0 || gBuildSelectedPart[playerid] == SAMPP_BUILD_PART_REMOVE)
	{
		DestroyBuildPreview(playerid);
		return 0;
	}

	new now = GetTickCount();
	if (!force && now < gBuildPreviewNextUpdate[playerid])
	{
		return gBuildPreviewValid[playerid] ? 1 : 0;
	}
	gBuildPreviewNextUpdate[playerid] = now + BUILD_DEMO_PREVIEW_MS;

	new modelid;
	new Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz;
	if (!ComputeBuildPreview(playerid, gBuildSelectedPart[playerid], gBuildRotationStep[playerid], gBuildFlipped[playerid], modelid, x, y, z, rx, ry, rz))
	{
		DestroyBuildPreview(playerid);
		return 0;
	}

	new previewModelId = GetBuildPreviewPartModelForState(gBuildSelectedPart[playerid], gBuildPreviewPlaceable[playerid], gBuildPreviewFoundationHeightStep[playerid]);
	if (gBuildPreviewObject[playerid] == INVALID_OBJECT_ID || gBuildPreviewModel[playerid] != previewModelId)
	{
		DestroyBuildPreview(playerid);
		gBuildPreviewObject[playerid] = CreatePlayerObject(playerid, previewModelId, x, y, z, rx, ry, rz, 80.0);
		gBuildPreviewModel[playerid] = previewModelId;
	}
	else
	{
		SetPlayerObjectPos(playerid, gBuildPreviewObject[playerid], x, y, z);
		SetPlayerObjectRot(playerid, gBuildPreviewObject[playerid], rx, ry, rz);
	}

	gBuildPreviewValid[playerid] = gBuildPreviewObject[playerid] != INVALID_OBJECT_ID;
	gBuildPreviewX[playerid] = x;
	gBuildPreviewY[playerid] = y;
	gBuildPreviewZ[playerid] = z;
	gBuildPreviewRX[playerid] = rx;
	gBuildPreviewRY[playerid] = ry;
	gBuildPreviewRZ[playerid] = rz;
	return gBuildPreviewValid[playerid] ? 1 : 0;
}

stock OpenBuildDemo(playerid)
{
	if (!SAMPP_BuildHasUI(playerid))
	{
		SendClientMessage(playerid, BUILD_DEMO_WARN_COLOUR, "[BuildDemo] OpenMP-Plus build UI is not available on this client.");
		return 1;
	}

	if (gBuildActive[playerid])
	{
		CloseBuildDemo(playerid);
	}

	gBuildSession[playerid] = BuildDemoSession(playerid);
	gBuildActive[playerid] = true;
	ResetBuildPreviewState(playerid);
	DestroyBuildPreview(playerid);

	if (!SAMPP_BuildOpen(playerid, gBuildSession[playerid], "Build Demo", 10.0))
	{
		SendClientMessage(playerid, BUILD_DEMO_WARN_COLOUR, "[BuildDemo] Could not open build UI.");
		return 1;
	}

	SAMPP_BuildClearParts(playerid);
	SAMPP_BuildAddPart(playerid, SAMPP_BUILD_PART_FOUNDATION, BUILD_MODEL_FOUNDATION, "Foundation", "Structure", "100 wood");
	SAMPP_BuildAddPart(playerid, SAMPP_BUILD_PART_WALL, BUILD_MODEL_WALL, "Wall", "Structure", "80 wood");
	SAMPP_BuildAddPart(playerid, SAMPP_BUILD_PART_DOORFRAME, BUILD_MODEL_DOORFRAME, "Door Frame", "Structure", "90 wood");
	SAMPP_BuildAddPart(playerid, SAMPP_BUILD_PART_FLOOR, BUILD_MODEL_FLOOR, "Floor/Ceiling", "Structure", "120 wood");
	SAMPP_BuildAddPart(playerid, SAMPP_BUILD_PART_ROOF, BUILD_MODEL_ROOF, "Roof", "Structure", "140 wood");
	SAMPP_BuildAddPart(playerid, SAMPP_BUILD_PART_STAIRS, BUILD_MODEL_STAIRS, "Stairs", "Access", "75 wood");
	SAMPP_BuildAddPart(playerid, SAMPP_BUILD_PART_DOOR, BUILD_MODEL_DOOR, "Door", "Access", "50 wood");
	SAMPP_BuildAddPart(playerid, SAMPP_BUILD_PART_REMOVE, 0, "Remove", "Tools", "orange");

	SendClientMessage(playerid, BUILD_DEMO_OK_COLOUR, "[BuildDemo] Build UI opened. Select a part to show a live preview.");
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] LMB confirms. MMB switches side for walls/doors. Remove mode deletes targeted placed parts.");
	return 1;
}

stock CloseBuildDemo(playerid)
{
	gBuildActive[playerid] = false;
	SendBuildRemoveFocus(playerid, false);
	DestroyBuildPreview(playerid);
	ResetBuildPreviewState(playerid);
	SAMPP_BuildClose(playerid);
	return 1;
}

stock bool:CommitBuildDemoSlot(playerid, partid, &foundationOut, &slotKindOut, &slotOut)
{
	new parent = gBuildPreviewParentFoundation[playerid];
	new slot = gBuildPreviewSlotIndex[playerid];
	foundationOut = -1;
	slotKindOut = gBuildPreviewSlotKind[playerid];
	slotOut = slot;

	switch (partid)
	{
		case SAMPP_BUILD_PART_FOUNDATION:
		{
			slotKindOut = BUILD_DEMO_SLOT_NONE;
			slotOut = -1;
			return RegisterBuildFoundation(playerid, gBuildPreviewX[playerid], gBuildPreviewY[playerid], gBuildPreviewZ[playerid], gBuildPreviewRZ[playerid], foundationOut);
		}
		case SAMPP_BUILD_PART_WALL:
		{
			if (parent < 0 || parent >= BUILD_DEMO_MAX_OBJECTS || !gBuildFoundationActive[playerid][parent] || slot < 0 || slot >= 4)
			{
				return false;
			}
			if (gBuildFoundationEdgePiece[playerid][parent][slot] != BUILD_DEMO_PIECE_NONE)
			{
				return false;
			}
			gBuildFoundationEdgePiece[playerid][parent][slot] = BUILD_DEMO_PIECE_WALL;
			foundationOut = parent;
			slotKindOut = BUILD_DEMO_SLOT_EDGE;
			slotOut = slot;
			return true;
		}
		case SAMPP_BUILD_PART_DOORFRAME:
		{
			if (parent < 0 || parent >= BUILD_DEMO_MAX_OBJECTS || !gBuildFoundationActive[playerid][parent] || slot < 0 || slot >= 4)
			{
				return false;
			}
			if (gBuildFoundationEdgePiece[playerid][parent][slot] != BUILD_DEMO_PIECE_NONE)
			{
				return false;
			}
			gBuildFoundationEdgePiece[playerid][parent][slot] = BUILD_DEMO_PIECE_DOORFRAME;
			foundationOut = parent;
			slotKindOut = BUILD_DEMO_SLOT_EDGE;
			slotOut = slot;
			return true;
		}
		case SAMPP_BUILD_PART_FLOOR:
		{
			if (parent < 0 || parent >= BUILD_DEMO_MAX_OBJECTS || !gBuildFoundationActive[playerid][parent])
			{
				return false;
			}
			if (gBuildFoundationTopPiece[playerid][parent] != BUILD_DEMO_PIECE_NONE)
			{
				return false;
			}
			gBuildFoundationTopPiece[playerid][parent] = BUILD_DEMO_PIECE_FLOOR;
			foundationOut = parent;
			slotKindOut = BUILD_DEMO_SLOT_TOP;
			slotOut = 0;
			return true;
		}
		case SAMPP_BUILD_PART_ROOF:
		{
			if (parent < 0 || parent >= BUILD_DEMO_MAX_OBJECTS || !gBuildFoundationActive[playerid][parent])
			{
				return false;
			}
			if (gBuildFoundationTopPiece[playerid][parent] != BUILD_DEMO_PIECE_NONE)
			{
				return false;
			}
			gBuildFoundationTopPiece[playerid][parent] = BUILD_DEMO_PIECE_ROOF;
			foundationOut = parent;
			slotKindOut = BUILD_DEMO_SLOT_TOP;
			slotOut = 0;
			return true;
		}
		case SAMPP_BUILD_PART_STAIRS:
		{
			if (parent < 0 || parent >= BUILD_DEMO_MAX_OBJECTS || !gBuildFoundationActive[playerid][parent])
			{
				return false;
			}
			if (gBuildFoundationStairsPiece[playerid][parent] != BUILD_DEMO_PIECE_NONE)
			{
				return false;
			}
			gBuildFoundationStairsPiece[playerid][parent] = BUILD_DEMO_PIECE_STAIRS;
			foundationOut = parent;
			slotKindOut = BUILD_DEMO_SLOT_STAIRS;
			slotOut = 0;
			return true;
		}
		case SAMPP_BUILD_PART_DOOR:
		{
			if (parent < 0 || parent >= BUILD_DEMO_MAX_OBJECTS || !gBuildFoundationActive[playerid][parent] || slot < 0 || slot >= 4)
			{
				return false;
			}
			if (gBuildFoundationEdgePiece[playerid][parent][slot] != BUILD_DEMO_PIECE_DOORFRAME || gBuildFoundationEdgeDoor[playerid][parent][slot])
			{
				return false;
			}
			gBuildFoundationEdgeDoor[playerid][parent][slot] = true;
			foundationOut = parent;
			slotKindOut = BUILD_DEMO_SLOT_DOOR;
			slotOut = slot;
			return true;
		}
	}

	return false;
}

stock bool:GetBuildHorizontalSurfaceHit(partid, Float:objectX, Float:objectY, Float:objectZ, Float:objectRZ, Float:camX, Float:camY, Float:camZ, Float:frontX, Float:frontY, Float:frontZ, &Float:distance)
{
	switch (partid)
	{
		case SAMPP_BUILD_PART_FOUNDATION, SAMPP_BUILD_PART_FLOOR, SAMPP_BUILD_PART_ROOF:
		{
			if (floatabs(frontZ) <= 0.01)
			{
				return false;
			}

			new Float:t = (objectZ - camZ) / frontZ;
			if (t < 0.35 || t > BUILD_DEMO_REMOVE_MAX_RAY_DISTANCE)
			{
				return false;
			}

			new Float:hitX = camX + (frontX * t);
			new Float:hitY = camY + (frontY * t);
			new Float:dx = hitX - objectX;
			new Float:dy = hitY - objectY;
			new Float:axisX = floatsin(-objectRZ, degrees);
			new Float:axisY = floatcos(-objectRZ, degrees);
			new Float:rightX = floatcos(-objectRZ, degrees);
			new Float:rightY = -floatsin(-objectRZ, degrees);
			new Float:localA = (dx * axisX) + (dy * axisY);
			new Float:localB = (dx * rightX) + (dy * rightY);
			new Float:half = BUILD_DEMO_FOUNDATION_HALF + 0.22;

			if (floatabs(localA) > half || floatabs(localB) > half)
			{
				return false;
			}

			distance = t;
			return true;
		}
	}
	return false;
}

stock bool:GetBuildVerticalSurfaceHit(partid, Float:objectX, Float:objectY, Float:objectZ, Float:objectRZ, Float:camX, Float:camY, Float:camZ, Float:frontX, Float:frontY, Float:frontZ, &Float:distance)
{
	switch (partid)
	{
		case SAMPP_BUILD_PART_WALL, SAMPP_BUILD_PART_DOORFRAME, SAMPP_BUILD_PART_DOOR:
		{
			new Float:normalA = NormalizeBuildAngle(objectRZ - 90.0);
			new Float:normalX = floatsin(-normalA, degrees);
			new Float:normalY = floatcos(-normalA, degrees);
			new Float:denom = (frontX * normalX) + (frontY * normalY);
			if (floatabs(denom) <= 0.01)
			{
				return false;
			}

			new Float:t = (((objectX - camX) * normalX) + ((objectY - camY) * normalY)) / denom;
			if (t < 0.35 || t > BUILD_DEMO_REMOVE_MAX_RAY_DISTANCE)
			{
				return false;
			}

			new Float:hitX = camX + (frontX * t);
			new Float:hitY = camY + (frontY * t);
			new Float:hitZ = camZ + (frontZ * t);
			new Float:dx = hitX - objectX;
			new Float:dy = hitY - objectY;
			new Float:widthX = floatsin(-objectRZ, degrees);
			new Float:widthY = floatcos(-objectRZ, degrees);
			new Float:localWidth = (dx * widthX) + (dy * widthY);
			new Float:localHeight = hitZ - objectZ;
			new Float:minWidth = -1.72;
			new Float:maxWidth = 1.72;
			new Float:minHeight = -1.75;
			new Float:maxHeight = 1.95;
			if (partid == SAMPP_BUILD_PART_DOOR)
			{
				minWidth = -0.12;
				maxWidth = BUILD_DEMO_DOOR_WIDTH + 0.12;
				minHeight = -1.55;
				maxHeight = 1.05;
			}

			if (localWidth < minWidth || localWidth > maxWidth || localHeight < minHeight || localHeight > maxHeight)
			{
				return false;
			}

			distance = t;
			return true;
		}
	}
	return false;
}

stock bool:FindBuildObjectFromAim(playerid, &objectIndex, &Float:targetDistance)
{
	new Float:px, Float:py, Float:pz;
	new Float:camX, Float:camY, Float:camZ;
	new Float:frontX, Float:frontY, Float:frontZ;
	if (!GetPlayerPos(playerid, px, py, pz)
		|| !GetPlayerCameraPos(playerid, camX, camY, camZ)
		|| !GetPlayerCameraFrontVector(playerid, frontX, frontY, frontZ))
	{
		return false;
	}

	new Float:frontLen = floatsqroot((frontX * frontX) + (frontY * frontY) + (frontZ * frontZ));
	if (frontLen <= 0.001)
	{
		return false;
	}

	frontX /= frontLen;
	frontY /= frontLen;
	frontZ /= frontLen;

	new bestIndex = -1;
	new bestPriority = 99;
	new Float:bestDistance = 999999.0;

	for (new i = 0; i < BUILD_DEMO_MAX_OBJECTS; i++)
	{
		if (gBuildObjects[playerid][i] == INVALID_OBJECT_ID)
		{
			continue;
		}

		new Float:playerDistance = GetBuildDistance2D(px, py, gBuildObjectX[playerid][i], gBuildObjectY[playerid][i]);
		if (playerDistance > BUILD_DEMO_REMOVE_MAX_PLAYER_DISTANCE)
		{
			continue;
		}

		new partid = gBuildObjectPart[playerid][i];
		new Float:objectRZ = GetBuildObjectCurrentRZ(playerid, i);
		new Float:surfaceDistance;
		if (!GetBuildHorizontalSurfaceHit(partid, gBuildObjectX[playerid][i], gBuildObjectY[playerid][i], gBuildObjectZ[playerid][i], objectRZ, camX, camY, camZ, frontX, frontY, frontZ, surfaceDistance)
			&& !GetBuildVerticalSurfaceHit(partid, gBuildObjectX[playerid][i], gBuildObjectY[playerid][i], gBuildObjectZ[playerid][i], objectRZ, camX, camY, camZ, frontX, frontY, frontZ, surfaceDistance)
			&& !RayIntersectsBuildRemoveOBB(partid, gBuildObjectX[playerid][i], gBuildObjectY[playerid][i], gBuildObjectZ[playerid][i], objectRZ, camX, camY, camZ, frontX, frontY, frontZ, surfaceDistance, gBuildObjectHeightStep[playerid][i]))
		{
			continue;
		}

		new priority = GetBuildRemovePriority(partid);
		if (surfaceDistance + BUILD_DEMO_REMOVE_PRIORITY_EPSILON < bestDistance
			|| (floatabs(surfaceDistance - bestDistance) <= BUILD_DEMO_REMOVE_PRIORITY_EPSILON && priority < bestPriority))
		{
			bestPriority = priority;
			bestIndex = i;
			bestDistance = surfaceDistance;
		}
	}

	if (bestIndex == -1)
	{
		return false;
	}

	objectIndex = bestIndex;
	targetDistance = bestDistance;
	return true;
}

stock EndBuildDoorInteraction(playerid)
{
	if (gBuildDoorTargetActive[playerid])
	{
		if (IsUsingSAMPP(playerid))
		{
			SAMPP_TargetClear(playerid);
		}
	}
	gBuildDoorFocusedObject[playerid] = -1;
	gBuildDoorTargetActive[playerid] = false;
	gBuildDoorTargetNextPublish[playerid] = 0;
	gBuildDoorTargetClearAfter[playerid] = 0;
	return 1;
}

stock ResetBuildDoorTargetState(playerid)
{
	gBuildDoorFocusedObject[playerid] = -1;
	gBuildDoorTargetActive[playerid] = false;
	gBuildDoorTargetNextPublish[playerid] = 0;
	gBuildDoorTargetClearAfter[playerid] = 0;
	return 1;
}

stock bool:GetBuildDoorInteractionPoseHit(playerid, objectIndex, Float:objectRZ, Float:camX, Float:camY, Float:camZ, Float:frontX, Float:frontY, Float:frontZ, &Float:surfaceDistance)
{
	if (!GetBuildVerticalSurfaceHit(SAMPP_BUILD_PART_DOOR, gBuildObjectX[playerid][objectIndex], gBuildObjectY[playerid][objectIndex], gBuildObjectZ[playerid][objectIndex], objectRZ, camX, camY, camZ, frontX, frontY, frontZ, surfaceDistance)
		&& !RayIntersectsBuildRemoveOBB(SAMPP_BUILD_PART_DOOR, gBuildObjectX[playerid][objectIndex], gBuildObjectY[playerid][objectIndex], gBuildObjectZ[playerid][objectIndex], objectRZ, camX, camY, camZ, frontX, frontY, frontZ, surfaceDistance))
	{
		return false;
	}

	return surfaceDistance <= BUILD_DEMO_DOOR_INTERACT_RAY_DISTANCE;
}

stock bool:FindBuildDoorFromAim(playerid, &objectIndex, &Float:targetDistance)
{
	new Float:px, Float:py, Float:pz;
	new Float:camX, Float:camY, Float:camZ;
	new Float:frontX, Float:frontY, Float:frontZ;
	if (!GetPlayerPos(playerid, px, py, pz)
		|| !GetPlayerCameraPos(playerid, camX, camY, camZ)
		|| !GetPlayerCameraFrontVector(playerid, frontX, frontY, frontZ))
	{
		return false;
	}

	new Float:frontLen = floatsqroot((frontX * frontX) + (frontY * frontY) + (frontZ * frontZ));
	if (frontLen <= 0.001)
	{
		return false;
	}

	frontX /= frontLen;
	frontY /= frontLen;
	frontZ /= frontLen;

	new bestIndex = -1;
	new Float:bestDistance = 999999.0;
	for (new i = 0; i < BUILD_DEMO_MAX_OBJECTS; i++)
	{
		if (gBuildObjects[playerid][i] == INVALID_OBJECT_ID || gBuildObjectPart[playerid][i] != SAMPP_BUILD_PART_DOOR)
		{
			continue;
		}

		if (GetPlayerDistanceFromPoint(playerid, gBuildObjectX[playerid][i], gBuildObjectY[playerid][i], gBuildObjectZ[playerid][i]) > BUILD_DEMO_DOOR_INTERACT_DISTANCE)
		{
			continue;
		}

		new Float:surfaceDistance;
		new bool:hit = GetBuildDoorInteractionPoseHit(playerid, i, gBuildObjectRZ[playerid][i], camX, camY, camZ, frontX, frontY, frontZ, surfaceDistance);
		if (gBuildDoorOpen[playerid][i])
		{
			new Float:openDistance;
			if (GetBuildDoorInteractionPoseHit(playerid, i, GetBuildObjectCurrentRZ(playerid, i), camX, camY, camZ, frontX, frontY, frontZ, openDistance)
				&& (!hit || openDistance < surfaceDistance))
			{
				surfaceDistance = openDistance;
				hit = true;
			}
		}

		if (!hit)
		{
			continue;
		}

		if (surfaceDistance < bestDistance)
		{
			bestDistance = surfaceDistance;
			bestIndex = i;
		}
	}

	if (bestIndex == -1)
	{
		return false;
	}

	objectIndex = bestIndex;
	targetDistance = bestDistance;
	return true;
}

stock RefreshBuildDoorInteraction(playerid, bool:force = false)
{
	if (gBuildActive[playerid] || !IsUsingSAMPP(playerid) || !SAMPP_HasFeature(playerid, SAMPP_FEATURE_TARGET) || IsPlayerInAnyVehicle(playerid))
	{
		return EndBuildDoorInteraction(playerid);
	}

	new now = GetTickCount();
	if (!force && now < gBuildDoorInteractNextUpdate[playerid])
	{
		return 1;
	}
	gBuildDoorInteractNextUpdate[playerid] = now + BUILD_DEMO_DOOR_INTERACT_MS;

	new objectIndex;
	new Float:targetDistance;
	if (!FindBuildDoorFromAim(playerid, objectIndex, targetDistance))
	{
		if (gBuildDoorTargetActive[playerid] && now < gBuildDoorTargetClearAfter[playerid])
		{
			return 1;
		}
		return EndBuildDoorInteraction(playerid);
	}

	new bool:focusChanged = gBuildDoorFocusedObject[playerid] != objectIndex;
	gBuildDoorFocusedObject[playerid] = objectIndex;
	gBuildDoorTargetActive[playerid] = true;
	gBuildDoorTargetClearAfter[playerid] = now + BUILD_DEMO_DOOR_TARGET_LOST_GRACE_MS;

	if (!force && !focusChanged && now < gBuildDoorTargetNextPublish[playerid])
	{
		return 1;
	}
	gBuildDoorTargetNextPublish[playerid] = now + BUILD_DEMO_DOOR_TARGET_REPUBLISH_MS;

	new targetid = BuildDoorTargetId(playerid);
	new action[32];
	format(action, sizeof action, "%s Door", gBuildDoorOpen[playerid][objectIndex] ? ("Close") : ("Open"));

	if (!SAMPP_TargetBeginDirect(playerid, targetid, SAMPP_TARGET_TYPE_OBJECT, action, BUILD_DEMO_DOOR_TARGET_TTL_MS))
	{
		return EndBuildDoorInteraction(playerid);
	}
	SAMPP_TargetSetLayout(playerid, targetid, SAMPP_TARGET_LAYOUT_MINIMAL);
	SAMPP_TargetAddAction(playerid, targetid, BUILD_DEMO_DOOR_OPTION_TOGGLE, action, "door");
	if (!SAMPP_TargetCommit(playerid, targetid))
	{
		return EndBuildDoorInteraction(playerid);
	}

	return 1;
}

stock ToggleBuildDoor(playerid, objectIndex)
{
	if (objectIndex < 0 || objectIndex >= BUILD_DEMO_MAX_OBJECTS
		|| gBuildObjects[playerid][objectIndex] == INVALID_OBJECT_ID
		|| gBuildObjectPart[playerid][objectIndex] != SAMPP_BUILD_PART_DOOR)
	{
		return 0;
	}

	new now = GetTickCount();
	if (now < gBuildDoorMovingUntil[playerid][objectIndex])
	{
		return 1;
	}

	new bool:open = !gBuildDoorOpen[playerid][objectIndex];
	new Float:targetRZ = NormalizeBuildAngle(gBuildObjectRZ[playerid][objectIndex] + (open ? BUILD_DEMO_DOOR_OPEN_ANGLE : 0.0));

	StopObject(gBuildObjects[playerid][objectIndex]);
	if (!MoveObject(
		gBuildObjects[playerid][objectIndex],
		gBuildObjectX[playerid][objectIndex],
		gBuildObjectY[playerid][objectIndex],
		gBuildObjectZ[playerid][objectIndex],
		BUILD_DEMO_DOOR_MOVE_SPEED,
		gBuildObjectRX[playerid][objectIndex],
		gBuildObjectRY[playerid][objectIndex],
		targetRZ))
	{
		SetObjectRot(gBuildObjects[playerid][objectIndex], gBuildObjectRX[playerid][objectIndex], gBuildObjectRY[playerid][objectIndex], targetRZ);
	}

	gBuildDoorOpen[playerid][objectIndex] = open;
	gBuildDoorMovingUntil[playerid][objectIndex] = now + BUILD_DEMO_DOOR_MOVE_MS;
	if (gBuildRemoveFocusedObject[playerid] == objectIndex)
	{
		SyncBuildRemoveHighlight(playerid, objectIndex);
	}
	return 1;
}

stock ToggleFocusedBuildDoor(playerid)
{
	new objectIndex;
	new Float:targetDistance;
	if (!FindBuildDoorFromAim(playerid, objectIndex, targetDistance))
	{
		return EndBuildDoorInteraction(playerid);
	}
	return ToggleBuildDoor(playerid, objectIndex);
}

stock RunBuildDoorTargetOption(playerid, optionid)
{
	if (optionid != BUILD_DEMO_DOOR_OPTION_TOGGLE)
	{
		ResetBuildDoorTargetState(playerid);
		return 1;
	}

	new objectIndex;
	new Float:targetDistance;
	if (!FindBuildDoorFromAim(playerid, objectIndex, targetDistance))
	{
		ResetBuildDoorTargetState(playerid);
		return 1;
	}

	ResetBuildDoorTargetState(playerid);
	return ToggleBuildDoor(playerid, objectIndex);
}

stock Float:GetBuildObjectDistanceToPlayer(playerid, objectIndex)
{
	new Float:px, Float:py, Float:pz;
	if (!GetPlayerPos(playerid, px, py, pz))
	{
		return 0.0;
	}

	new Float:objectX = gBuildObjectX[playerid][objectIndex];
	new Float:objectY = gBuildObjectY[playerid][objectIndex];
	if (gBuildObjectPart[playerid][objectIndex] == SAMPP_BUILD_PART_DOOR)
	{
		GetForwardPoint(gBuildObjectX[playerid][objectIndex], gBuildObjectY[playerid][objectIndex], GetBuildObjectCurrentRZ(playerid, objectIndex), BUILD_DEMO_DOOR_HALF_WIDTH, objectX, objectY);
	}

	new Float:dx = px - objectX;
	new Float:dy = py - objectY;
	new Float:dz = pz - gBuildObjectZ[playerid][objectIndex];
	return floatsqroot((dx * dx) + (dy * dy) + (dz * dz));
}

stock SyncBuildRemoveHighlight(playerid, objectIndex)
{
	if (objectIndex < 0 || objectIndex >= BUILD_DEMO_MAX_OBJECTS || gBuildObjects[playerid][objectIndex] == INVALID_OBJECT_ID)
	{
		return DestroyBuildRemoveHighlight(playerid);
	}

	new partid = gBuildObjectPart[playerid][objectIndex];
	new modelid = GetBuildRemoveHighlightModelForObject(playerid, objectIndex);
	if (modelid == 0)
	{
		return DestroyBuildRemoveHighlight(playerid);
	}

	new Float:x = gBuildObjectX[playerid][objectIndex];
	new Float:y = gBuildObjectY[playerid][objectIndex];
	new Float:z = gBuildObjectZ[playerid][objectIndex];
	new Float:rx = gBuildObjectRX[playerid][objectIndex];
	new Float:ry = gBuildObjectRY[playerid][objectIndex];
	new Float:rz = GetBuildObjectCurrentRZ(playerid, objectIndex);

	new layerCount = GetBuildRemoveHighlightLayerCount(partid);
	for (new layer = 0; layer < 2; layer++)
	{
		if (layer >= layerCount)
		{
			if (gBuildRemoveHighlightObject[playerid][layer] != INVALID_OBJECT_ID)
			{
				DestroyPlayerObject(playerid, gBuildRemoveHighlightObject[playerid][layer]);
			}
			gBuildRemoveHighlightObject[playerid][layer] = INVALID_OBJECT_ID;
			gBuildRemoveHighlightModel[playerid][layer] = 0;
			continue;
		}

		new Float:layerX = x;
		new Float:layerY = y;
		new Float:layerZ = z;
		OffsetBuildRemoveHighlight(partid, layer, rz, layerX, layerY, layerZ);

		if (gBuildRemoveHighlightObject[playerid][layer] == INVALID_OBJECT_ID || gBuildRemoveHighlightModel[playerid][layer] != modelid)
		{
			if (gBuildRemoveHighlightObject[playerid][layer] != INVALID_OBJECT_ID)
			{
				DestroyPlayerObject(playerid, gBuildRemoveHighlightObject[playerid][layer]);
			}
			gBuildRemoveHighlightObject[playerid][layer] = CreatePlayerObject(playerid, modelid, layerX, layerY, layerZ, rx, ry, rz, 80.0);
			gBuildRemoveHighlightModel[playerid][layer] = modelid;
		}
		else
		{
			SetPlayerObjectPos(playerid, gBuildRemoveHighlightObject[playerid][layer], layerX, layerY, layerZ);
			SetPlayerObjectRot(playerid, gBuildRemoveHighlightObject[playerid][layer], rx, ry, rz);
		}
	}

	return gBuildRemoveHighlightObject[playerid][0] != INVALID_OBJECT_ID;
}

stock SendBuildRemoveFocus(playerid, bool:active, objectIndex = -1, Float:distance = 0.0)
{
	if (!active || objectIndex < 0 || objectIndex >= BUILD_DEMO_MAX_OBJECTS || gBuildObjects[playerid][objectIndex] == INVALID_OBJECT_ID)
	{
		gBuildRemoveFocusedObject[playerid] = -1;
		DestroyBuildRemoveHighlight(playerid);
		return SAMPP_BuildSetRemoveTarget(playerid, false);
	}

	SyncBuildRemoveHighlight(playerid, objectIndex);
	new partName[32];
	GetBuildPartDisplayName(gBuildObjectPart[playerid][objectIndex], partName, sizeof partName);
	gBuildRemoveFocusedObject[playerid] = objectIndex;
	if (distance <= 0.01)
	{
		distance = GetBuildObjectDistanceToPlayer(playerid, objectIndex);
	}
	return SAMPP_BuildSetRemoveTarget(playerid, true, gBuildObjectPart[playerid][objectIndex], partName, distance);
}

stock RefreshBuildRemoveFocus(playerid, bool:force = false)
{
	if (!gBuildActive[playerid] || gBuildSelectedPart[playerid] != SAMPP_BUILD_PART_REMOVE)
	{
		if (gBuildRemoveFocusedObject[playerid] != -1)
		{
			SendBuildRemoveFocus(playerid, false);
		}
		return 0;
	}

	new now = GetTickCount();
	if (!force && now < gBuildRemoveTargetNextUpdate[playerid])
	{
		return 1;
	}
	gBuildRemoveTargetNextUpdate[playerid] = now + BUILD_DEMO_REMOVE_TARGET_MS;

	new objectIndex;
	new Float:targetDistance;
	if (!FindBuildObjectFromAim(playerid, objectIndex, targetDistance))
	{
		SendBuildRemoveFocus(playerid, false);
		return 0;
	}

	SendBuildRemoveFocus(playerid, true, objectIndex, targetDistance);
	return 1;
}

stock bool:BuildFoundationHasChildren(playerid, foundationIndex)
{
	if (foundationIndex < 0 || foundationIndex >= BUILD_DEMO_MAX_OBJECTS || !gBuildFoundationActive[playerid][foundationIndex])
	{
		return false;
	}

	if (gBuildFoundationTopPiece[playerid][foundationIndex] != BUILD_DEMO_PIECE_NONE
		|| gBuildFoundationStairsPiece[playerid][foundationIndex] != BUILD_DEMO_PIECE_NONE)
	{
		return true;
	}

	for (new slot = 0; slot < 4; slot++)
	{
		if (gBuildFoundationEdgePiece[playerid][foundationIndex][slot] != BUILD_DEMO_PIECE_NONE
			|| gBuildFoundationEdgeDoor[playerid][foundationIndex][slot])
		{
			return true;
		}
	}
	return false;
}

stock RefreshBuildFoundationSummary(playerid)
{
	gBuildHasFoundation[playerid] = false;
	gBuildFoundationX[playerid] = 0.0;
	gBuildFoundationY[playerid] = 0.0;
	gBuildFoundationZ[playerid] = 0.0;
	gBuildFoundationA[playerid] = 0.0;

	for (new i = 0; i < BUILD_DEMO_MAX_OBJECTS; i++)
	{
		if (!gBuildFoundationActive[playerid][i])
		{
			continue;
		}

		gBuildHasFoundation[playerid] = true;
		gBuildFoundationX[playerid] = gBuildFoundationGridX[playerid][i];
		gBuildFoundationY[playerid] = gBuildFoundationGridY[playerid][i];
		gBuildFoundationZ[playerid] = gBuildFoundationGridZ[playerid][i];
		gBuildFoundationA[playerid] = gBuildFoundationGridA[playerid][i];
		return 1;
	}
	return 1;
}

stock bool:RemoveBuildObjectAtIndex(playerid, objectIndex)
{
	if (objectIndex < 0 || objectIndex >= BUILD_DEMO_MAX_OBJECTS || gBuildObjects[playerid][objectIndex] == INVALID_OBJECT_ID)
	{
		return false;
	}

	new partid = gBuildObjectPart[playerid][objectIndex];
	new foundation = gBuildObjectFoundation[playerid][objectIndex];
	new slotKind = gBuildObjectSlotKind[playerid][objectIndex];
	new slot = gBuildObjectSlotIndex[playerid][objectIndex];

	if (partid == SAMPP_BUILD_PART_FOUNDATION && BuildFoundationHasChildren(playerid, foundation))
	{
		SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "Remove child pieces before removing this foundation.");
		return false;
	}

	if (partid == SAMPP_BUILD_PART_DOORFRAME
		&& foundation >= 0 && foundation < BUILD_DEMO_MAX_OBJECTS
		&& slot >= 0 && slot < 4
		&& gBuildFoundationEdgeDoor[playerid][foundation][slot])
	{
		SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "Remove the door before removing this door frame.");
		return false;
	}

	switch (partid)
	{
		case SAMPP_BUILD_PART_FOUNDATION:
		{
			if (foundation >= 0 && foundation < BUILD_DEMO_MAX_OBJECTS && gBuildFoundationActive[playerid][foundation])
			{
				gBuildFoundationActive[playerid][foundation] = false;
				gBuildFoundationGridX[playerid][foundation] = 0.0;
				gBuildFoundationGridY[playerid][foundation] = 0.0;
				gBuildFoundationGridZ[playerid][foundation] = 0.0;
				gBuildFoundationGridA[playerid][foundation] = 0.0;
				gBuildFoundationTopPiece[playerid][foundation] = BUILD_DEMO_PIECE_NONE;
				gBuildFoundationStairsPiece[playerid][foundation] = BUILD_DEMO_PIECE_NONE;
				for (new edge = 0; edge < 4; edge++)
				{
					gBuildFoundationEdgePiece[playerid][foundation][edge] = BUILD_DEMO_PIECE_NONE;
					gBuildFoundationEdgeDoor[playerid][foundation][edge] = false;
				}
				if (gBuildFoundationCount[playerid] > 0)
				{
					gBuildFoundationCount[playerid]--;
				}
				RefreshBuildFoundationSummary(playerid);
			}
		}
		case SAMPP_BUILD_PART_WALL:
		{
			if (slotKind == BUILD_DEMO_SLOT_EDGE && foundation >= 0 && foundation < BUILD_DEMO_MAX_OBJECTS && slot >= 0 && slot < 4)
			{
				gBuildFoundationEdgePiece[playerid][foundation][slot] = BUILD_DEMO_PIECE_NONE;
			}
		}
		case SAMPP_BUILD_PART_DOORFRAME:
		{
			if (slotKind == BUILD_DEMO_SLOT_EDGE && foundation >= 0 && foundation < BUILD_DEMO_MAX_OBJECTS && slot >= 0 && slot < 4)
			{
				gBuildFoundationEdgePiece[playerid][foundation][slot] = BUILD_DEMO_PIECE_NONE;
				gBuildFoundationEdgeDoor[playerid][foundation][slot] = false;
			}
		}
		case SAMPP_BUILD_PART_FLOOR, SAMPP_BUILD_PART_ROOF:
		{
			if (slotKind == BUILD_DEMO_SLOT_TOP && foundation >= 0 && foundation < BUILD_DEMO_MAX_OBJECTS)
			{
				gBuildFoundationTopPiece[playerid][foundation] = BUILD_DEMO_PIECE_NONE;
			}
		}
		case SAMPP_BUILD_PART_STAIRS:
		{
			if (slotKind == BUILD_DEMO_SLOT_STAIRS && foundation >= 0 && foundation < BUILD_DEMO_MAX_OBJECTS)
			{
				gBuildFoundationStairsPiece[playerid][foundation] = BUILD_DEMO_PIECE_NONE;
			}
		}
		case SAMPP_BUILD_PART_DOOR:
		{
			if (slotKind == BUILD_DEMO_SLOT_DOOR && foundation >= 0 && foundation < BUILD_DEMO_MAX_OBJECTS && slot >= 0 && slot < 4)
			{
				gBuildFoundationEdgeDoor[playerid][foundation][slot] = false;
			}
		}
		default:
		{
			return false;
		}
	}

	DestroyObject(gBuildObjects[playerid][objectIndex]);
	if (gBuildDoorFocusedObject[playerid] == objectIndex)
	{
		EndBuildDoorInteraction(playerid);
	}
	ResetBuildObjectSlot(playerid, objectIndex);
	if (gBuildObjectCount[playerid] > 0)
	{
		gBuildObjectCount[playerid]--;
	}
	return true;
}

stock RemoveBuildDemoTargetPart(playerid)
{
	if (!gBuildActive[playerid])
	{
		return SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "Build session is not active.");
	}

	new objectIndex;
	new Float:targetDistance;
	if (!FindBuildObjectFromAim(playerid, objectIndex, targetDistance))
	{
		return SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "No build part targeted.");
	}

	new partName[32];
	GetBuildPartDisplayName(gBuildObjectPart[playerid][objectIndex], partName, sizeof partName);
	if (!RemoveBuildObjectAtIndex(playerid, objectIndex))
	{
		return 1;
	}

	DestroyBuildPreview(playerid);
	SendBuildRemoveFocus(playerid, false);
	gBuildRemoveTargetNextUpdate[playerid] = 0;
	new message[96];
	format(message, sizeof message, "%s removed.", partName);
	return SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_SUCCESS, message);
}

stock PlaceBuildDemoPart(playerid, partid, rotationStep, bool:flipped)
{
	if (!gBuildActive[playerid])
	{
		return SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "Build session is not active.");
	}

	if (partid == SAMPP_BUILD_PART_REMOVE)
	{
		return RemoveBuildDemoTargetPart(playerid);
	}

	if (gBuildObjectCount[playerid] >= BUILD_DEMO_MAX_OBJECTS)
	{
		return SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "Demo object limit reached. Use /buildclear.");
	}

	gBuildSelectedPart[playerid] = partid;
	gBuildRotationStep[playerid] = GetBuildPlacementRotationStep(playerid, partid, rotationStep);
	gBuildFlipped[playerid] = BuildDemoPartSupportsVariant(partid) ? flipped : false;

	if (!RefreshBuildPreview(playerid, true))
	{
		return SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "No valid preview position for this part.");
	}
	if (!gBuildPreviewPlaceable[playerid])
	{
		new message[128];
		if (gBuildPreviewRejectReason[playerid][0] != EOS)
		{
			format(message, sizeof message, "%s", gBuildPreviewRejectReason[playerid]);
		}
		else
		{
			format(message, sizeof message, "This build slot is not available.");
		}
		return SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, message);
	}

	new modelid = GetBuildPartModelForState(partid, gBuildPreviewFoundationHeightStep[playerid]);
	new objectid = CreateObject(
		modelid,
		gBuildPreviewX[playerid],
		gBuildPreviewY[playerid],
		gBuildPreviewZ[playerid],
		gBuildPreviewRX[playerid],
		gBuildPreviewRY[playerid],
		gBuildPreviewRZ[playerid]
	);
	if (objectid == INVALID_OBJECT_ID)
	{
		return SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "Could not create object. Check demo model ids.");
	}

	new foundationIndex;
	new slotKind;
	new slotIndex;
	if (!CommitBuildDemoSlot(playerid, partid, foundationIndex, slotKind, slotIndex))
	{
		DestroyObject(objectid);
		return SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "Build slot changed before placement. Try again.");
	}
	if (!AddBuildDemoObject(playerid, objectid, partid, foundationIndex, slotKind, slotIndex, gBuildPreviewFoundationHeightStep[playerid], gBuildPreviewX[playerid], gBuildPreviewY[playerid], gBuildPreviewZ[playerid], gBuildPreviewRX[playerid], gBuildPreviewRY[playerid], gBuildPreviewRZ[playerid]))
	{
		return 1;
	}

	DestroyBuildPreview(playerid);
	ResetBuildFoundationSnapHeightLock(playerid);
	RefreshBuildPreview(playerid, true);
	return SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_SUCCESS, "Part placed from preview.");
}

stock SendBuildDemoHelp(playerid)
{
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] /builddemo opens a server-authoritative build menu.");
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] Select a part first; a temporary player-object preview follows your camera aim.");
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] LMB confirms the preview. RMB returns to the menu; RMB again or ESC closes. MMB switches side.");
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] Q/E sets only the first foundation top height; snapped foundations keep that same top level.");
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] Foundations snap to neighbours; walls/door frames snap to edge surfaces automatically.");
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] Aim a placed door from nearby and use the ALT target prompt to open or close it.");
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] Tools > Remove enters orange remove mode. Aim a placed part and press LMB to delete it.");
	return 1;
}

stock RegisterBuildDemoModels()
{
	static bool:registered = false;
	if (registered)
	{
		return 1;
	}

	new bool:foundationRegistered = AddSimpleModel(-1, BUILD_BASE_MODEL_FOUNDATION, BUILD_MODEL_FOUNDATION, BUILD_MODEL_FOUNDATION_DFF, BUILD_MODEL_FOUNDATION_TXD);
	new bool:wallRegistered = AddSimpleModel(-1, BUILD_BASE_MODEL_WALL, BUILD_MODEL_WALL, BUILD_MODEL_WALL_DFF, BUILD_MODEL_WALL_TXD);
	new bool:doorFrameRegistered = AddSimpleModel(-1, BUILD_BASE_MODEL_DOORFRAME, BUILD_MODEL_DOORFRAME, BUILD_MODEL_DOORFRAME_DFF, BUILD_MODEL_DOORFRAME_TXD);
	new bool:doorRegistered = AddSimpleModel(-1, BUILD_BASE_MODEL_DOOR, BUILD_MODEL_DOOR, BUILD_MODEL_DOOR_DFF, BUILD_MODEL_DOOR_TXD);
	new bool:floorRegistered = AddSimpleModel(-1, BUILD_BASE_MODEL_FLOOR, BUILD_MODEL_FLOOR, BUILD_MODEL_FLOOR_DFF, BUILD_MODEL_FLOOR_TXD);
	new bool:foundationPreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_FOUNDATION, BUILD_MODEL_FOUNDATION_PREVIEW_OK, BUILD_MODEL_FOUNDATION_DFF, BUILD_MODEL_PREVIEW_OK_TXD);
	new bool:wallPreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_WALL, BUILD_MODEL_WALL_PREVIEW_OK, BUILD_MODEL_WALL_DFF, BUILD_MODEL_PREVIEW_OK_TXD);
	new bool:doorFramePreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_DOORFRAME, BUILD_MODEL_DOORFRAME_PREVIEW_OK, BUILD_MODEL_DOORFRAME_DFF, BUILD_MODEL_PREVIEW_OK_TXD);
	new bool:doorPreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_DOOR, BUILD_MODEL_DOOR_PREVIEW_OK, BUILD_MODEL_DOOR_DFF, BUILD_MODEL_PREVIEW_OK_TXD);
	new bool:floorPreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_FLOOR, BUILD_MODEL_FLOOR_PREVIEW_OK, BUILD_MODEL_FLOOR_DFF, BUILD_MODEL_PREVIEW_OK_TXD);
	new bool:roofPreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_FLOOR, BUILD_MODEL_ROOF_PREVIEW_OK, BUILD_MODEL_FLOOR_DFF, BUILD_MODEL_PREVIEW_OK_TXD);
	new bool:stairsPreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_WALL, BUILD_MODEL_STAIRS_PREVIEW_OK, BUILD_MODEL_WALL_DFF, BUILD_MODEL_PREVIEW_OK_TXD);
	new bool:foundationPreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_FOUNDATION, BUILD_MODEL_FOUNDATION_PREVIEW_BAD, BUILD_MODEL_FOUNDATION_DFF, BUILD_MODEL_PREVIEW_BAD_TXD);
	new bool:wallPreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_WALL, BUILD_MODEL_WALL_PREVIEW_BAD, BUILD_MODEL_WALL_DFF, BUILD_MODEL_PREVIEW_BAD_TXD);
	new bool:doorFramePreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_DOORFRAME, BUILD_MODEL_DOORFRAME_PREVIEW_BAD, BUILD_MODEL_DOORFRAME_DFF, BUILD_MODEL_PREVIEW_BAD_TXD);
	new bool:doorPreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_DOOR, BUILD_MODEL_DOOR_PREVIEW_BAD, BUILD_MODEL_DOOR_DFF, BUILD_MODEL_PREVIEW_BAD_TXD);
	new bool:floorPreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_FLOOR, BUILD_MODEL_FLOOR_PREVIEW_BAD, BUILD_MODEL_FLOOR_DFF, BUILD_MODEL_PREVIEW_BAD_TXD);
	new bool:roofPreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_FLOOR, BUILD_MODEL_ROOF_PREVIEW_BAD, BUILD_MODEL_FLOOR_DFF, BUILD_MODEL_PREVIEW_BAD_TXD);
	new bool:stairsPreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_WALL, BUILD_MODEL_STAIRS_PREVIEW_BAD, BUILD_MODEL_WALL_DFF, BUILD_MODEL_PREVIEW_BAD_TXD);
	new bool:foundationHighlight = AddSimpleModel(-1, BUILD_BASE_MODEL_FOUNDATION, BUILD_MODEL_FOUNDATION_HIGHLIGHT, BUILD_MODEL_FOUNDATION_DFF, BUILD_MODEL_REMOVE_HIGHLIGHT_TXD);
	new bool:wallHighlight = AddSimpleModel(-1, BUILD_BASE_MODEL_WALL, BUILD_MODEL_WALL_HIGHLIGHT, BUILD_MODEL_WALL_DFF, BUILD_MODEL_REMOVE_HIGHLIGHT_TXD);
	new bool:doorFrameHighlight = AddSimpleModel(-1, BUILD_BASE_MODEL_DOORFRAME, BUILD_MODEL_DOORFRAME_HIGHLIGHT, BUILD_MODEL_DOORFRAME_DFF, BUILD_MODEL_REMOVE_HIGHLIGHT_TXD);
	new bool:doorHighlight = AddSimpleModel(-1, BUILD_BASE_MODEL_DOOR, BUILD_MODEL_DOOR_HIGHLIGHT, BUILD_MODEL_DOOR_DFF, BUILD_MODEL_REMOVE_HIGHLIGHT_TXD);
	new bool:floorHighlight = AddSimpleModel(-1, BUILD_BASE_MODEL_FLOOR, BUILD_MODEL_FLOOR_HIGHLIGHT, BUILD_MODEL_FLOOR_DFF, BUILD_MODEL_REMOVE_HIGHLIGHT_TXD);
	new bool:roofHighlight = AddSimpleModel(-1, BUILD_BASE_MODEL_FLOOR, BUILD_MODEL_ROOF_HIGHLIGHT, BUILD_MODEL_FLOOR_DFF, BUILD_MODEL_REMOVE_HIGHLIGHT_TXD);
	new bool:stairsHighlight = AddSimpleModel(-1, BUILD_BASE_MODEL_WALL, BUILD_MODEL_STAIRS_HIGHLIGHT, BUILD_MODEL_WALL_DFF, BUILD_MODEL_REMOVE_HIGHLIGHT_TXD);

	for (new heightStep = 0; heightStep < BUILD_DEMO_FOUNDATION_HEIGHT_STEPS; heightStep++)
	{
		new foundationDff[32];
		GetBuildFoundationDff(heightStep, foundationDff, sizeof foundationDff);
		foundationRegistered = AddSimpleModel(-1, BUILD_BASE_MODEL_FOUNDATION, GetBuildFoundationModel(heightStep), foundationDff, BUILD_MODEL_FOUNDATION_TXD) && foundationRegistered;
		foundationPreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_FOUNDATION, GetBuildFoundationPreviewModel(heightStep, true), foundationDff, BUILD_MODEL_PREVIEW_OK_TXD) && foundationPreviewOK;
		foundationPreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_FOUNDATION, GetBuildFoundationPreviewModel(heightStep, false), foundationDff, BUILD_MODEL_PREVIEW_BAD_TXD) && foundationPreviewBad;
		foundationHighlight = AddSimpleModel(-1, BUILD_BASE_MODEL_FOUNDATION, GetBuildFoundationHighlightModel(heightStep), foundationDff, BUILD_MODEL_REMOVE_HIGHLIGHT_TXD) && foundationHighlight;
	}

	if (foundationRegistered)
	{
		print("[BuildDemo] Registered custom foundation height models from models/foundation*.dff and models/foundation.txd.");
	}
	else
	{
		print("[BuildDemo] Could not register custom foundation height models. Check artwork config and model files.");
	}

	if (wallRegistered)
	{
		print("[BuildDemo] Registered custom wall model -2001 from models/wall.dff and models/wall.txd.");
	}
	else
	{
		print("[BuildDemo] Could not register custom wall model -2001. Check artwork config and model files.");
	}

	if (doorFrameRegistered)
	{
		print("[BuildDemo] Registered custom door frame model -2002 from models/door-frame.dff and models/door-frame.txd.");
	}
	else
	{
		print("[BuildDemo] Could not register custom door frame model -2002. Check artwork config and model files.");
	}

	if (doorRegistered)
	{
		print("[BuildDemo] Registered custom door model -2006 from models/door.dff and models/door.txd.");
	}
	else
	{
		print("[BuildDemo] Could not register custom door model -2006. Check artwork config and model files.");
	}

	if (floorRegistered)
	{
		print("[BuildDemo] Registered custom floor model -2003 from models/floor.dff and models/floor.txd.");
	}
	else
	{
		print("[BuildDemo] Could not register custom floor model -2003. Check artwork config and model files.");
	}

	if (foundationPreviewOK && wallPreviewOK && doorFramePreviewOK && doorPreviewOK && floorPreviewOK && roofPreviewOK && stairsPreviewOK
		&& foundationPreviewBad && wallPreviewBad && doorFramePreviewBad && doorPreviewBad && floorPreviewBad && roofPreviewBad && stairsPreviewBad
		&& foundationHighlight && wallHighlight && doorFrameHighlight && doorHighlight && floorHighlight && roofHighlight && stairsHighlight)
	{
		print("[BuildDemo] Registered green/red preview build models and orange remove highlight models.");
	}
	else
	{
		print("[BuildDemo] Could not register one or more preview/highlight models. Check green/red/orange preview TXDs.");
	}

	registered = foundationRegistered && wallRegistered && doorFrameRegistered && doorRegistered && floorRegistered
		&& foundationPreviewOK && wallPreviewOK && doorFramePreviewOK && doorPreviewOK && floorPreviewOK && roofPreviewOK && stairsPreviewOK
		&& foundationPreviewBad && wallPreviewBad && doorFramePreviewBad && doorPreviewBad && floorPreviewBad && roofPreviewBad && stairsPreviewBad
		&& foundationHighlight && wallHighlight && doorFrameHighlight && doorHighlight && floorHighlight && roofHighlight && stairsHighlight;
	return registered ? 1 : 0;
}

stock bool:IsBuildDemoActive(playerid)
{
	return gBuildActive[playerid];
}

stock GetBuildDemoSession(playerid)
{
	return gBuildSession[playerid];
}

stock SetBuildDemoSelection(playerid, partid, rotationStep, bool:flipped, bool:forceRefresh = true)
{
	gBuildSelectedPart[playerid] = partid;
	gBuildRotationStep[playerid] = GetBuildPlacementRotationStep(playerid, partid, rotationStep);
	gBuildFlipped[playerid] = BuildDemoPartSupportsVariant(partid) ? flipped : false;
	if (partid != SAMPP_BUILD_PART_FOUNDATION || gBuildFoundationCount[playerid] > 0)
	{
		ResetBuildFirstFoundationAimLock(playerid);
	}
	if (partid == SAMPP_BUILD_PART_REMOVE)
	{
		DestroyBuildPreview(playerid);
		RefreshBuildRemoveFocus(playerid, true);
		return 1;
	}
	SendBuildRemoveFocus(playerid, false);
	return RefreshBuildPreview(playerid, forceRefresh);
}

stock CancelBuildDemoSession(playerid, bool:sendMessage = false)
{
	gBuildActive[playerid] = false;
	SendBuildRemoveFocus(playerid, false);
	DestroyBuildPreview(playerid);
	ResetBuildPreviewState(playerid);

	if (sendMessage)
	{
		SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] Build session closed.");
	}
	return 1;
}

public OnFilterScriptInit()
{
	RegisterBuildDemoModels();

	for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		ResetBuildDemoPlayer(playerid);
	}

	print("[BuildDemo] OpenMP-Plus build demo loaded.");
	return 1;
}

public OnFilterScriptExit()
{
	for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		DestroyBuildDemoObjects(playerid);
	}
	return 1;
}

public OnPlayerConnect(playerid)
{
	ResetBuildDemoPlayer(playerid);
	return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
	CloseBuildDemo(playerid);
	DestroyBuildDemoObjects(playerid);
	ResetBuildDemoPlayer(playerid);
	return 1;
}

public OnPlayerUpdate(playerid)
{
	RefreshBuildDoorInteraction(playerid);

	if (gBuildActive[playerid])
	{
		if (gBuildSelectedPart[playerid] == SAMPP_BUILD_PART_REMOVE)
		{
			RefreshBuildRemoveFocus(playerid);
		}
		else
		{
			RefreshBuildPreview(playerid);
		}
	}
	return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
	if (!strcmp(cmdtext, "/buildhelp", true))
	{
		return SendBuildDemoHelp(playerid);
	}

	if (!strcmp(cmdtext, "/builddemo", true) || !strcmp(cmdtext, "/build", true))
	{
		return OpenBuildDemo(playerid);
	}

	if (!strcmp(cmdtext, "/buildclose", true))
	{
		CloseBuildDemo(playerid);
		SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] Build UI closed.");
		return 1;
	}

	if (!strcmp(cmdtext, "/buildclear", true))
	{
		CloseBuildDemo(playerid);
		DestroyBuildDemoObjects(playerid);
		SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] Demo build objects cleared.");
		return 1;
	}

	return 0;
}

public OnPlayerOMPPlusReady(playerid)
{
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] OpenMP-Plus connected. Use /builddemo to test the build UI.");
	return 1;
}

public OnPlayerOMPPlusBuildSelect(playerid, sessionid, partid)
{
	if (sessionid != GetBuildDemoSession(playerid))
	{
		return 1;
	}

	if (partid == SAMPP_BUILD_PART_FOUNDATION)
	{
		gBuildSelectedPart[playerid] = partid;
		gBuildRotationStep[playerid] = GetBuildDefaultRotationStep(partid);
		gBuildFlipped[playerid] = false;
		ResetBuildFoundationSnapHeightLock(playerid);
		ResetBuildFirstFoundationAimLock(playerid);
		SetBuildAimHit(playerid, false, 0.0, 0.0, 0.0);
		SendBuildRemoveFocus(playerid, false);
		DestroyBuildPreview(playerid);
	}
	else
	{
		SetBuildDemoSelection(playerid, partid, GetBuildDefaultRotationStep(partid), false, true);
	}

	new message[96];
	if (partid == SAMPP_BUILD_PART_REMOVE)
	{
		format(message, sizeof message, "[BuildDemo] Remove mode active. Aim a placed part and press LMB to delete it.");
	}
	else
	{
		format(message, sizeof message, "[BuildDemo] Selected part id %d. A preview is now active; LMB confirms it.", partid);
	}
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, message);
	return 1;
}

public OnPlayerOMPPlusTargetSelect(playerid, targetid, optionid)
{
	if (targetid != BuildDoorTargetId(playerid))
	{
		return 1;
	}

	return RunBuildDoorTargetOption(playerid, optionid);
}

public OnPlayerOMPPlusBuildPreview(playerid, sessionid, partid, rotation_step, bool:flipped)
{
	if (sessionid != GetBuildDemoSession(playerid))
	{
		return 1;
	}

	SetBuildAimHit(playerid, false, 0.0, 0.0, 0.0);
	SetBuildDemoSelection(playerid, partid, rotation_step, flipped, true);
	return 1;
}

public OnPlayerOMPPlusBuildPreviewEx(playerid, sessionid, partid, rotation_step, bool:flipped, bool:has_aim_hit, Float:aim_x, Float:aim_y, Float:aim_z)
{
	if (sessionid != GetBuildDemoSession(playerid))
	{
		return 1;
	}

	SetBuildAimHit(playerid, has_aim_hit, aim_x, aim_y, aim_z);
	SetBuildDemoSelection(playerid, partid, rotation_step, flipped, true);
	return 1;
}

public OnPlayerOMPPlusBuildPreviewGroundEx(playerid, sessionid, partid, rotation_step, bool:flipped, bool:has_aim_hit, Float:aim_x, Float:aim_y, Float:aim_z, aim_surface_state, Float:footprint_min_ground_z, Float:footprint_max_ground_z)
{
	if (sessionid != GetBuildDemoSession(playerid))
	{
		return 1;
	}

	SetBuildAimGroundHit(playerid, has_aim_hit, aim_x, aim_y, aim_z, aim_surface_state, footprint_min_ground_z, footprint_max_ground_z);
	SetBuildDemoSelection(playerid, partid, rotation_step, flipped, true);
	return 1;
}

public OnPlayerOMPPlusBuildPlace(playerid, sessionid, partid, rotation_step, bool:flipped)
{
	if (sessionid != GetBuildDemoSession(playerid))
	{
		return 1;
	}

	SetBuildAimHit(playerid, false, 0.0, 0.0, 0.0);
	return PlaceBuildDemoPart(playerid, partid, rotation_step, flipped);
}

public OnPlayerOMPPlusBuildPlaceEx(playerid, sessionid, partid, rotation_step, bool:flipped, bool:has_aim_hit, Float:aim_x, Float:aim_y, Float:aim_z)
{
	if (sessionid != GetBuildDemoSession(playerid))
	{
		return 1;
	}

	SetBuildAimHit(playerid, has_aim_hit, aim_x, aim_y, aim_z);
	return PlaceBuildDemoPart(playerid, partid, rotation_step, flipped);
}

public OnPlayerOMPPlusBuildPlaceGroundEx(playerid, sessionid, partid, rotation_step, bool:flipped, bool:has_aim_hit, Float:aim_x, Float:aim_y, Float:aim_z, aim_surface_state, Float:footprint_min_ground_z, Float:footprint_max_ground_z)
{
	if (sessionid != GetBuildDemoSession(playerid))
	{
		return 1;
	}

	SetBuildAimGroundHit(playerid, has_aim_hit, aim_x, aim_y, aim_z, aim_surface_state, footprint_min_ground_z, footprint_max_ground_z);
	return PlaceBuildDemoPart(playerid, partid, rotation_step, flipped);
}

public OnPlayerOMPPlusBuildCancel(playerid, sessionid)
{
	if (sessionid == GetBuildDemoSession(playerid))
	{
		CancelBuildDemoSession(playerid, true);
	}
	return 1;
}
