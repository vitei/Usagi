#include "../includes/platformdefines.inc"
#include "../includes/assao.inc"


ATTRIB_LOC(0) in vec2 vo_vTexCoord;

SAMPLER_LOC(1, 3) uniform sampler2D g_ImportanceMap;   // corresponds to SSAO_TEXTURE_SLOT3

layout(location = 0) out float colorOut;


void main()
{
    uvec2 pos = (uvec2(gl_FragCoord.xy));

    float centre = textureLod( g_ImportanceMap, vo_vTexCoord, 0.0 ).x;
    //return centre;

    vec2 halfPixel = g_ASSAOConsts.QuarterResPixelSize * 0.5f;

    vec4 vals;
    vals.x = textureLod( g_ImportanceMap, vo_vTexCoord + vec2( -halfPixel.x * 3, -halfPixel.y ), 0.0 ).x;
    vals.y = textureLod( g_ImportanceMap, vo_vTexCoord + vec2( +halfPixel.x, -halfPixel.y * 3 ), 0.0 ).x;
    vals.z = textureLod( g_ImportanceMap, vo_vTexCoord + vec2( +halfPixel.x * 3, +halfPixel.y ), 0.0 ).x;
    vals.w = textureLod( g_ImportanceMap, vo_vTexCoord + vec2( -halfPixel.x, +halfPixel.y * 3 ), 0.0 ).x;

    float avgVal = dot( vals, vec4( 0.25, 0.25, 0.25, 0.25 ) );
    vals.xy = max( vals.xy, vals.zw );
    float maxVal = max( centre, max( vals.x, vals.y ) );

    colorOut = mix( maxVal, avgVal, cSmoothenImportance );
}
