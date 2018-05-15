#include "../includes/platformdefines.inc"

SAMPLER_LOC(1, 0) uniform sampler2D sampler0; 

in GeometryData
{
    vec4    go_vColor;
    vec2    go_vTexCoord;

} geometryData;


layout(location = 0) out vec4 colorOut;

void main()
{
    vec4 vTex0      =  texture(sampler0, geometryData.go_vTexCoord);
    vec4 vOut = geometryData.go_vColor;
    vOut *= vTex0;

    colorOut =  vOut;
}