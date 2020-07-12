#include "../includes/platformdefines.inc"
#include "../includes/assao.inc"


ATTRIB_LOC(0) in vec2 vo_vTexCoord;

SAMPLER_LOC(1, 0) uniform sampler2D g_BlurInput;   // corresponds to SSAO_TEXTURE_SLOT2

layout(location = 0) out vec2 colorOut;



vec2 SampleBlurred( vec4 inPos, vec2 coord )
{
    float packedEdges   = texelFetch(g_BlurInput, ivec2( inPos.xy ), 0 ).y;
    vec4 edgesLRTB    = UnpackEdges( packedEdges );

    vec4 valuesUL     = textureGather( g_BlurInput, coord - g_ASSAOConsts.HalfViewportPixelSize * 0.5 , 0);
    vec4 valuesBR     = textureGather( g_BlurInput, coord + g_ASSAOConsts.HalfViewportPixelSize * 0.5, 0 );

    float ssaoValue     = valuesUL.y;
    float ssaoValueL    = valuesUL.x;
    float ssaoValueT    = valuesUL.z;
    float ssaoValueR    = valuesBR.z;
    float ssaoValueB    = valuesBR.x;

    float sumWeight = 0.5f;
    float sum = ssaoValue * sumWeight;

    AddSample( ssaoValueL, edgesLRTB.x, sum, sumWeight );
    AddSample( ssaoValueR, edgesLRTB.y, sum, sumWeight );

    AddSample( ssaoValueT, edgesLRTB.z, sum, sumWeight );
    AddSample( ssaoValueB, edgesLRTB.w, sum, sumWeight );

    float ssaoAvg = sum / sumWeight;

    ssaoValue = ssaoAvg; //min( ssaoValue, ssaoAvg ) * 0.2 + ssaoAvg * 0.8;

    return vec2( ssaoValue, packedEdges );
}


void main()
{
    colorOut = SampleBlurred( gl_FragCoord, vo_vTexCoord );
}