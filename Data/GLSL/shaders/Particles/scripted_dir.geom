#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"


// Input/ output definitions
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;


ATTRIB_LOC(0) in vec4    vo_vColor[];
ATTRIB_LOC(1) in vec2    vo_vSize[];
ATTRIB_LOC(2) in float   vo_fRot[];
ATTRIB_LOC(3) in vec4    vo_vUVRange[][2];
ATTRIB_LOC(5) in vec3    vo_velocity[];

out GeometryData
{
    INT_LOC(0) vec4    vo_vColor;
    INT_LOC(1) vec2    vo_vTexcoord[2];
    INT_LOC(3) vec2    vo_vScreenTex;
    INT_LOC(4) float   vo_fEyeDepth;
    INT_LOC(5) float   vo_fDepthFadeClamp;  // TODO: On define

} geometryData;


void CreateVertex(int ii, vec2 scale, vec3 vPos)
{
    vec4 pos;
  
    vec2 uv;
    pos = vec4(vPos, 0.0);

    pos += vec4(gl_in[ii].gl_Position.xyz, 0.0);
    pos.w = 1.0;
    geometryData.vo_vColor = vo_vColor[ii];
    // TODO: Multiple images in the same texture
    geometryData.vo_vTexcoord[0] = (vec2(scale.x, 1.0 - scale.y) * vo_vUVRange[ii][0].zw) + vo_vUVRange[ii][0].xy;
    geometryData.vo_vTexcoord[1] = (vec2(scale.x, 1.0 - scale.y) * vo_vUVRange[ii][0].zw) + vo_vUVRange[ii][0].xy;

    vec4 vEyePos    = pos * mViewMat;
    
    // Add the transparency for the fog
    geometryData.vo_vColor.w *= (1.0-CalculateLinearFog(-vEyePos.xyz));
    pos = vec4( vEyePos.xyz, 1.0 ) * mProjMat;
    pos.w *= gl_in[ii].gl_Position.w;

    geometryData.vo_vScreenTex = pos.xy / pos.ww;
    geometryData.vo_fEyeDepth = vEyePos.z/vNearFar.y;
    geometryData.vo_fDepthFadeClamp = 1.0f;

    gl_Position = pos;

    EmitVertex();
}

void main(void)
{
    for (int ii = 0; ii < gl_in.length(); ii++)
    {
        vec3 vLeftVec = normalize(vo_velocity[ii]) * vo_vSize[ii].xxx;
        vec3 vEyeVec = normalize(gl_in[ii].gl_Position.xyz - vEyePos.xyz);
        vec3 vUpVec = normalize(cross(vLeftVec, vEyeVec)) * vo_vSize[ii].yyy;
    	CreateVertex(ii, vec2(1.0, 1.0), vLeftVec + vUpVec);
    	CreateVertex(ii, vec2(1.0, 0.0), vLeftVec - vUpVec);
    	CreateVertex(ii, vec2(0.0, 1.0), -vLeftVec + vUpVec);
        CreateVertex(ii, vec2(0.0, 0.0), -vLeftVec - vUpVec);

        EndPrimitive();
    }
}
