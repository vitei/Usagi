#include "../includes/platformdefines.inc"

BUFFER_LAYOUT(1, UBO_GLOBAL_ID) uniform Globals
{
    mat4    mMatrices[6];
    vec4    lightPos;
    float   lightFarDist;
};

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;


// The world position of this fragment
ATTRIB_LOC(0) out vec4 go_vFragPos;

void main()
{
    for(int cubeFace = 0; cubeFace < 6; ++cubeFace)
    {
        gl_Layer = cubeFace; 
        for(int triangleVert = 0; triangleVert < 3; ++triangleVert) 
        {
            go_vFragPos = gl_in[triangleVert].gl_Position;
            gl_Position = go_vFragPos * mMatrices[cubeFace];
            EmitVertex();
        }    
        EndPrimitive();
    }
}  