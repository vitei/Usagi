#include "../includes/platformdefines.inc"

BUFFER_LAYOUT(1, UBO_CUSTOM0_ID) uniform Custom0
{
  mat4  mTransform;
  vec4  vColour;
  bool  bLit;
};

// Input attributes
in vec3 vo_vWorldPos;
in vec3 vo_vNormal;

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
