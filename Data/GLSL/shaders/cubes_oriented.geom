#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"

layout(points) in;
layout (triangle_strip, max_vertices=24) out;


in VertexData {
    mat3x4  vo_matrix;
    vec4    vo_vColor;
} VertexIn[];


out GeometryData
{
    vec4 	vo_vColor;
} geometryData;


vec3 lightDir = vec3(-0.7, 0.4, 0.7);
float amb = 0.3;
float degToRad = 0.0174532925;

void MakeVertex(vec3 orig, vec4 clr, vec3 norm)
{
	vec4 vWorldPos = vec4( vec4(orig, 1.0) * VertexIn[0].vo_matrix, 1.0);
    vec4 vViewPos = vWorldPos * mViewMat;

	vec4 vProjPos = vec4(vViewPos.xyz, 1.0) * mProjMat;
	gl_Position	= vProjPos;
    
    geometryData.vo_vColor = clr;
    
    EmitVertex();
}

void MakeCube(int ii)
{
    vec4 clr = VertexIn[ii].vo_vColor;

    // front
    MakeVertex(vec3(1.0,1.0,1.0), clr, vec3(0,0,1));
    MakeVertex(vec3(1.0,-1.0,1.0), clr, vec3(0,0,1));
    MakeVertex(vec3(-1.0,1.0,1.0), clr, vec3(0,0,1));
    MakeVertex(vec3(-1.0,-1.0,1.0), clr, vec3(0,0,1));
    EndPrimitive();
    
    // back
    MakeVertex(vec3(-1.0,1.0,-1.0), clr, vec3(0,0,-1));
    MakeVertex(vec3(-1.0,-1.0,-1.0), clr, vec3(0,0,-1));
    MakeVertex(vec3(1.0,1.0,-1.0), clr, vec3(0,0,-1));
    MakeVertex(vec3(1.0,-1.0,-1.0), clr, vec3(0,0,-1));
    EndPrimitive();
    

    // right
    MakeVertex(vec3(1.0,-1.0,1.0), clr, vec3(1,0,0));
    MakeVertex(vec3(1.0,1.0,1.0), clr, vec3(1,0,0));
    MakeVertex(vec3(1.0,-1.0,-1.0), clr, vec3(1,0,0));
    MakeVertex(vec3(1.0,1.0,-1.0), clr, vec3(1,0,0));
    EndPrimitive();

 
    // left
    MakeVertex(vec3(-1.0,-1.0,-1.0), clr, vec3(-1,0,0));
    MakeVertex(vec3(-1.0,1.0,-1.0), clr, vec3(-1,0,0));
    MakeVertex(vec3(-1.0,-1.0,1.0), clr, vec3(-1,0,0));
    MakeVertex(vec3(-1.0,1.0,1.0), clr, vec3(-1,0,0));
    EndPrimitive();
 
    
    // top
    MakeVertex(vec3(1.0,1.0,-1.0), clr, vec3(0,1,0));
    MakeVertex(vec3(1.0,1.0,1.0), clr, vec3(0,1,0));
    MakeVertex(vec3(-1.0,1.0,-1.0), clr, vec3(0,1,0));
    MakeVertex(vec3(-1.0,1.0,1.0), clr, vec3(0,1,0));
    EndPrimitive();


    // bottom
    MakeVertex(vec3(-1.0,-1.0,-1.0), clr, vec3(0,-1,0));
    MakeVertex(vec3(-1.0,-1.0,1.0), clr, vec3(0,-1,0));
    MakeVertex(vec3(1.0,-1.0,-1.0), clr, vec3(0,-1,0));
    MakeVertex(vec3(1.0,-1.0,1.0), clr, vec3(0,-1,0));
    EndPrimitive();
 
    
}

void main()
{   
    for(int i = 0; i < gl_in.length(); i++)
    {
        MakeCube(i);
    }
}
