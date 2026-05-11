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
#define BUILD_DEMO_WALL_HEIGHT 1.55
#define BUILD_DEMO_ROOF_HEIGHT 3.05
#define BUILD_DEMO_PREVIEW_MS 75
#define BUILD_DEMO_EDGE_SNAP_RADIUS 2.25
#define BUILD_DEMO_CENTER_SNAP_RADIUS 2.35
#define BUILD_DEMO_REJECT_REASON_SIZE 96

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
#define BUILD_BASE_MODEL_WALL 19380
#define BUILD_MODEL_WALL -2001
#define BUILD_MODEL_WALL_PREVIEW_OK -2101
#define BUILD_MODEL_WALL_PREVIEW_BAD -2201
#define BUILD_BASE_MODEL_DOORFRAME 19381
#define BUILD_MODEL_DOORFRAME -2002
#define BUILD_MODEL_DOORFRAME_PREVIEW_OK -2102
#define BUILD_MODEL_DOORFRAME_PREVIEW_BAD -2202
#define BUILD_BASE_MODEL_FLOOR 19378
#define BUILD_MODEL_FLOOR -2003
#define BUILD_MODEL_FLOOR_PREVIEW_OK -2103
#define BUILD_MODEL_FLOOR_PREVIEW_BAD -2203
#define BUILD_MODEL_ROOF 19377
#define BUILD_MODEL_ROOF_PREVIEW_OK -2104
#define BUILD_MODEL_ROOF_PREVIEW_BAD -2204
#define BUILD_MODEL_STAIRS 19387
#define BUILD_MODEL_STAIRS_PREVIEW_OK -2105
#define BUILD_MODEL_STAIRS_PREVIEW_BAD -2205
#define BUILD_MODEL_DOOR 1491
#define BUILD_MODEL_DOOR_PREVIEW_OK -2106
#define BUILD_MODEL_DOOR_PREVIEW_BAD -2206
#define BUILD_MODEL_PREVIEW_OK_TXD "build-preview-green.txd"
#define BUILD_MODEL_PREVIEW_BAD_TXD "build-preview-red.txd"
#define BUILD_MODEL_FOUNDATION_DFF "foundation.dff"
#define BUILD_MODEL_FOUNDATION_TXD "foundation.txd"
#define BUILD_MODEL_WALL_DFF "wall.dff"
#define BUILD_MODEL_WALL_TXD "wall.txd"
#define BUILD_MODEL_DOORFRAME_DFF "door-frame.dff"
#define BUILD_MODEL_DOORFRAME_TXD "door-frame.txd"
#define BUILD_MODEL_FLOOR_DFF "floor.dff"
#define BUILD_MODEL_FLOOR_TXD "floor.txd"

static gBuildSession[MAX_PLAYERS];
static bool:gBuildActive[MAX_PLAYERS];
static gBuildSelectedPart[MAX_PLAYERS];
static gBuildRotationStep[MAX_PLAYERS];
static bool:gBuildFlipped[MAX_PLAYERS];
static gBuildObjects[MAX_PLAYERS][BUILD_DEMO_MAX_OBJECTS];
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
static gBuildPreviewRejectReason[MAX_PLAYERS][BUILD_DEMO_REJECT_REASON_SIZE];
static bool:gBuildHasFoundation[MAX_PLAYERS];
static Float:gBuildFoundationX[MAX_PLAYERS];
static Float:gBuildFoundationY[MAX_PLAYERS];
static Float:gBuildFoundationZ[MAX_PLAYERS];
static Float:gBuildFoundationA[MAX_PLAYERS];
static gBuildFoundationCount[MAX_PLAYERS];
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

