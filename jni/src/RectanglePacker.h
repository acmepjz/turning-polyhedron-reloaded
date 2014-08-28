#pragma once

/** \file
*/

#include <vector>

/** A strucure represent a rectangle used in rectangle packer. */

struct RectanglePackerRectangle{
	short index;///< [out] The index of sprite sheet.
	short priority;///< [in] The priority.
	short x,y;///< [out] The position of sprite.
	short w,h;///< [in] The size of sprite.
};

/** A simple rectangle packer for sprite sheet generation.
*/

class RectanglePacker{
public:
	/** Pack retangles.
	\param width The rectangle width.
	\param height The rectangle height.
	\return The total height of generated sprite sheets.
	Largr than height means multiple sprite sheets.
	-1 means failed, for example some rectangle's size exceeds the size of a sprite sheet.
	*/
	int pack(int width,int height);
private:
	/** Internal function.
	\return Height of generated sprite sheets. >=0 means finished all rectangles.
	-1 means there are still some rectangles not packed.
	*/
	static int packOnePage(int width,int height,int index,std::vector<RectanglePackerRectangle*>& rects);
public:
	/** The rectangles to pack. */
	std::vector<RectanglePackerRectangle> rectangles;
};
