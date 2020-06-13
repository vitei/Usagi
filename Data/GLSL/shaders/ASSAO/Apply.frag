#include "../includes/platformdefines.inc"
#include "../includes/assao.inc"


ATTRIB_LOC(0) in vec2 vo_vTexCoord;

SAMPLER_LOC(1, 0) uniform sampler2DArray g_FinalSSAO;   


layout(location = 0) out vec4 colorOut;


void main()
{
    float ao;
    ivec2 pixPos     = ivec2(gl_FragCoord.xy);
    uvec2 pixPosHalf = pixPos / uvec2(2, 2);

    // calculate index in the four deinterleaved source array texture
    int mx = (pixPos.x % 2);
    int my = (pixPos.y % 2);
    int ic = mx + my * 2;       // center index
    int ih = (1-mx) + my * 2;   // neighbouring, horizontal
    int iv = mx + (1-my) * 2;   // neighbouring, vertical
    int id = (1-mx) + (1-my)*2; // diagonal
    
    vec2 centerVal = texelFetch( g_FinalSSAO, ivec3( pixPosHalf, ic), 0 ).xy;
    
    ao = centerVal.x;

#if 1   // change to 0 if you want to disable last pass high-res blur (for debugging purposes, etc.)
    vec4 edgesLRTB = UnpackEdges( centerVal.y );

    // return 1.0 - vec4( edgesLRTB.x, edgesLRTB.y * 0.5 + edgesLRTB.w * 0.5, edgesLRTB.z, 0.0 ); // debug show edges

    // convert index shifts to sampling offsets
    float fmx   = float(mx);
    float fmy   = float(my); 
    
    // in case of an edge, push sampling offsets away from the edge (towards pixel center)
    float fmxe  = (edgesLRTB.y - edgesLRTB.x);
    float fmye  = (edgesLRTB.w - edgesLRTB.z);

    // calculate final sampling offsets and sample using bilinear filter
    vec2  uvH = (gl_FragCoord.xy + vec2( fmx + fmxe - 0.5, 0.5 - fmy ) ) * 0.5 * g_ASSAOConsts.HalfViewportPixelSize;
    float   aoH = textureLod(g_FinalSSAO, vec3( uvH, ih ), 0 ).x;
    vec2  uvV = (gl_FragCoord.xy + vec2( 0.5 - fmx, fmy - 0.5 + fmye ) ) * 0.5 * g_ASSAOConsts.HalfViewportPixelSize;
    float   aoV = textureLod(g_FinalSSAO, vec3( uvV, iv ), 0 ).x;
    vec2  uvD = (gl_FragCoord.xy + vec2( fmx - 0.5 + fmxe, fmy - 0.5 + fmye ) ) * 0.5 * g_ASSAOConsts.HalfViewportPixelSize;
    float   aoD = textureLod(g_FinalSSAO, vec3( uvD, id ), 0 ).x;

    // reduce weight for samples near edge - if the edge is on both sides, weight goes to 0
    vec4 blendWeights;
    blendWeights.x = 1.0;
    blendWeights.y = (edgesLRTB.x + edgesLRTB.y) * 0.5;
    blendWeights.z = (edgesLRTB.z + edgesLRTB.w) * 0.5;
    blendWeights.w = (blendWeights.y + blendWeights.z) * 0.5;

    // calculate weighted average
    float blendWeightsSum   = dot( blendWeights, vec4( 1.0, 1.0, 1.0, 1.0 ) );
    ao = dot( vec4( ao, aoH, aoV, aoD ), blendWeights ) / blendWeightsSum;
#endif

    colorOut = vec4( ao.xxx, 1.0 );
}