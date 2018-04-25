

layout(location = 0) out vec4 po_vDiffuse;
layout(location = 2) out vec4 po_vNormal;
//layout(location = 3) out vec4 po_vVelocity;
layout(location = 3) out vec4 po_vEmissive;
layout(location = 4) out vec4 po_vSpecular;

void OutputDeferredDataLinDepth(vec3 vEyePos, vec3 vViewNormal, vec3 color, vec3 vEmissive, vec3 specCol, float specularPow)
{
	OutputLinearDepth(vEyePos.z);
	po_vNormal		= vec4(vViewNormal, 0.0);
	po_vDiffuse		= vec4(color, specularPow/128.f);
	po_vEmissive	= vec4(vEmissive, 1.0);
	po_vSpecular	= vec4(specCol, 1.0);
}

void OutputDeferredDataLinDepth(float fEyeDepth, vec3 vViewNormal, vec3 color, vec3 vEmissive, vec3 specCol, float specularPow)
{
	OutputLinearDepth(fEyeDepth);
	po_vNormal		= vec4(vViewNormal, 0.0);
	po_vDiffuse		= vec4(color, specularPow/128.f);
	po_vEmissive	= vec4(vEmissive, 0.0);
	po_vSpecular	= vec4(specCol, 1.0);
}

#if 0
void OutputScreenSpaceVelocity(vec4 vPos, vec4 vPrevPos)
{
	vec2 a = (vPos.xy / vPos.ww);
	vec2 b = (vPrevPos.xy / vPrevPos.ww);

	vec2 vVel = (a-b)*vMotionBlurVars.xy;

	po_vVelocity = vec4(vVel, 0.0, 0.0);
}
#endif
