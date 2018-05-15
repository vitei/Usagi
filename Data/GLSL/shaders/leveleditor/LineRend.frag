#include "../includes/platformdefines.inc"


// Input attributes
in vec4 vo_vColor;

layout(location = 0) out vec4 colorOut;

void main(void)
{ 
  vec4 fc = vo_vColor;
  colorOut = fc;
}
