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

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;

// Output attributes
out vec2 vo_vPos;
out vec4 vo_vPosPos;

BUFFER_LAYOUT(1,  UBO_MATERIAL_ID) uniform Material
{
	vec4  vRcpFrameOpt;
	vec4  vRcpFrameOpt2;
};

void main(void)
{
    vec4 vPosition = vec4( ao_position.xy, -0.5, 1.0);
    
    gl_Position     = vPosition;    

    vec2 vTexCoord = vec2(0.5, 0.5) * vPosition.xy + vec2(0.5, 0.5);
    vo_vPosPos.xy = GetRTUV( vTexCoord - vRcpFrameOpt.zw );
    vo_vPosPos.zw = GetRTUV( vTexCoord + vRcpFrameOpt.zw );

    vo_vPos = GetRTUV(vTexCoord);   
}
