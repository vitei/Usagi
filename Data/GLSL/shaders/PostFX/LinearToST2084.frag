#include "../includes/platformdefines.inc"
#include "../includes/colorspace.inc"

precision highp float;

// <<GENERATED_CODE>>

ATTRIB_LOC(0) in vec2 vo_vTexCoord;
layout(location = 0) out vec4 colorOut;

#define kMaxNitsFor2084 10000.0f

const mat3 k709to2020 = mat3(
   0.6274040f, 0.3292820f, 0.0433136f,
   0.0690970f, 0.9195400f, 0.0113612f,
   0.0163916f, 0.0880132f, 0.8955950f);

const mat3 kExpanded709to2020 = mat3(
    0.6274040f, 0.3292820f, 0.0433136f,
    0.0457456f, 0.941777f,  0.0124772f,
   -0.00121055f, 0.0176041f, 0.983607f);

vec3 LinearToST2084(vec3 normalizedLinearValue)
{
   vec3 ST2084 = pow((0.8359375f + 18.8515625f * pow(abs(normalizedLinearValue), vec3(0.1593017578f))) / 
                     (1.0f + 18.6875f * pow(abs(normalizedLinearValue), vec3(0.1593017578f))), 
                     vec3(78.84375f));
   return ST2084;
}

void main()
{
   vec4 linear = texture(sampler0, vo_vTexCoord);
   
   // Convert BT.709 to BT.2020 primaries
   vec3 rec2020 = linear.rgb * (uMaterial.bExpandGamut ? kExpanded709to2020 : k709to2020);
   
   // Scale from display range to ST.2084 nit range
   vec3 scaledForST2084 = rec2020 * (200.0f / 10000.0f);  // or use a uniform
   
   // Apply PQ curve
   vec3 hdr10 = LinearToST2084(scaledForST2084);
   
   colorOut = vec4(hdr10, linear.a);
}