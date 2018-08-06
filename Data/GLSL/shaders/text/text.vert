#include "../includes/platformdefines.inc"
#include "../includes/global_2d.inc"

// Input attributes
layout (location = 0) in vec4 ao_posRange;
layout (location = 1) in float ao_fDepth;
layout (location = 2) in vec4 ao_uvRange;
layout (location = 3) in vec4 ao_colUpper;
layout (location = 4) in vec4 ao_colLower;
layout (location = 5) in vec4 ao_colBg;
layout (location = 6) in vec4 ao_colFg;


// Output attributes
out VertexData
{
	INT_LOC(0) vec4 vo_posTL;
    INT_LOC(1) vec4 vo_posTR;
    INT_LOC(2) vec4 vo_posBL;
    INT_LOC(3) vec4 vo_posBR;
    INT_LOC(4) vec4 vo_vColorUpper;
    INT_LOC(5) vec4 vo_vColorLower;
    INT_LOC(6) vec4 vo_vBgColor;
    INT_LOC(7) vec4 vo_vFgColor;
    INT_LOC(8) vec4 vo_vUVRange;
} vertexData;


void main(void)
{
 	vertexData.vo_posTL = vec4( ao_posRange.xy, ao_fDepth, 1.0 ) * proj;
 	vertexData.vo_posTR = vec4( ao_posRange.zy, ao_fDepth, 1.0 ) * proj;
 	vertexData.vo_posBL = vec4( ao_posRange.xw, ao_fDepth, 1.0 ) * proj;
 	vertexData.vo_posBR = vec4( ao_posRange.zw, ao_fDepth, 1.0 ) * proj;
 	
	vertexData.vo_vColorUpper 		= ao_colUpper;
	vertexData.vo_vColorLower 		= ao_colLower;		
	vertexData.vo_vBgColor			= ao_colBg;
	vertexData.vo_vFgColor			= ao_colFg;
	vertexData.vo_vUVRange.xy  	= GetUV(ao_uvRange.xy);
	vertexData.vo_vUVRange.zw  	= GetUV(ao_uvRange.zw);
}
