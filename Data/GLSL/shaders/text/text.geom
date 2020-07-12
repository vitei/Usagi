#include "../includes/platformdefines.inc"
#include "../includes/global_2d.inc"


// Input/ output definitions
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;


ATTRIB_LOC(0) in vec4 vo_posTL[];
ATTRIB_LOC(1) in vec4 vo_posTR[];
ATTRIB_LOC(2) in vec4 vo_posBL[];
ATTRIB_LOC(3) in vec4 vo_posBR[];
ATTRIB_LOC(4) in vec4 vo_vColorUpper[];
ATTRIB_LOC(5) in vec4 vo_vColorLower[];
ATTRIB_LOC(6) in vec4 vo_vBgColor[];
ATTRIB_LOC(7) in vec4 vo_vFgColor[];
ATTRIB_LOC(8) in vec4 vo_vUVRange[];


out GeometryData
{
    INT_LOC(0) vec2  go_vPos;
    INT_LOC(1) vec4 	go_vColor;
    INT_LOC(2) vec4  go_vBgColor;
    INT_LOC(3) vec2 	go_vTexcoord;

} geometryData;


void CreateVertex(vec2 vTexCoord, vec4 pos, vec4 color, int ii)
{
	geometryData.go_vColor = vo_vFgColor[ii] * color;

    geometryData.go_vBgColor = vo_vBgColor[ii] * color;
	geometryData.go_vTexcoord = vTexCoord;

	gl_Position = pos;
    geometryData.go_vPos = pos.xy;

	EmitVertex();
}

void main(void)
{
    for (int ii = 0; ii < gl_in.length(); ii++)
    {
        vec4 vTexCoordMinMax;
        vTexCoordMinMax = vo_vUVRange[ii];

        CreateVertex(vTexCoordMinMax.zy, vo_posBR[ii], vo_vColorLower[ii], ii);
        CreateVertex(vTexCoordMinMax.xy, vo_posBL[ii],vo_vColorLower[ii], ii);        
        CreateVertex(vTexCoordMinMax.zw, vo_posTR[ii],vo_vColorUpper[ii], ii); 
        CreateVertex(vTexCoordMinMax.xw, vo_posTL[ii],vo_vColorUpper[ii], ii); 
        
        EndPrimitive();
    }
}
