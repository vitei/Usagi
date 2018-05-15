#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/layout_3d_gs.inc"

// Input triangles
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;


// Input attributes
ATTRIB_LOC(0) in vec3 vo_positionTL[];
ATTRIB_LOC(1) in vec3 vo_positionTR[];
ATTRIB_LOC(2) in vec3 vo_positionBL[];
ATTRIB_LOC(3) in vec3 vo_positionBR[];
ATTRIB_LOC(4) in vec4 vo_vColorUpper[];
ATTRIB_LOC(5) in vec4 vo_vColorLower[];
ATTRIB_LOC(6) in vec4 vo_vColorBg[];
ATTRIB_LOC(7) in vec4 vo_vTexCoordRange[];

// 3d position data
ATTRIB_LOC(8) in float vo_fVisiblity[];
ATTRIB_LOC(9) in vec4 vo_transformedPos[];

// Output attributes
ATTRIB_LOC(0) out vec2 go_vPos;
ATTRIB_LOC(1) out vec4 go_vColor;
ATTRIB_LOC(2) out vec4 go_vBgColor;
ATTRIB_LOC(3) out vec2 go_vTexCoord;

void CreateVertex(vec3 vLayoutPos, vec2 vTexCoord, vec4 color)
{
	vec4 vPosition = vec4( vLayoutPos, 1.0) * proj;
 	vPosition.xy += vo_transformedPos[0].xy;
 	vPosition.z = vo_transformedPos[0].z;
 	vPosition.w = 1.0;

	gl_Position		= vPosition;
	go_vPos = vPosition.xy;	

 	vec4 vColor = color;
 	vColor.a *= vo_fVisiblity[0];
	go_vColor 		= vColor;	
	go_vTexCoord	= vTexCoord;
	go_vBgColor = vo_vColorBg[0];

	EmitVertex();
}

void main(void)
{
    for (int ii = 0; ii < gl_in.length(); ii++)
    {
   		CreateVertex(vo_positionBR[ii], vo_vTexCoordRange[ii].zy, vo_vColorLower[ii]);
 		CreateVertex(vo_positionBL[ii], vo_vTexCoordRange[ii].xy, vo_vColorLower[ii]);
 		CreateVertex(vo_positionTR[ii], vo_vTexCoordRange[ii].zw, vo_vColorUpper[ii]);
 		CreateVertex(vo_positionTL[ii], vo_vTexCoordRange[ii].xw, vo_vColorUpper[ii]);
 		EndPrimitive();
	}
}