stock GetForwardPoint(Float:x, Float:y, Float:a, Float:distance, &Float:outX, &Float:outY)
{
	outX = x + (floatsin(-a, degrees) * distance);
	outY = y + (floatcos(-a, degrees) * distance);
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

stock SetBuildPreviewCandidate(playerid, bool:placeable, parentFoundation, slotIndex, slotKind, const reason[] = "")
{
	gBuildPreviewPlaceable[playerid] = placeable;
	gBuildPreviewParentFoundation[playerid] = parentFoundation;
	gBuildPreviewSlotIndex[playerid] = slotIndex;
	gBuildPreviewSlotKind[playerid] = slotKind;
	format(gBuildPreviewRejectReason[playerid], sizeof(gBuildPreviewRejectReason[]), "%s", reason);
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
	SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_NONE, "");
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

stock ResetBuildDemoPlayer(playerid)
{
	gBuildSession[playerid] = BuildDemoSession(playerid);
	gBuildActive[playerid] = false;
	gBuildObjectCount[playerid] = 0;
	gBuildPreviewObject[playerid] = INVALID_OBJECT_ID;
	gBuildHasFoundation[playerid] = false;
	gBuildFoundationX[playerid] = 0.0;
	gBuildFoundationY[playerid] = 0.0;
	gBuildFoundationZ[playerid] = 0.0;
	gBuildFoundationA[playerid] = 0.0;
	gBuildFoundationCount[playerid] = 0;
	ResetBuildPreviewState(playerid);

	for (new i = 0; i < BUILD_DEMO_MAX_OBJECTS; i++)
	{
		gBuildObjects[playerid][i] = INVALID_OBJECT_ID;
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

	for (new i = 0; i < gBuildObjectCount[playerid]; i++)
	{
		if (gBuildObjects[playerid][i] != INVALID_OBJECT_ID)
		{
			DestroyObject(gBuildObjects[playerid][i]);
			gBuildObjects[playerid][i] = INVALID_OBJECT_ID;
		}
	}

	gBuildObjectCount[playerid] = 0;
	gBuildHasFoundation[playerid] = false;
	gBuildFoundationCount[playerid] = 0;
	for (new i = 0; i < BUILD_DEMO_MAX_OBJECTS; i++)
	{
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

stock bool:AddBuildDemoObject(playerid, objectid)
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

	gBuildObjects[playerid][gBuildObjectCount[playerid]++] = objectid;
	return true;
}

stock bool:IsBuildFoundationSlotOccupied(playerid, Float:x, Float:y)
{
	for (new i = 0; i < gBuildFoundationCount[playerid]; i++)
	{
		if (GetBuildDistance2D(x, y, gBuildFoundationGridX[playerid][i], gBuildFoundationGridY[playerid][i]) <= BUILD_DEMO_FOUNDATION_OCCUPIED_RADIUS)
		{
			return true;
		}
	}
	return false;
}

stock bool:RegisterBuildFoundation(playerid, Float:x, Float:y, Float:z, Float:a)
{
	if (gBuildFoundationCount[playerid] >= BUILD_DEMO_MAX_OBJECTS)
	{
		return false;
	}

	new index = gBuildFoundationCount[playerid]++;
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

stock bool:GetNearestFoundationCenterToPoint(playerid, Float:aimX, Float:aimY, &foundationIndex, &Float:centerX, &Float:centerY, &Float:centerZ, &Float:centerA, &Float:centerDistance)
{
	if (gBuildFoundationCount[playerid] <= 0)
	{
		return false;
	}

	new bestIndex = 0;
	new Float:bestDistance = 999999.0;
	for (new i = 0; i < gBuildFoundationCount[playerid]; i++)
	{
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

	for (new i = 0; i < gBuildFoundationCount[playerid]; i++)
	{
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
	for (new foundation = 0; foundation < gBuildFoundationCount[playerid]; foundation++)
	{
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

stock bool:ComputeBuildPreview(playerid, partid, rotationStep, bool:flipped, &modelid, &Float:x, &Float:y, &Float:z, &Float:rx, &Float:ry, &Float:rz)
{
	modelid = GetBuildPartModel(partid);
	if (modelid == 0)
	{
		SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_NONE, "Unknown build part.");
		return false;
	}

	rx = 0.0;
	ry = 0.0;
	rz = 0.0;
	SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_NONE, "");

	new Float:aimX, Float:aimY, Float:aimZ, Float:playerAngle;
	if (!ComputeAimPoint(playerid, BUILD_DEMO_FOUNDATION_Z_OFFSET, aimX, aimY, aimZ, playerAngle))
	{
		SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_NONE, "Could not compute camera aim point.");
		return false;
	}

	switch (partid)
	{
		case SAMPP_BUILD_PART_FOUNDATION:
		{
			if (gBuildFoundationCount[playerid] > 0)
			{
				new parentIndex;
				new slotIndex;
				new occupied;
				new Float:snapA;
				new Float:snapDistance;
				if (GetNearestFoundationSnapToPoint(playerid, aimX, aimY, parentIndex, slotIndex, occupied, x, y, z, snapA, snapDistance))
				{
					rz = NormalizeBuildAngle(snapA + (flipped ? 180.0 : 0.0));
					if (occupied)
					{
						SetBuildPreviewCandidate(playerid, false, parentIndex, slotIndex, BUILD_DEMO_SLOT_NONE, "That foundation neighbour slot is already occupied.");
					}
					else
					{
						SetBuildPreviewCandidate(playerid, true, parentIndex, slotIndex, BUILD_DEMO_SLOT_NONE, "");
					}
					return true;
				}

				x = aimX;
				y = aimY;
				z = aimZ;
				rz = NormalizeBuildAngle(playerAngle + GetRotationFromStep(rotationStep) + (flipped ? 180.0 : 0.0));
				SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_NONE, "Aim near an empty foundation neighbour slot.");
				return true;
			}

			x = aimX;
			y = aimY;
			z = aimZ;
			rz = NormalizeBuildAngle(playerAngle + GetRotationFromStep(rotationStep) + (flipped ? 180.0 : 0.0));
			SetBuildPreviewCandidate(playerid, true, -1, -1, BUILD_DEMO_SLOT_NONE, "");
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
				rz = NormalizeBuildAngle(playerAngle + GetRotationFromStep(rotationStep) + (flipped ? 180.0 : 0.0));
				SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_EDGE, "Wall pieces require a foundation edge.");
				return true;
			}

			z += BUILD_DEMO_WALL_HEIGHT;
			rz = NormalizeBuildAngle(edgeA + GetRotationFromStep(rotationStep) + (flipped ? 180.0 : 0.0));
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
				rz = NormalizeBuildAngle(playerAngle + GetRotationFromStep(rotationStep) + (flipped ? 180.0 : 0.0));
				SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_TOP, "Floors and roofs require a foundation.");
				return true;
			}

			z += BUILD_DEMO_ROOF_HEIGHT;
			rz = NormalizeBuildAngle(centerA + GetRotationFromStep(rotationStep) + (flipped ? 180.0 : 0.0));
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
				rz = NormalizeBuildAngle(playerAngle + GetRotationFromStep(rotationStep) + (flipped ? 180.0 : 0.0));
				SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_STAIRS, "Stairs require a foundation.");
				return true;
			}

			z += 0.1;
			rz = NormalizeBuildAngle(centerA + GetRotationFromStep(rotationStep) + (flipped ? 180.0 : 0.0));
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
				x = aimX;
				y = aimY;
				z = aimZ - BUILD_DEMO_FOUNDATION_Z_OFFSET;
				rz = NormalizeBuildAngle(playerAngle + GetRotationFromStep(rotationStep) + (flipped ? 180.0 : 0.0));
				SetBuildPreviewCandidate(playerid, false, -1, -1, BUILD_DEMO_SLOT_DOOR, "Doors require a placed door frame.");
				return true;
			}

			z += 0.05;
			rz = NormalizeBuildAngle(edgeA + GetRotationFromStep(rotationStep) + (flipped ? 180.0 : 0.0));
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
	if (!gBuildActive[playerid] || gBuildSelectedPart[playerid] == 0)
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

	new previewModelId = GetBuildPreviewPartModel(gBuildSelectedPart[playerid], gBuildPreviewPlaceable[playerid]);
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

	SendClientMessage(playerid, BUILD_DEMO_OK_COLOUR, "[BuildDemo] Build UI opened. Select a part to show a live preview.");
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] LMB confirms the preview. Q/E rotates, middle mouse flips, RMB/ESC closes.");
	return 1;
}

