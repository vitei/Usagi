#include "../includes/platformdefines.inc"
#include "../includes/colorspace.inc"

precision highp float;

// <<GENERATED_CODE>>

ATTRIB_LOC(0) in vec2 vo_vTexCoord;
layout(location = 0) out vec4 colorOut;

void main()
{
   vec4 linear = texture(sampler0, vo_vTexCoord);
   
   vec3 extended = linear.rgb * uMaterial.fBrightness;
   
   colorOut = vec4(extended, linear.a);
}