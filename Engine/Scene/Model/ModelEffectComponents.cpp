#include "Engine/Common/Common.h"
#include "Engine/Scene/Model/ModelEffectComponents.pb.h"

namespace usg
{

	void FadeComponentRuntimeData::Update(float fDelta)
	{
		for (uint32 i = 0; i < uCurrentFades; i++)
		{
			if (pFadeData[i].fTimer > 0)
			{
				const float fElapsed = Math::Min(fDelta, pFadeData[i].fTimer);
				pFadeData[i].fTimer -= fElapsed;
				pFadeData[i].fCurrentFade += pFadeData[i].fDeltaFade*fElapsed;
				if (pFadeData[i].fTimer <= 0 && pFadeData[i].fCurrentFade > 1.0f - Math::EPSILON)
				{
					RemoveFade(pFadeData[i].uId);
					i--;
				}
				else if (pFadeData[i].fTimer <= 0)
				{
					pFadeData[i].fCurrentFade = pFadeData[i].fTargetFade;
				}
			}
		}
	}

	void FadeComponentRuntimeData::Reset()
	{
		ASSERT(pFadeData != NULL);
		uCurrentFades = 0;
		for (uint32 i = 0; i < MaxSimultaneousFades; i++)
		{
			pFadeData[i].Reset();
		}
	}

	float FadeComponentRuntimeData::GetFadeMultiplier(uint32 uId) const
	{
		for (uint32 i = 0; i < uCurrentFades; i++)
		{
			if (pFadeData[i].uId == uId)
			{
				return pFadeData[i].fCurrentFade;
			}
		}
		return 1.0f;
	}

	void FadeComponentRuntimeData::RemoveFade(uint32 uId)
	{
		for (uint32 i = 0; i < uCurrentFades; i++)
		{
			if (pFadeData[i].uId == uId)
			{
				for (uint32 j = i; j < uCurrentFades - 1; j++)
				{
					pFadeData[j] = pFadeData[j + 1];
				}
				uCurrentFades--;
			}
		}
	}

	FadeComponentRuntimeData::FadeData& FadeComponentRuntimeData::GetFadeData(uint32 uId)
	{
		for (uint32 i = 0; i < uCurrentFades; i++)
		{
			if (pFadeData[i].uId == uId)
			{
				return pFadeData[i];
			}
		}
		ASSERT(uCurrentFades < MaxSimultaneousFades);
		uCurrentFades++;
		pFadeData[uCurrentFades - 1].uId = uId;
		return pFadeData[uCurrentFades - 1];
	}

	void FadeComponentRuntimeData::SetFade(uint32 uId, float fTargetFade, float fFadeTime)
	{
		FadeData& fade = GetFadeData(uId);
		fade.fTargetFade = fTargetFade;
		if (fFadeTime > 0)
		{
			fade.fDeltaFade = (fTargetFade - fade.fCurrentFade) / fFadeTime;
			fade.fTimer = fFadeTime;
		}
		else
		{
			fade.fTimer = 0;
			fade.fCurrentFade = fTargetFade;
		}
	}

	template<>
	void OnDeactivate<FadeComponent>(Component<FadeComponent>& c, ComponentLoadHandles& handles)
	{
		FadeComponentRuntimeData& f = c.GetRuntimeData();
		if (f.pFadeData != NULL)
		{
			vdelete[] f.pFadeData;
		}
		f.pFadeData = NULL;
	}

	template<>
	void OnActivate<FadeComponent>(Component<FadeComponent>& c)
	{
		FadeComponentRuntimeData& f = c.GetRuntimeData();
		f.pFadeData = vnew(ALLOC_CONTAINER) FadeComponentRuntimeData::FadeData[FadeComponentRuntimeData::MaxSimultaneousFades];
		f.Reset();
	}
	
}