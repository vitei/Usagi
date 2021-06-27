/****************************************************************************
//	Usagi Engine, Copyright � Vitei, Inc. 2013
//	Description: Base class for handling the state of an animation resource
*****************************************************************************/
#ifndef _USG_SCENE_MODEL_ANIMATION_BASE_H_
#define _USG_SCENE_MODEL_ANIMATION_BASE_H_


#include "Engine/Resource/ResourceDecl.h"

namespace usg {

class Model;
class ModelResource;

class AnimationBase
{
public:
	AnimationBase();
	virtual ~AnimationBase();

	virtual void Reset();
	
	// TODO: Weighting probably belongs as meta data in the model anim player
	void SetWeighting(float fWeighting) { m_fWeighting = fWeighting; }
	float GetWeighting() const { return m_fWeighting;  }
	void Play();
	void Stop() { m_bActive = false;  }
	void SetSpeed(float fSpeed) { m_fPlaybackSpeed = fSpeed;  }

	bool IsPlaying() const { return m_bActive; }


protected:
	void UpdateInt(float fElapsed, float fFrameRate, float fFrameCount, bool bLoop);

	float								m_fWeighting;
	float								m_fPlaybackSpeed;
	float								m_fActiveFrame;
	bool								m_bActive;
};

}

#endif	// #ifndef _USG_SCENE_MODEL_SKELETAL_ANIMATION_H_