stock CloseBuildDemo(playerid)
{
	gBuildActive[playerid] = false;
	DestroyBuildPreview(playerid);
	ResetBuildPreviewState(playerid);
	SAMPP_BuildClose(playerid);
	return 1;
}

stock bool:CommitBuildDemoSlot(playerid, partid)
{
	new parent = gBuildPreviewParentFoundation[playerid];
	new slot = gBuildPreviewSlotIndex[playerid];

	switch (partid)
	{
		case SAMPP_BUILD_PART_FOUNDATION:
		{
			return RegisterBuildFoundation(playerid, gBuildPreviewX[playerid], gBuildPreviewY[playerid], gBuildPreviewZ[playerid], gBuildPreviewRZ[playerid]);
		}
		case SAMPP_BUILD_PART_WALL:
		{
			if (parent < 0 || parent >= gBuildFoundationCount[playerid] || slot < 0 || slot >= 4)
			{
				return false;
			}
			if (gBuildFoundationEdgePiece[playerid][parent][slot] != BUILD_DEMO_PIECE_NONE)
			{
				return false;
			}
			gBuildFoundationEdgePiece[playerid][parent][slot] = BUILD_DEMO_PIECE_WALL;
			return true;
		}
		case SAMPP_BUILD_PART_DOORFRAME:
		{
			if (parent < 0 || parent >= gBuildFoundationCount[playerid] || slot < 0 || slot >= 4)
			{
				return false;
			}
			if (gBuildFoundationEdgePiece[playerid][parent][slot] != BUILD_DEMO_PIECE_NONE)
			{
				return false;
			}
			gBuildFoundationEdgePiece[playerid][parent][slot] = BUILD_DEMO_PIECE_DOORFRAME;
			return true;
		}
		case SAMPP_BUILD_PART_FLOOR:
		{
			if (parent < 0 || parent >= gBuildFoundationCount[playerid])
			{
				return false;
			}
			if (gBuildFoundationTopPiece[playerid][parent] != BUILD_DEMO_PIECE_NONE)
			{
				return false;
			}
			gBuildFoundationTopPiece[playerid][parent] = BUILD_DEMO_PIECE_FLOOR;
			return true;
		}
		case SAMPP_BUILD_PART_ROOF:
		{
			if (parent < 0 || parent >= gBuildFoundationCount[playerid])
			{
				return false;
			}
			if (gBuildFoundationTopPiece[playerid][parent] != BUILD_DEMO_PIECE_NONE)
			{
				return false;
			}
			gBuildFoundationTopPiece[playerid][parent] = BUILD_DEMO_PIECE_ROOF;
			return true;
		}
		case SAMPP_BUILD_PART_STAIRS:
		{
			if (parent < 0 || parent >= gBuildFoundationCount[playerid])
			{
				return false;
			}
			if (gBuildFoundationStairsPiece[playerid][parent] != BUILD_DEMO_PIECE_NONE)
			{
				return false;
			}
			gBuildFoundationStairsPiece[playerid][parent] = BUILD_DEMO_PIECE_STAIRS;
			return true;
		}
		case SAMPP_BUILD_PART_DOOR:
		{
			if (parent < 0 || parent >= gBuildFoundationCount[playerid] || slot < 0 || slot >= 4)
			{
				return false;
			}
			if (gBuildFoundationEdgePiece[playerid][parent][slot] != BUILD_DEMO_PIECE_DOORFRAME || gBuildFoundationEdgeDoor[playerid][parent][slot])
			{
				return false;
			}
			gBuildFoundationEdgeDoor[playerid][parent][slot] = true;
			return true;
		}
	}

	return false;
}

