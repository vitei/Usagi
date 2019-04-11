#ifndef _LAYOUT_3D_VS_INC
#define _LAYOUT_3D_VS_INC


// The VS material data is used for placing the pane in the world
BUFFER_LAYOUT(2, UBO_MATERIAL_ID) uniform Material
{
	vec4 position3D;
	vec4 depthRange;
	vec2 offsetPos2D;
};

float GetPosAndVisibility(out vec4 vTransformedPos)
{
	vec4 vViewPos = vec4(position3D.xyz, 1.0) * mViewMat;
    vTransformedPos = vViewPos * mProjMat;

    vTransformedPos /= vTransformedPos.w;
    vTransformedPos.xy += offsetPos2D.xy;

    float fDistance = length(vViewPos);
    fDistance = clamp(fDistance, depthRange.x, depthRange.y);
    fDistance -= depthRange.x;
    return 1.0f-((fDistance)*depthRange.z);
}


#endif

