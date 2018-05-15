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
ATTRIB_LOC(0) out vec3 vo_positionTL;
ATTRIB_LOC(1) out vec3 vo_positionTR;
ATTRIB_LOC(2) out vec3 vo_positionBL;
ATTRIB_LOC(3) out vec3 vo_positionBR;
ATTRIB_LOC(4) out vec4 vo_vColorUpper;
ATTRIB_LOC(5) out vec4 vo_vColorLower;
ATTRIB_LOC(6) out vec4 vo_vColorBg;
ATTRIB_LOC(7) out vec4 vo_vTexCoordRange;

// 3d position data
ATTRIB_LOC(8) out float vo_fVisiblity;
ATTRIB_LOC(9) out vec4 vo_transformedPos;

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
