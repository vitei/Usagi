#include "../includes/platformdefines.inc"

/*============================================================================


                    NVIDIA FXAA 3.11 by TIMOTHY LOTTES


------------------------------------------------------------------------------
COPYRIGHT (C) 2010, 2011 NVIDIA CORPORATION. ALL RIGHTS RESERVED.
------------------------------------------------------------------------------
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR
LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION,
OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE
THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
DAMAGES.

#define SHARPNESS 8.0 //4.0
//#define EDGE_THRESHOLD 0.25  // Less aliasing, softer
#define EDGE_THRESHOLD 0.25   // More aliasing, sharper

#define EDGE_THRESHOLD_MIN 0.05
#define GREEN_AS_LUMA 1

#ifndef PLATFORM_PC
precision mediump float;
#endif


BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	vec4  vRcpFrameOpt;
	vec4  vRcpFrameOpt2;
};

in vec2 vo_vPos;
in vec4 vo_vPosPos;

// TODO: Try using lowp for the floats
// e.g. lowp float, ditto for the bloom

#ifdef PLATFORM_PC
#define PREC
#else
#define PREC mediump
#endif

PREC SAMPLER_LOC(1, 0) uniform sampler2D sampler0;
layout(location = 0) out PREC vec4 colorOut;


#define FxaaTexTop(t, p) textureLod(t, p, 0.0)

#if (GREEN_AS_LUMA == 0)
    #define FxaaLuma(rgba) rgba.w
#else
    #define FxaaLuma(rgba) rgba.y
#endif    

void main(void)
{
    PREC float lumaNw = FxaaLuma(FxaaTexTop(sampler0, vo_vPosPos.xy));
    PREC float lumaSw = FxaaLuma(FxaaTexTop(sampler0, vo_vPosPos.xw));
    PREC float lumaNe = FxaaLuma(FxaaTexTop(sampler0, vo_vPosPos.zy));
    PREC float lumaSe = FxaaLuma(FxaaTexTop(sampler0, vo_vPosPos.zw));

    PREC vec4 rgbyM = FxaaTexTop(sampler0, vo_vPos.xy);
    #if (GREEN_AS_LUMA == 0)
        PREC float lumaM = rgbyM.w;
    #else
        PREC float lumaM = rgbyM.y;
    #endif

    PREC float lumaMaxNwSw = max(lumaNw, lumaSw);
    lumaNe += 1.0/384.0;
    PREC float lumaMinNwSw = min(lumaNw, lumaSw);

    PREC float lumaMaxNeSe = max(lumaNe, lumaSe);
    PREC float lumaMinNeSe = min(lumaNe, lumaSe);

    PREC float lumaMax = max(lumaMaxNeSe, lumaMaxNwSw);
    PREC float lumaMin = min(lumaMinNeSe, lumaMinNwSw);

    PREC float lumaMaxScaled = lumaMax * EDGE_THRESHOLD;

    PREC float lumaMinM = min(lumaMin, lumaM);
    PREC float lumaMaxScaledClamped = max(EDGE_THRESHOLD_MIN, lumaMaxScaled);
    PREC float lumaMaxM = max(lumaMax, lumaM);
    PREC float dirSwMinusNe = lumaSw - lumaNe;
    PREC float lumaMaxSubMinM = lumaMaxM - lumaMinM;
    PREC float dirSeMinusNw = lumaSe - lumaNw;
    if(lumaMaxSubMinM < lumaMaxScaledClamped)
    {
    	colorOut = rgbyM;
    	return;
    }

    PREC vec2 dir;
    dir.x = dirSwMinusNe + dirSeMinusNw;
    dir.y = dirSwMinusNe - dirSeMinusNw;

    PREC vec2 dir1 = normalize(dir.xy);
    PREC vec4 rgbyN1 = FxaaTexTop(sampler0, vo_vPos.xy - dir1 * vRcpFrameOpt.zw);
    PREC vec4 rgbyP1 = FxaaTexTop(sampler0, vo_vPos.xy + dir1 * vRcpFrameOpt.zw);

    PREC float dirAbsMinTimesC = min(abs(dir1.x), abs(dir1.y)) * SHARPNESS;
    PREC vec2 dir2 = clamp(dir1.xy / dirAbsMinTimesC, -2.0, 2.0);

    PREC vec4 rgbyN2 = FxaaTexTop(sampler0, vo_vPos.xy - dir2 * vRcpFrameOpt2.zw);
    PREC vec4 rgbyP2 = FxaaTexTop(sampler0, vo_vPos.xy + dir2 * vRcpFrameOpt2.zw);

    PREC vec4 rgbyA = rgbyN1 + rgbyP1;
    PREC vec4 rgbyB = ((rgbyN2 + rgbyP2) * 0.25) + (rgbyA * 0.25);

    #if (GREEN_AS_LUMA == 0)
        bool twoTap = (rgbyB.w < lumaMin) || (rgbyB.w > lumaMax);
    #else
        bool twoTap = (rgbyB.y < lumaMin) || (rgbyB.y > lumaMax);
    #endif
    if(twoTap) rgbyB.xyz = rgbyA.xyz * 0.5;
    colorOut = rgbyB;
}



