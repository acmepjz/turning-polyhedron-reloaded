TurningSquare file format
=========================

The block is 1x1x2.

File format
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

Tiles
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
