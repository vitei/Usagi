/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Handle to an instance of a sound
*****************************************************************************/
#ifndef __USG_AUDIO_AUDIODEFS_H__
#define __USG_AUDIO_AUDIODEFS_H__
#include "Engine/Common/Common.h"

namespace usg{

enum SoundChannel
{
	SOUND_CHANNEL_LEFT = 0,
	SOUND_CHANNEL_RIGHT,
	SOUND_CHANNEL_COUNT
};

struct PanningData
{
	float fMatrix[SOUND_CHANNEL_COUNT];
};


enum PLAY_STATE
{
	PLAY_STATE_PLAYING = 0,
	PLAY_STATE_PAUSED,
	PLAY_STATE_STOPPED,
	PLAY_STATE_NONE
};



const uint8 MAX_AUDIO_PRIORITY = 255;

}

#endif
