#include "../includes/platformdefines.inc"
#include "../includes/assao.inc"


ATTRIB_LOC(0) in vec2 vo_vTexCoord;

SAMPLER_LOC(1, 4) uniform sampler2DArray g_FinalSSAO;   // corresponds to SSAO_TEXTURE_SLOT4

layout(location = 0) out vec4 colorOut;



void main()
{
    float a = textureLod(g_FinalSSAO, vec3( vo_vTexCoord.xy, 0 ), 0.0 ).x;
    float d = textureLod(g_FinalSSAO, vec3( vo_vTexCoord.xy, 3 ), 0.0 ).x;
    float avg = (a+d) * 0.5;
    colorOut = vec4( avg.xxx, 1.0 );
}