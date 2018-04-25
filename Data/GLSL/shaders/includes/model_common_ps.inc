#ifndef _MODEL_COMMON_PS_INC
#define _MODEL_COMMON_PS_INC

#ifndef SHADOW_PASS	
vec3 ApplyHemisphereLighting(in vec3 vColor, in vec3 vNormView)
{
    float fDot = dot(vNormView, vHemisphereDir.xyz);
    float fRate = (fDot*vHemisphereDir.w)+vHemisphereDir.w;
    vec3 vHemColor = vHemSkyColor.rgb - vHemGroundColor.rgb;
   	vHemColor =  (vHemColor * fRate) + vHemGroundColor.rgb;
    return vColor * vHemColor;
}
#endif

float GetRimValue(in float rimInt, in float rimPower, in float fViewNormZ)
{
 	return rimInt * pow(1 - abs(clamp( -fViewNormZ, 0, 1)), rimPower);
}

vec3 GetRimValueColor(in vec3 rimCol, in float rimPower, in float fViewNormZ)
{
 	return rimCol * pow(1 - abs(clamp( -fViewNormZ, 0, 1)), rimPower);
}


vec3 GetRimValueColorTest(in vec3 rimCol, in float rimPower, in vec3 vNormal, in vec3 vViewDir)
{
 	return rimCol * pow(1 - abs(clamp( dot(vNormal, vViewDir), 0, 1)), rimPower);
}

#endif

