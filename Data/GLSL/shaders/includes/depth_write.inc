
#ifndef SHADOW_PASS

layout(location = 1) out vec4 depthOut;

void OutputLinearDepth(vec3 vEyePos)
{
	depthOut	= vec4(vEyePos.z/vNearFar.y, 0.0, 0.0, 0.0);
}

void OutputLinearDepth(float fEyeDepth)
{
	depthOut	= vec4(fEyeDepth/vNearFar.y, 0.0, 0.0, 0.0);
}

#endif 
