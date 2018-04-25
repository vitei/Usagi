#ifndef _CLIP_READ_INC
#define _CLIP_READ_INC

in float vo_vClipValue;

void PerformClipping()
{
	if( bClipEnabled && vo_vClipValue > 0.0 )
   	{
   		discard;
   	}
  }

#endif


