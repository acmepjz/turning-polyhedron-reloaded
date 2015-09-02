Game Logic Document
===================

[TOC]

Blah blah blah...

Good old TurningSquare {#a1}
======================

The block is 1x1x2.

File format {#a11}
-----------

The file extension is `box`.
All numbers are little-endian. The compress algorithm is a version of LZSS written by Haruhiko Okumura.
See also clsTheFile.cls and clsBloxorz.cls and clsLZSS_ASM.bas.

Offset | Type    | Value | Remark
-------|---------|-------|--------------
+0     | char*8  | `D2 A1 B7 BD BF E9 58 50` | Signature. Don't rely on it because the game saves file with garbage signature on Windows with non CP-936 code page.
+8     | int     | <i>n</i>                  | >=0 means the file is uncompressed and the data starts from +8 (therefore this number is node count); otherwise <i>-n</i> means size of uncompressed data and compressed data starts from +12.

Uncompressed data:

Offset | Type    | Value | Remark
-------|---------|-------|--------------
+0     | int     | Node count | Always 1 in TurningSquare level file.
--     | --      | --    | For each node:
node+0 | char*4  | Node type  | Always <code>"LEV\0"</code> in TurningSquare level file.
node+4 | int     | Item count | Means level count in TurningSquare.
--     | --      | --    | For each item:
item+0 | int     | Item size  | --
item+4 | char*(Item size) | Item data | Level data in TurningSquare.

Level Data:

Offset | Type    | Value | Remark
-------|---------|-------|--------------
+0     | int     | Width | --
+4     | int     |Height | --
+8     | int     |StartX | 1-based
+12    | int     |StartY | 1-based
+16    |char*(w*h) |dat  | Array of tile types
+?     |int*(w*h)  |dat2 | If tile type is button (i.e. 2 or 3) then it is index of switch data for this button (1-based); if tile type is teleporter (i.e. 4) then bit 0-7, 8-15, 16-23, 24-31 are x1, y1, x2, y2, respectively (all 1-based).
+?     | int     |Switch count | --
--     | --      | --    | For each switch:
switch+0 | int   |Bridge count | --
--     | --      | --    | For each bridge:
bridge+0 | int   | x     | 1-based
bridge+4 | int   | y     | 1-based
bridge+8 | int   |Behavior     | 0=off 1=on 2=toggle

Tiles {#a2}
-----

Found in clsBloxorz.cls:38.

Value | Name       | Remark
------|------------|-------------
0     |Empty       | --
1     |Ground      | --
2     |Soft button | Activated when any part of the block presses it.
3     |Hard button | Activated only when the block <b>standing</b> on it.
4     |Teleporter  | Teleporter will teleports your block to different locations, optionally (always in Bloxorz) splitting it into two smaller blocks at the same time.
5     |Thin ground | If your block <b>stands</b> up vertically on it, the tile will give way and your block will fall.
6     |Bridge (off)| --
7     |Bridge (on) | --
8     |Goal        | You'll win the game if you get your block to fall into this square hole.
--    | --         | New tile types not presented in Bloxorz:
9     |Ice         | If the block is not rough enough and is completely on the ice, it will slip until get off the ice or hit the wall.
10    |Pyramid     | Your block is unstable when standing on the pyramid, it will lie down immediately unless there is a wall next to your block.
11    |Wall        | As an obstacle, your block can't pass through the wall, but it can lean against the wall and move around. In a word, a 1x1x1 solid block.

TurningPolyhedron {#a3}
=================

Object types {#a3b}
------------

Object type is extensible but almost nothing implemented. (???)

### Examples {#a3ba}

Found in DefaultObjectTypes.xml.

Name      | Remark
----------|-------------
(default) | --
black     | --
white     | --
ice       | slippery unless +fire=melting +sand=moveable
fire      | --
sand      | --
wood      | +fire=burn
rough-wood| +fire=burn +ice=moveable

### Format {#a3bb}

The node named `objectType`.

Attribute | Remark
----------|----------
name      | The name.

Subnode          |Count  | Remark
------------     |-----  |--------
interaction      |0 or 1 | Has many attributes like <i>type</i>=<i>interaction</i>, where <i>type</i> is the type of the second object or `default` for all other unspecified types. <i>interaction</i> is interaction type. Priority blah blah...
multiInteraction |0 or 1 | ??? Unimplemented ??? Has attributes named <code>[<type1>-](onSameType\|onDifferentType\|<type2>)</code> ???

### Interaction types {#a3a}

Found in clsGameManager::ParseInteraction (clsGameManager.cls:2732).
Almost nothing implemented. (???)

Name      | Remark
----------|-------------
moveable  | Default interaction type.
not-moveable   | ??? The block can't move as if it is glued ???
slippery       | Slip when object completely on 'slippery' block or any part of it on 'superSlippery' block.
superSlippery  | Slip when object completely on 'slippery' block or any part of it on 'superSlippery' block.
blocked        | ???
game-over      | Various sub-types, e.g. meltihg, burn, etc. Not necessarily game over, though; it just destroys coressponding object.

Tiles {#a3c}
-----

Tile types in TurningPolyhedron are extensible.

### Examples {#a3ca}

These are only some examples found in DefaultTileTypes.xml.
Some of them won't be implemented in Turning Polyhedron Reloaded.
Note that the `id` is case-sensitive.

Value  | id         | Name        | Remark
-------|------      |-------      |----------
0      | --         | Empty       | Hard-coded, not present in DefaultTileTypes.xml.
1      |ground      |Ground       | --
--     |block-ground| --          | Ground with different appearance.
2      |soft-button |Soft button  | --
3      |hard-button |Hard button  | --
4      |teleporter  |Teleporter   | --
5      |thin-ground |Thin ground  | --
6      |bridge-off  |Bridge (off) | --
7      |bridge-on   |Bridge (on)  | --
8      |goal        |Goal         | You'll win the game if you get your block to fall into this square hole <b>after visiting all checkpoints</b>.
--     |floating-goal  | --       | Non-block goal floating in the air.
--     |checkpoint  |Checkpoint   | Disappear after you visited it. Appeared in Ice Princess.
9      |ice         |Ice          | --
10     |pyramid     |Pyramid      | --
11     |wall        |Wall         | --
--     |one-time |Very thin ground| Break down immediately after you move on it. Appeared in Ice Princess.
--     |tricky      |Tricky ground| After you <b>stand</b> on a tricky ground, it will turn to thin ground. Appeared in Ice Princess.
--     |strange  |Strange ground  | The block can move onto a strange ground only if it stands upright on it.
--     |black    |Black ground    | Only black block can pass through black ground.
--     |white    |White ground    | Only white block can pass through white ground.
--     |color-undefined | Color-undefined ground | Change to the block's color after a black or white block is leaving it.
--     |color-inverse   | Color-inverse ground   | Change to the color opposite to the block's color after a black or white block is leaving it.
--     |BLUE, RED, GREEN, YELLOW, CYAN, MAGENTA, WHITE, BLACK, ... | -- | Various colored block used in color zone mazes.

### Format {#a3cb}

The node named `tileType`.

Attribute |  Default | Remark
----------|----------|--------
id        | --       | The string identifier of this tile.
index    |(undefined)| The index of this tile.
type      | default  | Object type of this tile.
blocked   | false    | Make the map solid at y+1 to y+(Block height) ??? (deprecated!!!)
block-height|infinity| Block height ??? (deprecated!!!)
tilt-supporter|true  | Determine if block can lean against this tile.
supporter | true     | Determine if block can be on this tile.
non-block | false    | Make the map transparent at the position of this tile.
checkpoint| false    | Count this tile as a checkpoint.
invisibleAtRuntime|false| ??? deprecated ???
elevator  | false    | ??? unimplemented ???

Subnode          |Count  | Remark
------------     |-----  |--------
name             |0 or 1 | gettext'ed tile name
description      |0 or 1 | gettext'ed tile description
appearance       |???    | Out of scope of this document
on***            |Any    | Events

### Events {#a3cc}

Blah blah blah...

### Unimplemented ideas {#a3cz}

* Conveyor belts
* Buttons on the edge or vertex (soft, hard)
* Colored pieces on the ground or on the polyhedron, blah blah...
* Items

Blah blah blah...

Polyhedron {#a3d}
----------

Blah blah blah...

Turning Polyhedron Reloaded {#a4}
=================

Object Types:

* Map
* Polyhedron
* Pushable Block/Target Block
* Rotate Block
* Puzzle Boy
* Laser ???

