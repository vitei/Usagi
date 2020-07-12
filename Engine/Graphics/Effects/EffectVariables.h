/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_EFFECT_VARIABLES_H_
#define _USG_GRAPHICS_EFFECT_VARIABLES_H_


namespace usg {

struct Vector4b
{
	bool	x;
	bool	y;
	bool	z;
	bool	w;
};

struct Vector4i
{
	int		x;
	int		y;
	int		z;
	int		w;
};

struct Vector2i
{
	Vector2i() {}
	Vector2i(int xIn, int yIn) { x = xIn; y = yIn; }
	int		x;
	int		y;
};

}

#endif
