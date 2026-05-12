# OpenMP-Plus Build Demo

The build demo is an experimental Rust-style base-building proof of concept.
It is intentionally server-authoritative:

```text
Client ASI
- renders the ImGui build menu
- captures mouse/keyboard while the menu is open
- sends selected part, side variant, preview, place, and cancel events
- sends remove-tool selections through the same server-authoritative build flow

Pawn / open.mp
- decides whether the player can build
- creates and updates the temporary player-object preview
- computes the final placement
- performs snap validation
- creates the real server object
- tracks placed object metadata and validates removals
```

The client never creates the real build piece. It only asks the server to place
one. The demo preview is also server-owned: Pawn uses a per-player temporary
object so the visible preview and final object are driven by the same validation
path.

Preview objects use separate open.mp custom model ids and separate green/red
TXD files. Green means the current snap slot can be placed; red means the
server already knows the slot is invalid. Confirmed placements use normal
server objects, so the final structure keeps the original custom model and
texture instead of the preview material.

## Files

```text
Build/Release/sampp_client.asi
Build/Release/omp-plus.dll
Build/sampp.inc
examples/filterscripts/sampp_builddemo.pwn
examples/filterscripts/sampp_builddemo.amx
examples/models/foundation.dff
examples/models/foundation.txd
examples/models/wall.dff
examples/models/wall.txd
examples/models/door-frame.dff
examples/models/door-frame.txd
examples/models/floor.dff
examples/models/floor.txd
examples/models/build-preview-green.txd
examples/models/build-preview-red.txd
```

Install the component, ASI, and include from the same build. Then copy
`sampp_builddemo.amx` to the server `filterscripts` folder and add it to
`pawn.side_scripts`.

Load the build demo from one script owner only. The capability demo is kept
separate from the build demo so `/builddemo` and the build callbacks are not
handled twice.

The demo foundation and wall use custom open.mp models registered with
`AddSimpleModel(-1, 19379, -2000, "foundation.dff", "foundation.txd")`.
The wall is registered with
`AddSimpleModel(-1, 19380, -2001, "wall.dff", "wall.txd")`.
The door frame and floor are registered with
`AddSimpleModel(-1, 19381, -2002, "door-frame.dff", "door-frame.txd")`
and `AddSimpleModel(-1, 19378, -2003, "floor.dff", "floor.txd")`.
The live placement preview uses separate model ids `-2100..-2106` for valid
green previews and `-2200..-2206` for invalid red previews. Copy these DFF/TXD
files to the server `models` folder and keep
`artwork.enable` set to `true` in `config.json`.

## Commands

```text
/buildhelp   Shows controls and snap rules.
/builddemo   Opens the build UI.
/build       Alias for /builddemo.
/buildclose  Closes the build UI.
/buildclear  Removes your demo build objects.
```

## Controls

```text
Left mouse      Place selected part.
Left mouse      In Remove mode, delete the aimed placed build part.
Right mouse     In placement mode, return to the part menu. In the menu, close build UI.
ESC             Close build UI.
Middle mouse    Switch side for Wall, Door Frame, and Door.
```

When right mouse returns from placement to the menu, the client sends a clear
preview event with `partid = SAMPP_BUILD_PART_NONE`. The component forwards
that event to Pawn so the temporary preview object can be destroyed without
closing the build session.

## Demo Snap Rules

Foundation placement uses a temporary preview object projected from the
player's camera aim. The first foundation is free-placed. After that, Pawn
builds a small snap graph from every placed foundation and generates four
empty neighbour slots at exact 3.0m offsets. This value comes from the demo
`foundation.dff` bounding box, which is 3.0m wide/deep. If the player's aim is close to
one of those slots, the preview locks to the slot center and inherits the
parent foundation rotation, so adjacent foundations line up exactly like
puzzle pieces. Occupied slots render red and cannot be confirmed.

Wall and Door Frame use all placed foundations as possible parents. Pawn finds
the nearest foundation edge to the player's aim and snaps the part to that edge.
Each edge has one structural slot: a Wall and Door Frame cannot occupy the same
edge. Their rotation is derived from the edge plus a calibrated model offset, so
players do not manually rotate them. Middle mouse only switches the inner/outer
surface variant.

Floor/Ceiling and Roof use the nearest foundation top snap. They share one top
slot in this demo, so one cannot be stacked on the other.

Stairs use a separate per-foundation stairs slot and face the foundation edge
nearest to the player's aim. Door placement requires an existing Door Frame edge
and is rejected if that frame already has a door. Middle mouse changes the door
opening side.

## Remove Tool

The menu includes `Tools > Remove`. This is a special build part
(`SAMPP_BUILD_PART_REMOVE`) rather than a real model. Selecting it enters an
orange remove mode with no placement preview. Left mouse sends a normal
`OnPlayerOMPPlusBuildPlace` event with the remove part id; Pawn then ray-checks
the player's camera aim against the server-owned build object metadata.

The demo removes only validated placed parts. Door Frames with installed Doors
must have the Door removed first. Foundations can be removed only when their
edge, top, stairs, and door slots are empty. This keeps the snap graph coherent
and avoids accidental cascade deletes in the demo.

## Pawn API Surface

```pawn
native SAMPP_BuildOpen(playerid, sessionid, const title[], Float:max_distance = 8.0);
native SAMPP_BuildClose(playerid);
native SAMPP_BuildClearParts(playerid);
native SAMPP_BuildAddPart(playerid, partid, modelid, const name[], const category[] = "", const cost[] = "");
native SAMPP_BuildSendResult(playerid, result, const message[]);

forward OnPlayerOMPPlusBuildSelect(playerid, sessionid, partid);
forward OnPlayerOMPPlusBuildPlace(playerid, sessionid, partid, rotation_step, bool:flipped);
forward OnPlayerOMPPlusBuildCancel(playerid, sessionid);
```

Use `SAMPP_HasFeature(playerid, SAMPP_FEATURE_BUILD)` or the helper
`SAMPP_BuildHasUI(playerid)` before opening the build UI.

## Production Guidance

For a real base-building system, keep these rules:

- Treat every client build request as untrusted.
- Use a session id and reject stale requests.
- Recompute snap positions server-side.
- Check distance, world/interior, claim ownership, materials, cooldowns, object
  limits, and collision before creating an object.
- Store permanent object data in a database using your own stable object id,
  not only the runtime object id.
- Use this demo as the transport/UI layer, not as a complete base persistence
  system.
