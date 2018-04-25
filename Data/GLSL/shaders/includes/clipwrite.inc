#ifndef _CLIP_WRITE_INC
#define _CLIP_WRITE_INC

out float vo_vClipValue;

// Fiddling with the projection matrix leads to problems with vertex shaders so
// we calculate the clipping here
void CalculateClipDistance(vec4 vPos)
{
	vo_vClipValue = dot( vClipPlane, vPos );
}

#endif

