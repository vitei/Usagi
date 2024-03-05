/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _FADER_H
#define _FADER_H

#include "Engine/Framework/EventManager.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Graphics/Device/GFXHandles.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Core/Singleton.h"

namespace usg
{
	class GFXContext;

	class Fader : public usg::Singleton<Fader>
	{
	public:
		enum {
			FADE_IN = 1,
			FADE_OUT = 2,
			FADE_WIPE = 3
		};

		enum FadeType
		{
			FADE_TYPE_GAME = 0,
			FADE_TYPE_SYSTEM,
			FADE_TYPE_COUNT
		};

		void Init(usg::GFXDevice* pDevice, const usg::RenderPassHndl& renderPass);
		void CleanUpDeviceData(usg::GFXDevice* pDevice);

		void Draw(usg::GFXContext* pContext);

		void GPUUpdate(usg::GFXDevice* pDevice);
		void Update(float fElapsed);

		void StartFade(int type, FadeType eType);

		bool IsFading(FadeType eType);

		void Blackout(FadeType eType);

		bool IsBlackout(FadeType eType);

		void ForceAlpha(float fAlpha, FadeType eType);

		float GetFadeDuration(FadeType eType) const;

		void ResetFade(FadeType eType);

	private:
		usg::PipelineStateHndl	m_pipelineState;
		usg::DescriptorSet		m_descriptorSet;
		usg::ConstantSet		m_constants;
		usg::VertexBuffer		m_VertexBuffer;

		struct FadeInfo
		{
			float fTime = 1.0f;
			int iFadeType = 0;
			float fAlpha = 0.0f;
		};

		FadeInfo m_fade[FADE_TYPE_COUNT];
	};

}

#endif // _FADER_H
