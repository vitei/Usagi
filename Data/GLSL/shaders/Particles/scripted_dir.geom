#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"


// Input/ output definitions
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;


in VertexData
{
    AT_LCMP(0, 0) vec4    vo_vColor;
    AT_LCMP(1, 0) vec2    vo_vSize;
    AT_LCMP(1, 2) float   vo_fRot;
    AT_LCMP(2, 0) vec4    vo_vUVRange[2];
    AT_LCMP(4, 0) vec3    vo_velocity;
} vertexData[];


out GeometryData
{
    AT_LCMP(0, 0) vec4    vo_vColor;
    AT_LCMP(1, 0) vec2    vo_vTexcoord[2];
    AT_LCMP(1, 2) vec2    vo_vScreenTex;
    AT_LCMP(2, 2) float   vo_fEyeDepth;

} geometryData;


void CreateVertex(int ii, vec2 scale, vec3 vPos)
{
    vec4 pos;
  
    vec2 uv;
    pos = vec4(vPos, 0.0);

    pos += vec4(gl_in[ii].gl_Position.xyz, 0.0);
    pos.w = 1.0;
    geometryData.vo_vColor = vertexData[ii].vo_vColor;
    // TODO: Multiple images in the same texture
    geometryData.vo_vTexcoord[0] = (vec2(scale.x, 1.0 - scale.y) * vertexData[ii].vo_vUVRange[0].zw) + vertexData[ii].vo_vUVRange[0].xy;
    geometryData.vo_vTexcoord[1] = (vec2(scale.x, 1.0 - scale.y) * vertexData[ii].vo_vUVRange[0].zw) + vertexData[ii].vo_vUVRange[0].xy;

    vec4 vEyePos    = pos * mViewMat;
    
    // Add the transparency for the fog
    geometryData.vo_vColor.w *= (1.0-CalculateLinearFog(-vEyePos.xyz));
    pos = vec4( vEyePos.xyz, 1.0 ) * mProjMat;
    pos.w *= gl_in[ii].gl_Position.w;

    geometryData.vo_vScreenTex = pos.xy / pos.ww;
    geometryData.vo_fEyeDepth = vEyePos.z/vNearFar.y;

    gl_Position = pos;

    EmitVertex();
}

void main(void)
{
    for (int ii = 0; ii < gl_in.length(); ii++)
    {
        vec3 vLeftVec = normalize(vertexData[ii].vo_velocity) * vertexData[ii].vo_vSize.xxx;
        vec3 vEyeVec = normalize(gl_in[ii].gl_Position.xyz - vEyePos.xyz);
        vec3 vUpVec = normalize(cross(vLeftVec, vEyeVec)) * vertexData[ii].vo_vSize.yyy;
    	CreateVertex(ii, vec2(1.0, 1.0), vLeftVec + vUpVec);
    	CreateVertex(ii, vec2(1.0, 0.0), vLeftVec - vUpVec);
    	CreateVertex(ii, vec2(0.0, 1.0), -vLeftVec + vUpVec);
        CreateVertex(ii, vec2(0.0, 0.0), -vLeftVec - vUpVec);

        EndPrimitive();
    }
}
