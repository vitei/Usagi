/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An interface for getting callbacks from sounds (primarily
//	custom streams such as movie playback)
*****************************************************************************/
#ifndef __USG_AUDIO_SOUND_CALLBACKS_H__
#define __USG_AUDIO_SOUND_CALLBACKS_H__

namespace usg
{

	class SoundCallbacks
	{
	public:
		virtual void StreamEnd() {}
		virtual void PassedEnd() {}
		virtual void BufferEnd() {}
		virtual void BufferStart() {}
		virtual void LoopEnd() {}
		virtual void Stopped() {}
	};

}

#endif

