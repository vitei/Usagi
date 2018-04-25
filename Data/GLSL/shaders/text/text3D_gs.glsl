#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/layout_3d_gs.inc"

// Input triangles
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;


// Input attributes
in vec3 vo_positionTL[];
in vec3 vo_positionTR[];
in vec3 vo_positionBL[];
in vec3 vo_positionBR[];
in vec4 vo_vColorUpper[];
in vec4 vo_vColorLower[];
in vec4 vo_vColorBg[];
in vec4 vo_vTexCoordRange[];

// 3d position data
in float vo_fVisiblity[];
in vec4 vo_transformedPos[];

// Output attributes
out vec2 go_vPos;
out vec4 go_vColor;
out vec4 go_vBgColor;
out vec2 go_vTexCoord;

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
