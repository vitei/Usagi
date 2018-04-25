#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

in vec4 go_vFragPos;

void main()
{
    // Distance of this fragment from the light postion
    float lightDistance = length(go_vFragPos.xyz - lightPos.xyz);
    
    // Linear depth value for the light
    lightDistance = lightDistance / lightFarDist;
    
    // Manually set the z-buffer value to be this linear depth value
    gl_FragDepth = lightDistance;
}  