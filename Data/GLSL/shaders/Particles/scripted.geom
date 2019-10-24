#include "../includes/platformdefines.inc"
#define OVERRIDE_TRANS 
#include "../includes/global_3d.inc"

// Input/ output definitions
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;


BUFFER_LAYOUT(1, UBO_CUSTOM_1_ID) uniform Instance1
{
    mat3x4  mUserMat;
    vec2    vParticleCenter;
    float   fDepthFadeDist;
    float   fCameraOffset;
    bool    bCustomMatrix;
    bool    bYAxisAlign;
};

in VertexData
{
    INT_LOC(0) vec4    vo_vColor;
    INT_LOC(1) vec2    vo_vSize;
    INT_LOC(2) float   vo_fRot;
    INT_LOC(3) vec4    vo_vUVRange[2];
    INT_LOC(5) vec3    vo_velocity;

} vertexData[];


out GeometryData
{
    INT_LOC(0) vec4    vo_vColor;
    INT_LOC(1) vec2    vo_vTexcoord[2];
    INT_LOC(3) vec2    vo_vScreenTex;
    INT_LOC(4) float   vo_fEyeDepth;
    INT_LOC(5) float   vo_fDepthFadeClamp;  // TODO: On define

} geometryData;


vec4 ParticleRotation(vec4 pos, float rot)
{
    float c_rot = rot;
    float sin_rz = sin(c_rot);
    float cos_rz = cos(c_rot);
    vec4 posr = pos;
    posr.x = pos.x * cos_rz - pos.y * sin_rz;
    posr.y = pos.x * sin_rz + pos.y * cos_rz;
    return posr;
}


void CreateVertex(int ii, vec2 scale)
{
    vec4 pos;
  
    vec2 uv;
    uv = scale - vec2(0.5, 0.5);
    uv -= vParticleCenter;
    uv *= vertexData[ii].vo_vSize;

    if(bYAxisAlign)
    {
        vec3 vUp = vec3(0.0, 1.0, 0.0);
        vec4 vRight = vec4(1.0, 0.0, 0.0, 0.0);
        vRight = vRight * mInvViewMat;

        pos.xyz = uv.x * vRight.xyz + uv.y * vUp;
    }
    else
    {
        pos = ParticleRotation(vec4(uv, 0.0, 0.0), vertexData[ii].vo_fRot);

        if(bCustomMatrix)
        {
            pos.xyz = pos * mUserMat;
        }
        else
        {   
            pos = pos * mInvViewMat;
        }
    }

    vec3 vDirToEye = normalize(vEyePos.xyz - gl_in[ii].gl_Position.xyz);

    pos += vec4(gl_in[ii].gl_Position.xyz, 0.0);
    pos.xyz += (vDirToEye * fCameraOffset);
    pos.w = 1.0;
    geometryData.vo_vColor = vertexData[ii].vo_vColor;
    // TODO: Multiple images in the same texture
    geometryData.vo_vTexcoord[0] = (vec2(scale.x, 1.0 - scale.y) * vertexData[ii].vo_vUVRange[0].zw) + vertexData[ii].vo_vUVRange[0].xy;
    geometryData.vo_vTexcoord[1] = (vec2(scale.x, 1.0 - scale.y) * vertexData[ii].vo_vUVRange[1].zw) + vertexData[ii].vo_vUVRange[1].xy;

    vec4 vEyePos    = pos * mViewMat;

    // Add the transparency for the fog
    geometryData.vo_vColor.w *= (1.0-CalculateLinearFog(-vEyePos.xyz));
    pos = vec4( vEyePos.xyz, 1.0 ) * mProjMat;
    pos /= pos.w;

    float fHalfDepthFade = fDepthFadeDist*0.5f;
    float fZDepth = max(vEyePos.z - fHalfDepthFade, min(vNearFar.x, vEyePos.z + fHalfDepthFade));//min(vNearFar.x + 0.2f, vEyePos.z));

    // Back pos - 
    geometryData.vo_fDepthFadeClamp = fDepthFadeDist - ((vEyePos.z - fHalfDepthFade) - fZDepth);
 
    vec4 vOffsetPos = vec4( vEyePos.xy, fZDepth, 1.0f) * mProjMat;
    vOffsetPos /= vOffsetPos.w;


    pos.w *= gl_in[ii].gl_Position.w;

    geometryData.vo_fEyeDepth = fZDepth;

    geometryData.vo_vScreenTex = pos.xy / pos.ww;

    gl_Position = vec4(pos.xy, vOffsetPos.z, 1.0f);

    EmitVertex();
}

void main(void)
{
    for (int ii = 0; ii < gl_in.length(); ii++)
    {
        CreateVertex(ii, vec2(0.0, 1.0));
        CreateVertex(ii, vec2(0.0, 0.0));
        CreateVertex(ii, vec2(1.0, 1.0));
        CreateVertex(ii, vec2(1.0, 0.0));

        EndPrimitive();
    }
}
