#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"


// Input/ output definitions
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;


BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	mat4	mProj;
	// FIXME: Should be vec2 but not supported yet
	vec4	vCharSize;	
	vec4	vTexCoordRng;	
};



in VertexData
{
    vec4    vo_vColor;
    vec2    vo_vTexCoord;

} vertexData[];


out GeometryData
{
    vec4 	vo_vColor;
    vec2 	vo_vTexcoord;

} geometryData;


void CreateVertex(int ii, vec2 vTexCoord, vec4 pos)
{
	//vec4 pos;
  
	//pos = vec4(vPos, 0.0, 1.0);
	geometryData.vo_vColor = vertexData[ii].vo_vColor;

	geometryData.vo_vTexcoord = vTexCoord;

	//pos = pos * mProj;

	gl_Position = pos;

	EmitVertex();
}

void main(void)
{
    for (int ii = 0; ii < gl_in.length(); ii++)
    {
        vec2 vTLPos = gl_in[ii].gl_Position.xy;
        vec2 vSize = vCharSize.xy;
        vec4 vTexCoordMinMax;
        vTexCoordMinMax = vertexData[ii].vo_vTexCoord.xyxy + vTexCoordRng;

        vec4 vPos = vec4(vTLPos, 0.0, 1.0) * mProj;

        CreateVertex(ii, vTexCoordMinMax.xw, vPos + vCharSize.xyzz);
        CreateVertex(ii, vTexCoordMinMax.zw, vPos + vCharSize.xzzz );        
        CreateVertex(ii, vTexCoordMinMax.xy, vPos + vCharSize.zyzz); 
        CreateVertex(ii, vTexCoordMinMax.zy, vPos); 
        
        EndPrimitive();
    }
}
