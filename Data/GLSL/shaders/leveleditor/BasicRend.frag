#include "../includes/platformdefines.inc"

BUFFER_LAYOUT(1, UBO_CUSTOM_3_ID) uniform Custom0
{
  mat4  mTransform;
  vec4  vColour;
  bool  bLit;
};

// Input attributes
ATTRIB_LOC(0) in vec3 vo_vWorldPos;
ATTRIB_LOC(1) in vec3 vo_vNormal;

layout(location = 0) out vec4 colorOut;

void main(void)
{ 
  vec4 fc = vColour;
    if(bLit)
    {
      vec3 norm = normalize(vo_vWorldPos);
      // TODO: Perform lighting
    }
    colorOut = fc;
}
