#include "../includes/platformdefines.inc"
#include "../includes/global_2d.inc"


// Input/ output definitions
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;



in VertexData
{
    ATTRIB_LOC(0) vec4 vo_posTL;
    ATTRIB_LOC(1) vec4 vo_posTR;
    ATTRIB_LOC(2) vec4 vo_posBL;
    ATTRIB_LOC(3) vec4 vo_posBR;
    ATTRIB_LOC(4) vec4 vo_vColorUpper;
    ATTRIB_LOC(5) vec4 vo_vColorLower;
    ATTRIB_LOC(6) vec4 vo_vBgColor;
    ATTRIB_LOC(7) vec4 vo_vFgColor;
    ATTRIB_LOC(8) vec4 vo_vUVRange;

} vertexData[];


out GeometryData
{
    ATTRIB_LOC(0) vec2  go_vPos;
    ATTRIB_LOC(1) vec4 	go_vColor;
    ATTRIB_LOC(2) vec4  go_vBgColor;
    ATTRIB_LOC(3) vec2 	go_vTexcoord;

} geometryData;


void CreateVertex(vec2 vTexCoord, vec4 pos, vec4 color)
{
	geometryData.go_vColor = vertexData[0].vo_vFgColor * color;

    geometryData.go_vBgColor = vertexData[0].vo_vBgColor * color;
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
        vTexCoordMinMax = vertexData[ii].vo_vUVRange;

        CreateVertex(vTexCoordMinMax.zy, vertexData[ii].vo_posBR, vertexData[ii].vo_vColorLower);
        CreateVertex(vTexCoordMinMax.xy, vertexData[ii].vo_posBL,vertexData[ii].vo_vColorLower);        
        CreateVertex(vTexCoordMinMax.zw, vertexData[ii].vo_posTR,vertexData[ii].vo_vColorUpper); 
        CreateVertex(vTexCoordMinMax.xw, vertexData[ii].vo_posTL,vertexData[ii].vo_vColorUpper); 
        
        EndPrimitive();
    }
}
