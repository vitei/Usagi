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
	vec4 vo_posTL;
	vec4 vo_posTR;
	vec4 vo_posBL;
	vec4 vo_posBR;
	vec4 vo_vColorUpper;
	vec4 vo_vColorLower;
	vec4 vo_vBgColor;
	vec4 vo_vFgColor;
	vec4 vo_vUVRange;
} vertexData;


void main(void)
{
	vec4 vTL = vec4( ao_posRange.xy, ao_fDepth, 1.0 );
	vec4 vTR = vec4( ao_posRange.zy, ao_fDepth, 1.0 );
	vec4 vBL = vec4( ao_posRange.xw, ao_fDepth, 1.0 );
	vec4 vBR = vec4( ao_posRange.zw, ao_fDepth, 1.0 );

 	vertexData.vo_posTL = vTL * proj;
 	vertexData.vo_posTR = vTR * proj;
 	vertexData.vo_posBL = vBL * proj;
 	vertexData.vo_posBR = vBR * proj;
 	
	vertexData.vo_vColorUpper 		= ao_colUpper;
	vertexData.vo_vColorLower 		= ao_colLower;		
	vertexData.vo_vBgColor			= ao_colBg;
	vertexData.vo_vFgColor			= ao_colFg;
	vertexData.vo_vUVRange.xy  	= GetUV(ao_uvRange.xy);
	vertexData.vo_vUVRange.zw  	= GetUV(ao_uvRange.zw);
}
