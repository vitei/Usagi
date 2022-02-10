#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"

layout(points) in;
layout (triangle_strip, max_vertices=24) out;

BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	float  vLighting;
};

BUFFER_LAYOUT(1, UBO_CUSTOM_0_ID) uniform Instance
{
    mat3x4  mModelMat;
};


ATTRIB_LOC(0) in vec3    vo_vScale[];
ATTRIB_LOC(1) in vec4    vo_vColor[];
ATTRIB_LOC(2) in float	vo_fYaw[];

ATTRIB_LOC(0) out vec4 	go_vColor;


vec3 lightDir = vec3(-0.7, 0.4, 0.7);
float amb = 0.3;
float degToRad = 0.0174532925;

void MakeVertex(vec3 orig, vec3 off, vec4 clr, vec3 norm, float yaw)
{
    vec3 wNorm = norm * mat3(mModelMat);
    float yawInRad = degToRad * yaw;
    float s = sin(yawInRad);
    float c = cos(yawInRad);

	off = vec3(c * off.x - s * off.z, off.y, s * off.x + c * off.z);
    vec4 vWorldPos = vec4(orig+off, 1.0);
    //vWorldPos = vec4((val * vWorldPos.x - val * vWorldPos.z), vWorldPos.y, (val * vWorldPos.x + val * vWorldPos.z), 1.0);
	vec4 vViewPos = vWorldPos * mViewMat;	// Reverse ordering as a mat3x4

	vec4 vProjPos = vec4(vViewPos.xyz, 1.0) * mProjMat;
	gl_Position	= vProjPos;
    
    if (vLighting>0.0)
    {
        vec4 mclr = clr * max(0, dot(wNorm,lightDir)) + (clr * amb);
        go_vColor = vec4(mclr.rgb, vViewPos.z);
    }else
    {
        go_vColor = clr;
    }
    
    EmitVertex();
}

void MakeCube(int ii)
{
    vec3 orig = gl_in[ii].gl_Position.xyz;
    vec4 clr = vo_vColor[ii];
    vec3 sc = vo_vScale[ii];
    float yaw = vo_fYaw[ii];

    // front
    MakeVertex(orig, vec3(sc.x,sc.y,sc.z), clr, vec3(0,0,1), yaw);
    MakeVertex(orig, vec3(sc.x,-sc.y,sc.z), clr, vec3(0,0,1), yaw);
    MakeVertex(orig, vec3(-sc.x,sc.y,sc.z), clr, vec3(0,0,1), yaw);
    MakeVertex(orig, vec3(-sc.x,-sc.y,sc.z), clr, vec3(0,0,1), yaw);
    EndPrimitive();
    
    // back
    MakeVertex(orig, vec3(-sc.x,sc.y,-sc.z), clr, vec3(0,0,-1), yaw);
    MakeVertex(orig, vec3(-sc.x,-sc.y,-sc.z), clr, vec3(0,0,-1), yaw);
    MakeVertex(orig, vec3(sc.x,sc.y,-sc.z), clr, vec3(0,0,-1), yaw);
    MakeVertex(orig, vec3(sc.x,-sc.y,-sc.z), clr, vec3(0,0,-1), yaw);
    EndPrimitive();
    

    // right
    MakeVertex(orig, vec3(sc.x,-sc.y,sc.z), clr, vec3(1,0,0), yaw);
    MakeVertex(orig, vec3(sc.x,sc.y,sc.z), clr, vec3(1,0,0), yaw);
    MakeVertex(orig, vec3(sc.x,-sc.y,-sc.z), clr, vec3(1,0,0), yaw);
    MakeVertex(orig, vec3(sc.x,sc.y,-sc.z), clr, vec3(1,0,0), yaw);
    EndPrimitive();

 
    // left
    MakeVertex(orig, vec3(-sc.x,-sc.y,-sc.z), clr, vec3(-1,0,0), yaw);
    MakeVertex(orig, vec3(-sc.x,sc.y,-sc.z), clr, vec3(-1,0,0), yaw);
    MakeVertex(orig, vec3(-sc.x,-sc.y,sc.z), clr, vec3(-1,0,0), yaw);
    MakeVertex(orig, vec3(-sc.x,sc.y,sc.z), clr, vec3(-1,0,0), yaw);
    EndPrimitive();
 
    
    // top
    MakeVertex(orig, vec3(sc.x,sc.y,-sc.z), clr, vec3(0,1,0), yaw);
    MakeVertex(orig, vec3(sc.x,sc.y,sc.z), clr, vec3(0,1,0), yaw);
    MakeVertex(orig, vec3(-sc.x,sc.y,-sc.z), clr, vec3(0,1,0), yaw);
    MakeVertex(orig, vec3(-sc.x,sc.y,sc.z), clr, vec3(0,1,0), yaw);
    EndPrimitive();


    // bottom
    MakeVertex(orig, vec3(-sc.x,-sc.y,-sc.z), clr, vec3(0,-1,0), yaw);
    MakeVertex(orig, vec3(-sc.x,-sc.y,sc.z), clr, vec3(0,-1,0), yaw);
    MakeVertex(orig, vec3(sc.x,-sc.y,-sc.z), clr, vec3(0,-1,0), yaw);
    MakeVertex(orig, vec3(sc.x,-sc.y,sc.z), clr, vec3(0,-1,0), yaw);
    EndPrimitive();
 
    
}

void main()
{   
    for(int i = 0; i < gl_in.length(); i++)
    {
        MakeCube(i);
    }
}
