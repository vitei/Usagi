/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
*****************************************************************************/

#ifndef __FADE_RUNTIEM_COMPONENT__
#define __FADE_RUNTIEM_COMPONENT__


#include "Engine/Framework/GameComponents.h"

namespace usg
{

	struct FadeComponentRuntimeData
	{
		static const uint32 MaxSimultaneousFades = 8;

		uint32 uCurrentFades;

		struct FadeData
		{
			uint32 uId;
			float fCurrentFade;
			float fTimer;
			float fDeltaFade;
			float fTargetFade;
			void Reset()
			{
				uId = 0;
				fCurrentFade = 1;
			}
		};

		FadeData* pFadeData;

		void Reset();
		float GetFadeMultiplier(uint32 uId) const;
		void RemoveFade(uint32 uId);
		FadeData& GetFadeData(uint32 uId);
		void SetFade(uint32 uId, float fTargetFade, float fFadeTime);
		void Update(float fDelta);

		float GetCurrentFade() const
		{
			float fFade = 1.0f;
			for (uint32 i = 0; i < uCurrentFades; i++)
			{
				fFade *= pFadeData[i].fCurrentFade;
			}
			return fFade;
		}
	};

	template<>
	struct RuntimeData<usg::FadeComponent> : public FadeComponentRuntimeData
	{

	};

}

#endif
