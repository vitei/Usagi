#include "../includes/platformdefines.inc"
#include "../includes/assao.inc"


ATTRIB_LOC(0) in vec2 vo_vTexCoord;

SAMPLER_LOC(1, 0) uniform sampler2D g_ImportanceMap;   // corresponds to SSAO_TEXTURE_SLOT3

layout(binding=4, r32ui) uniform uimage1D g_LoadCounterOutputUAV;   // corresponds to SSAO_LOAD_COUNTER_UAV_SLOT 

layout(location = 0) out float colorOut;

void main()
{
    uvec2 pos = ( uvec2(gl_FragCoord.xy) );

    float centre = textureLod(g_ImportanceMap, vo_vTexCoord, 0.0 ).x;
    //return centre;

    vec2 halfPixel = g_ASSAOConsts.QuarterResPixelSize * 0.5f;

    vec4 vals;
    vals.x = textureLod(g_ImportanceMap,vo_vTexCoord + vec2( -halfPixel.x, -halfPixel.y * 3 ), 0.0 ).x;
    vals.y = textureLod(g_ImportanceMap, vo_vTexCoord + vec2( +halfPixel.x * 3, -halfPixel.y ), 0.0 ).x;
    vals.z = textureLod(g_ImportanceMap, vo_vTexCoord + vec2( +halfPixel.x, +halfPixel.y * 3 ), 0.0 ).x;
    vals.w = textureLod(g_ImportanceMap, vo_vTexCoord + vec2( -halfPixel.x * 3, +halfPixel.y ), 0.0 ).x;

    float avgVal = dot( vals, vec4( 0.25, 0.25, 0.25, 0.25 ) );
    vals.xy = max( vals.xy, vals.zw );
    float maxVal = max( centre, max( vals.x, vals.y ) );

    float retVal = mix( maxVal, avgVal, cSmoothenImportance );

    // sum the average; to avoid overflowing we assume max AO resolution is not bigger than 16384x16384; so quarter res (used here) will be 4096x4096, which leaves us with 8 bits per pixel 
    uint sum = uint(clamp(retVal, 0.0, 1.0) * 255.0 + 0.5);
    
    // save every 9th to avoid InterlockedAdd congestion - since we're blurring, this is good enough; compensated by multiplying LoadCounterAvgDiv by 9
    if( ((pos.x % 3) + (pos.y % 3)) == 0  )
        imageAtomicAdd( g_LoadCounterOutputUAV, 0, sum );

    colorOut = retVal;
}