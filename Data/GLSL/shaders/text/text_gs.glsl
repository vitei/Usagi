#include "../includes/platformdefines.inc"
#include "../includes/global_2d.inc"


// Input/ output definitions
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;



in VertexData
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

} vertexData[];


out GeometryData
{
    vec2    go_vPos;
    vec4 	go_vColor;
    vec4    go_vBgColor;
    vec2 	go_vTexcoord;

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
