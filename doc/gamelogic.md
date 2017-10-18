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

<table>
<tr><th> Offset <th> Type    <th> Value      <th> Remark
<tr><td> +0     <td> char*8  <td> `D2 A1 B7 BD BF E9 58 50` <td>
Signature. Don't rely on it because the game saves file with garbage signature on Windows with non CP-936 code page.
<tr><td> +8     <td> int     <td> <i>n</i>                  <td>
* >=0 means the file is uncompressed and the data starts from +8 (therefore this number is node count);
* otherwise <i>-n</i> means size of uncompressed data and compressed data starts from +12.
</table>

Uncompressed data:

<table>
<tr><th> Offset <th> Type    <th> Value      <th> Remark
<tr><td> +0     <td> int     <td> Node count <td> Always 1 in TurningSquare level file.
<tr><td colspan="4"> For each node:
	<table>
	<tr><th> Offset <th> Type    <th> Value      <th> Remark
	<tr><td> node+0 <td> char*4  <td> Node type  <td> Always <code>"LEV\0"</code> in TurningSquare level file.
	<tr><td> node+4 <td> int     <td> Item count <td> Means level count in TurningSquare.
	<tr><td colspan="4"> For each item:
		<table>
		<tr><th> Offset <th> Type    <th> Value      <th> Remark
		<tr><td> item+0 <td> int     <td> Item size  <td> --
		<tr><td> item+4 <td> char*(Item size) <td> Item data <td> Level data in TurningSquare.
		</table>
	</table>
</table>

Level Data:

<table>
<tr><th> Offset <th> Type    <th> Value      <th> Remark
<tr><td> +0     <td> int     <td> Width <td> --
<tr><td> +4     <td> int     <td>Height <td> --
<tr><td> +8     <td> int     <td>StartX <td> 1-based
<tr><td> +12    <td> int     <td>StartY <td> 1-based
<tr><td> +16    <td>char*(w*h) <td>dat  <td> Array of tile types
<tr><td> +?     <td>int*(w*h)  <td>dat2 <td>
* If tile type is button (i.e. 2 or 3) then it is index of switch data for this button (1-based);
* if tile type is teleporter (i.e. 4) then bit 0-7, 8-15, 16-23, 24-31 are x1, y1, x2, y2, respectively (all 1-based).
<tr><td> +?     <td> int     <td>Switch count <td> --
<tr><td colspan="4"> For each switch:
	<table>
	<tr><th> Offset <th> Type    <th> Value      <th> Remark
	<tr><td> switch+0 <td> int   <td>Bridge count <td> --
	<tr><td colspan="4"> For each bridge:
		<table>
		<tr><th> Offset <th> Type    <th> Value      <th> Remark
		<tr><td> bridge+0 <td> int   <td> x     <td> 1-based
		<tr><td> bridge+4 <td> int   <td> y     <td> 1-based
		<tr><td> bridge+8 <td> int   <td>Behavior     <td> 0=off 1=on 2=toggle
		</table>
	</table>
</table>

Tiles {#a2}
-----

Found in clsBloxorz.cls:38.

<table>
<tr><th> Value <th> Name       <th> Remark
<tr><td> 0     <td>Empty       <td> --
<tr><td> 1     <td>Ground      <td> --
<tr><td> 2     <td>Soft button <td> Activated when any part of the block presses it.
<tr><td> 3     <td>Hard button <td> Activated only when the block <b>standing</b> on it.
<tr><td> 4     <td>Teleporter  <td> Teleporter will teleports your block to different locations, optionally (always in Bloxorz) splitting it into two smaller blocks at the same time.
<tr><td> 5     <td>Thin ground <td> If your block <b>stands</b> up vertically on it, the tile will give way and your block will fall.
<tr><td> 6     <td>Bridge (off)<td> --
<tr><td> 7     <td>Bridge (on) <td> --
<tr><td> 8     <td>Goal        <td> You'll win the game if you get your block to fall into this square hole.
<tr><th colspan="3"> New tile types not presented in Bloxorz:
<tr><td> 9     <td>Ice         <td> If the block is not rough enough and is completely on the ice, it will slip until get off the ice or hit the wall.
<tr><td> 10    <td>Pyramid     <td> Your block is unstable when standing on the pyramid, it will lie down immediately unless there is a wall next to your block.
<tr><td> 11    <td>Wall        <td> As an obstacle, your block can't pass through the wall, but it can lean against the wall and move around. In a word, a 1x1x1 solid block.
</table>

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
blocked   | false    | Make the map solid at y+1 to y+(Block height) ??? (deprecated!!! use \ref game::TileType::blockedArea instead)
block-height|infinity| Block height ??? (deprecated!!! use \ref game::TileType::blockedArea instead)
tilt-supporter|true  | Determine if block can lean against this tile.
supporter | true     | Determine if block can be on this tile.
non-block | false    | Make the map transparent at the position of this tile. (deprecated!!! use \ref game::TileType::blockedArea instead)
checkpoint| false    | Count this tile as a checkpoint.
invisibleAtRuntime|false| deprecated!!! use node mask instead
elevator  | false    | deprecated!!! use \ref game::Polyhedron::controller instead

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

