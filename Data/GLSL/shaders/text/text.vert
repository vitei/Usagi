#include "../includes/platformdefines.inc"
#include "../includes/global_2d.inc"
#include "../includes/colorspace.inc"

// Input attributes
layout (location = 0) in vec4 ao_posRange;
layout (location = 1) in float ao_fDepth;
layout (location = 2) in vec4 ao_uvRange;
layout (location = 3) in vec4 ao_colUpper;
layout (location = 4) in vec4 ao_colLower;
layout (location = 5) in vec4 ao_colBg;
layout (location = 6) in vec4 ao_colFg;


// Output attributes
ATTRIB_LOC(0) out vec4 vo_posTL;
ATTRIB_LOC(1) out vec4 vo_posTR;
ATTRIB_LOC(2) out vec4 vo_posBL;
ATTRIB_LOC(3) out vec4 vo_posBR;
ATTRIB_LOC(4) out vec4 vo_vColorUpper;
ATTRIB_LOC(5) out vec4 vo_vColorLower;
ATTRIB_LOC(6) out vec4 vo_vBgColor;
ATTRIB_LOC(7) out vec4 vo_vFgColor;
ATTRIB_LOC(8) out vec4 vo_vUVRange;


void main(void)
{
 	vo_posTL = vec4( ao_posRange.xy, ao_fDepth, 1.0 ) * proj;
 	vo_posTR = vec4( ao_posRange.zy, ao_fDepth, 1.0 ) * proj;
 	vo_posBL = vec4( ao_posRange.xw, ao_fDepth, 1.0 ) * proj;
 	vo_posBR = vec4( ao_posRange.zw, ao_fDepth, 1.0 ) * proj;
 	
	vo_vColorUpper 		= toLinear(ao_colUpper);
	vo_vColorLower 		= toLinear(ao_colLower);		
	vo_vBgColor			= toLinear(ao_colBg);
	vo_vFgColor			= toLinear(ao_colFg);
	vo_vUVRange.xy  	= GetUV(ao_uvRange.xy);
	vo_vUVRange.zw  	= GetUV(ao_uvRange.zw);
}
