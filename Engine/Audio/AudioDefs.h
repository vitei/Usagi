/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Handle to an instance of a sound
*****************************************************************************/
#ifndef __USG_AUDIO_AUDIODEFS_H__
#define __USG_AUDIO_AUDIODEFS_H__


namespace usg{

enum SoundChannel
{
	SOUND_CHANNEL_FRONT_LEFT = 0,
	SOUND_CHANNEL_FRONT_RIGHT,
	SOUND_CHANNEL_CENTER,
	SOUND_CHANNEL_SIDE_LEFT,
	SOUND_CHANNEL_SIDE_RIGHT,
	SOUND_CHANNEL_BACK_LEFT,
	SOUND_CHANNEL_BACK_RIGHT,
	SOUND_CHANNEL_LOW_FREQ,	// Not involved in panning
	SOUND_CHANNEL_COUNT
};

enum ChannelConfig
{
	CHANNEL_CONFIG_2_0,
	CHANNEL_CONFIG_HEADPHONES,
	CHANNEL_CONFIG_2_1,
	CHANNEL_CONFIG_5_1,
	CHANNEL_CONFIG_7_1,
	CHANNEL_CONFIG_COUNT
};

struct PanningData
{
	float			fMatrix[SOUND_CHANNEL_COUNT];
	ChannelConfig	eConfig = CHANNEL_CONFIG_2_0;
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
