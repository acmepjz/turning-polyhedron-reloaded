#pragma once

/** \file
*/

#include <SDL.h>

/** Create an SDL_Color. */
inline SDL_Color SDL_MakeColor(Uint8 r,Uint8 g,Uint8 b,Uint8 a){
	SDL_Color clr={r,g,b,a};
	return clr;
}
