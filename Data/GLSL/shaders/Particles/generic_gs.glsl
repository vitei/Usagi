#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

// FIXME: Should be an external define, boost the effects to specify defines when creating an effect
#define APPLY_ROTATION

// Input/ output definitions
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;


in VertexData
{
    vec4    vo_vColor;
    vec2    vo_vRotSize;
    vec4    vo_vUVRange;

} vertexData[];


out GeometryData
{
    vec4 	vo_vColor;
    vec2 	vo_vTexcoord;
    vec2    vo_vScreenTex;
    float   vo_fEyeDepth;

} geometryData;


vec4 ParticleRotation(vec4 pos, float rot)
{
    float c_rot = mod(rot, 2.0*3.141592653589793);
    float sin_rz = sin(c_rot);
    float cos_rz = cos(c_rot);
    vec4 posr = pos;
    posr.x = pos.x * cos_rz - pos.y * sin_rz;
    posr.y = pos.x * sin_rz + pos.y * cos_rz;
    return posr;
}


void CreateVertex(int ii, vec2 scale, vec2 vAspect)
{
    vec4 pos;
  
#ifdef APPLY_ROTATION  
    vec2 uv;
    uv = scale - vec2(0.5, 0.5);
    uv *= vertexData[ii].vo_vRotSize.yy;
    pos = ParticleRotation(vec4(uv*vAspect, 0.0, 0.0), vertexData[ii].vo_vRotSize.x) * mInvViewMat;
#else
	pos = scale * vertexData[ii].vo_vRotSize.yy;
#endif
    pos += vec4(gl_in[ii].gl_Position.xyz, 0.0);
    pos.w = 1.0;
    geometryData.vo_vColor = vertexData[ii].vo_vColor;
    // TODO: Multiple images in the same texture
    geometryData.vo_vTexcoord = (vec2(scale.x, 1.0 - scale.y) * vertexData[ii].vo_vUVRange.zw) + vertexData[ii].vo_vUVRange.xy;

    vec4 vEyePos    = pos * mViewMat;
    // Calculate the linear depth
    geometryData.vo_fEyeDepth = vEyePos.z/vNearFar.y;

    // Add the transparency for the fog
    geometryData.vo_vColor.w *= (1.0-CalculateLinearFog(-vEyePos.xyz));
    pos = vec4( vEyePos.xyz, 1.0 ) * mProjMat;
    pos.w *= gl_in[ii].gl_Position.w;

    geometryData.vo_vScreenTex = pos.xy / pos.ww;

    gl_Position = pos;

    EmitVertex();
}

void main(void)
{
    for (int ii = 0; ii < gl_in.length(); ii++)
    {
        vec2 vAspect = vec2(vertexData[ii].vo_vUVRange.z/vertexData[ii].vo_vUVRange.w, 1.0);
    	CreateVertex(ii, vec2(0.0, 1.0), vAspect);
    	CreateVertex(ii, vec2(0.0, 0.0), vAspect);
    	CreateVertex(ii, vec2(1.0, 1.0), vAspect);
        CreateVertex(ii, vec2(1.0, 0.0), vAspect);

        EndPrimitive();
    }
}
