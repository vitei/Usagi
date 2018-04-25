#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/layout_3d_vs.inc"

// Input attributes
layout (location = 0) in vec4 ao_posRange;
layout (location = 1) in float ao_fDepth;
layout (location = 2) in vec4 ao_uvRange;
layout (location = 3) in vec4 ao_colUpper;
layout (location = 4) in vec4 ao_colLower;
layout (location = 5) in vec4 ao_colBg;


// Output attributes
out vec3 vo_positionTL;
out vec3 vo_positionTR;
out vec3 vo_positionBL;
out vec3 vo_positionBR;
out vec4 vo_vColorUpper;
out vec4 vo_vColorLower;
out vec4 vo_vColorBg;
out vec4 vo_vTexCoordRange;

// 3d position data
out float vo_fVisiblity;
out vec4 vo_transformedPos;

void main(void)
{
    vo_fVisiblity = GetPosAndVisibility(vo_transformedPos);

	vo_positionTL = vec3( ao_posRange.xy, ao_fDepth );
	vo_positionTR = vec3( ao_posRange.zy, ao_fDepth );
	vo_positionBL = vec3( ao_posRange.xw, ao_fDepth );
	vo_positionBR = vec3( ao_posRange.zw, ao_fDepth );

	vo_vColorUpper 		= ao_colUpper;	
	vo_vColorLower 		= ao_colLower;	
	vo_vColorBg			= ao_colBg;
	
	vo_vTexCoordRange.xy	= GetUV(ao_uvRange.xy);
	vo_vTexCoordRange.zw	= GetUV(ao_uvRange.zw);
}
