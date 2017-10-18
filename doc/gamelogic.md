Game Logic XML Reference
========================

[objectTypes]: #objectTypes
[objectType]: #objectType
[tileTypes]: #tileTypes
[tileType]: #tileType
[level]: #level
[levelCollection]: #levelCollection

[TOC]

These are some game logic XML node specification implemented in Turning Polyhedron Reloaded
as well as some unimplemented one used in TurningPolyhedron.

File Structure {#FileStructure}
==============

File | Root node
-----|-----------
DefaultObjectTypes.xml | [objectTypes]
DefaultTileTypes.xml | [tileTypes]
level file | [level] or [levelCollection]

XML Nodes {#a3}
=================

objectTypes XML node {#objectTypes}
------------

This node only contains [objectType] subnode.

objectType XML node {#objectType}
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


tileTypes XML node {#tileTypes}
------------

This node only contains [tileType] subnode.

tileType XML node {#tileType}
------------

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
on???            |Any    | Events

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