stock PlaceBuildDemoPart(playerid, partid, rotationStep, bool:flipped)
{
	if (!gBuildActive[playerid])
	{
		return SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "Build session is not active.");
	}

	if (gBuildObjectCount[playerid] >= BUILD_DEMO_MAX_OBJECTS)
	{
		return SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "Demo object limit reached. Use /buildclear.");
	}

	gBuildSelectedPart[playerid] = partid;
	gBuildRotationStep[playerid] = rotationStep;
	gBuildFlipped[playerid] = flipped;

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

	new modelid = GetBuildPartModel(partid);
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
	if (!CommitBuildDemoSlot(playerid, partid))
	{
		DestroyObject(objectid);
		return SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_ERROR, "Build slot changed before placement. Try again.");
	}
	if (!AddBuildDemoObject(playerid, objectid))
	{
		return 1;
	}

	DestroyBuildPreview(playerid);
	RefreshBuildPreview(playerid, true);
	return SAMPP_BuildSendResult(playerid, SAMPP_BUILD_RESULT_SUCCESS, "Part placed from preview.");
}

stock SendBuildDemoHelp(playerid)
{
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] /builddemo opens a server-authoritative build menu.");
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] Select a part first; a temporary player-object preview follows your camera aim.");
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] LMB confirms the preview, RMB/ESC closes, Q/E rotates 90 degrees, MMB flips.");
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] Foundations snap to empty neighbour slots; walls snap to the nearest foundation edge.");
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
	new bool:floorRegistered = AddSimpleModel(-1, BUILD_BASE_MODEL_FLOOR, BUILD_MODEL_FLOOR, BUILD_MODEL_FLOOR_DFF, BUILD_MODEL_FLOOR_TXD);
	new bool:foundationPreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_FOUNDATION, BUILD_MODEL_FOUNDATION_PREVIEW_OK, BUILD_MODEL_FOUNDATION_DFF, BUILD_MODEL_PREVIEW_OK_TXD);
	new bool:wallPreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_WALL, BUILD_MODEL_WALL_PREVIEW_OK, BUILD_MODEL_WALL_DFF, BUILD_MODEL_PREVIEW_OK_TXD);
	new bool:doorFramePreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_DOORFRAME, BUILD_MODEL_DOORFRAME_PREVIEW_OK, BUILD_MODEL_DOORFRAME_DFF, BUILD_MODEL_PREVIEW_OK_TXD);
	new bool:floorPreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_FLOOR, BUILD_MODEL_FLOOR_PREVIEW_OK, BUILD_MODEL_FLOOR_DFF, BUILD_MODEL_PREVIEW_OK_TXD);
	new bool:roofPreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_FLOOR, BUILD_MODEL_ROOF_PREVIEW_OK, BUILD_MODEL_FLOOR_DFF, BUILD_MODEL_PREVIEW_OK_TXD);
	new bool:stairsPreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_WALL, BUILD_MODEL_STAIRS_PREVIEW_OK, BUILD_MODEL_WALL_DFF, BUILD_MODEL_PREVIEW_OK_TXD);
	new bool:doorPreviewOK = AddSimpleModel(-1, BUILD_BASE_MODEL_DOORFRAME, BUILD_MODEL_DOOR_PREVIEW_OK, BUILD_MODEL_DOORFRAME_DFF, BUILD_MODEL_PREVIEW_OK_TXD);
	new bool:foundationPreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_FOUNDATION, BUILD_MODEL_FOUNDATION_PREVIEW_BAD, BUILD_MODEL_FOUNDATION_DFF, BUILD_MODEL_PREVIEW_BAD_TXD);
	new bool:wallPreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_WALL, BUILD_MODEL_WALL_PREVIEW_BAD, BUILD_MODEL_WALL_DFF, BUILD_MODEL_PREVIEW_BAD_TXD);
	new bool:doorFramePreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_DOORFRAME, BUILD_MODEL_DOORFRAME_PREVIEW_BAD, BUILD_MODEL_DOORFRAME_DFF, BUILD_MODEL_PREVIEW_BAD_TXD);
	new bool:floorPreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_FLOOR, BUILD_MODEL_FLOOR_PREVIEW_BAD, BUILD_MODEL_FLOOR_DFF, BUILD_MODEL_PREVIEW_BAD_TXD);
	new bool:roofPreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_FLOOR, BUILD_MODEL_ROOF_PREVIEW_BAD, BUILD_MODEL_FLOOR_DFF, BUILD_MODEL_PREVIEW_BAD_TXD);
	new bool:stairsPreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_WALL, BUILD_MODEL_STAIRS_PREVIEW_BAD, BUILD_MODEL_WALL_DFF, BUILD_MODEL_PREVIEW_BAD_TXD);
	new bool:doorPreviewBad = AddSimpleModel(-1, BUILD_BASE_MODEL_DOORFRAME, BUILD_MODEL_DOOR_PREVIEW_BAD, BUILD_MODEL_DOORFRAME_DFF, BUILD_MODEL_PREVIEW_BAD_TXD);

	if (foundationRegistered)
	{
		print("[BuildDemo] Registered custom foundation model -2000 from models/foundation.dff and models/foundation.txd.");
	}
	else
	{
		print("[BuildDemo] Could not register custom foundation model -2000. Check artwork config and model files.");
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

	if (floorRegistered)
	{
		print("[BuildDemo] Registered custom floor model -2003 from models/floor.dff and models/floor.txd.");
	}
	else
	{
		print("[BuildDemo] Could not register custom floor model -2003. Check artwork config and model files.");
	}

	if (foundationPreviewOK && wallPreviewOK && doorFramePreviewOK && floorPreviewOK && roofPreviewOK && stairsPreviewOK && doorPreviewOK
		&& foundationPreviewBad && wallPreviewBad && doorFramePreviewBad && floorPreviewBad && roofPreviewBad && stairsPreviewBad && doorPreviewBad)
	{
		print("[BuildDemo] Registered green/red preview build models -2100..-2106 and -2200..-2206.");
	}
	else
	{
		print("[BuildDemo] Could not register one or more preview models. Check models/build-preview-green.txd and models/build-preview-red.txd.");
	}

	registered = foundationRegistered && wallRegistered && doorFrameRegistered && floorRegistered
		&& foundationPreviewOK && wallPreviewOK && doorFramePreviewOK && floorPreviewOK && roofPreviewOK && stairsPreviewOK && doorPreviewOK
		&& foundationPreviewBad && wallPreviewBad && doorFramePreviewBad && floorPreviewBad && roofPreviewBad && stairsPreviewBad && doorPreviewBad;
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
	gBuildRotationStep[playerid] = rotationStep;
	gBuildFlipped[playerid] = flipped;
	return RefreshBuildPreview(playerid, forceRefresh);
}

stock CancelBuildDemoSession(playerid, bool:sendMessage = false)
{
	gBuildActive[playerid] = false;
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
	if (gBuildActive[playerid])
	{
		RefreshBuildPreview(playerid);
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

	SetBuildDemoSelection(playerid, partid, 0, false, true);

	new message[96];
	format(message, sizeof message, "[BuildDemo] Selected part id %d. A preview is now active; LMB confirms it.", partid);
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, message);
	return 1;
}

public OnPlayerOMPPlusBuildPreview(playerid, sessionid, partid, rotation_step, bool:flipped)
{
	if (sessionid != GetBuildDemoSession(playerid))
	{
		return 1;
	}

	SetBuildDemoSelection(playerid, partid, rotation_step, flipped, true);
	return 1;
}

public OnPlayerOMPPlusBuildPlace(playerid, sessionid, partid, rotation_step, bool:flipped)
{
	if (sessionid != GetBuildDemoSession(playerid))
	{
		return 1;
	}

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
