#include "../includes/platformdefines.inc"
#include "../includes/assao.inc"


ATTRIB_LOC(0) in vec2 vo_vTexCoord;

SAMPLER_LOC(1, 0) uniform sampler2DArray g_FinalSSAO;   // corresponds to SSAO_TEXTURE_SLOT2

layout(location = 0) out float colorOut;



void main()
{
    uvec2 basePos = (uvec2(gl_FragCoord.xy)) * 2;

    vec2 baseUV = (vec2(basePos) + vec2( 0.5, 0.5 ) ) * g_ASSAOConsts.HalfViewportPixelSize;
    vec2 gatherUV = (vec2(basePos) + vec2( 1.0, 1.0 ) ) * g_ASSAOConsts.HalfViewportPixelSize;

    float avg = 0.0;
    float minV = 1.0;
    float maxV = 0.0;

    for( int i = 0; i < 4; i++ )
    {
        vec4 vals = textureGather(g_FinalSSAO, vec3( gatherUV, i ) );

        // apply the same modifications that would have been applied in the main shader
        vals = g_ASSAOConsts.EffectShadowStrength * vals;

        vals = 1-vals;

        vals = pow( clamp( vals, 0, 1 ), vec4(g_ASSAOConsts.EffectShadowPow) );

        avg += dot( vec4( vals.x, vals.y, vals.z, vals.w ), vec4( 1.0 / 16.0, 1.0 / 16.0, 1.0 / 16.0, 1.0 / 16.0 ) );

        maxV = max( maxV, max( max( vals.x, vals.y ), max( vals.z, vals.w ) ) );
        minV = min( minV, min( min( vals.x, vals.y ), min( vals.z, vals.w ) ) );
    }

    float minMaxDiff = maxV - minV;

    //return pow( saturate( minMaxDiff * 1.2 + (1.0-avg) * 0.3 ), 0.8 );
    colorOut = pow( clamp( minMaxDiff * 2.0, 0.0, 1.0 ), 0.8 );
}