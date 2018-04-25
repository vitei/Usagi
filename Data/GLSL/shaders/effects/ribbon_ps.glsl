#include "../includes/platformdefines.inc"

SAMPLER_LOC(1, 0) uniform sampler2D sampler0; 
SAMPLER_LOC(1, 1) uniform sampler2D sampler1; 

in GeometryData
{
    vec4    go_vColor;
    vec2    go_vTexCoord;
    vec2	go_vTexCoord2;

} geometryData;


layout(location = 0) out vec4 colorOut;

void main()
{
   vec4 vTex0      =  texture(sampler0, geometryData.go_vTexCoord);
   vec4 vTex1      =  texture(sampler1, geometryData.go_vTexCoord2);
   // vec4 vOut = geometryData.go_vColor;

   vec4 vOut;
   // This may look a bit strange, the ribbon trails have gone through some iteration
   // The central core, regardless of the outer color, is white (super bright). This would need adjusting on an HDR system
   // but the CTR has no HDR capabilities. The core is determined by the red componenet of texture 1
   // As the rear of the trail should be "cooler", the interpolation into white decreases as the alpha decreases
   vOut.rgb = mix(vec3(1.0, 1.0, 1.0), geometryData.go_vColor.rgb, clamp(vTex1.r + (1.0-geometryData.go_vColor.a), 0.0, 1.0));

   float fAlpha = vTex0.r * geometryData.go_vColor.a;
   // We are using additive blending, so we manually multiply the alpha in here
   vOut.rgb *= fAlpha;

   // The destination blend is the inverse of our alpha, we use the green channel to control how much of the destination colour is dampened
   // in order to avoid the typical overexposure problems of additive blending
   vOut.a = vTex1.g * fAlpha;

   colorOut =  vOut;
}