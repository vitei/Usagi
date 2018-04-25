// Includes.
#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VertexData
{
	vec3 vo_vStart;
	vec3 vo_vEnd;
	vec2 vo_vWidth;
	vec4 vo_vColor;
} VertexIn[];

out GeometryData
{
    vec4 	vo_vColor;
} geometryData;

vec3 lightDir = vec3(-0.7, 0.4, 0.7);
float amb = 0.3;

void MakeVertex(vec3 orig, vec3 offs, vec3 norm, vec4 clr)
{
	//vec3 wNorm = norm * mat3(mModelMat);

	vec4 vWorldPos = vec4(orig + offs, 1.0);
	vec4 vViewPos = vWorldPos * mViewMat;
	vec4 vProjPos = vec4(vViewPos.xyz, 1.0) * mProjMat;
	gl_Position = vProjPos;

	geometryData.vo_vColor = clr;

	EmitVertex();
}

void MakeLine(int ii)
{
	vec3 start = VertexIn[ii].vo_vStart;
	vec3 end = VertexIn[ii].vo_vEnd;
	float start_width = VertexIn[ii].vo_vWidth.x;
	float end_width = VertexIn[ii].vo_vWidth.y;
	vec4 clr = VertexIn[ii].vo_vColor;

	MakeVertex(start, vec3(0, start_width, 0), vec3(0,0,1), clr);
	MakeVertex(end, vec3(0, end_width, 0), vec3(0,0,1), clr);
	MakeVertex(start, vec3(0, -start_width, 0), vec3(0,0,1), clr);
	MakeVertex(end, vec3(0, -end_width, 0), vec3(0,0,1), clr);
	EndPrimitive();
}

void main()
{
	for(int i = 0; i < gl_in.length(); i++)
	{
		MakeLine(i);
	}
}
