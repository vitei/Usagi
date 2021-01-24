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


		void Init(usg::GFXDevice* pDevice, const usg::RenderPassHndl& renderPass);
		void CleanUpDeviceData(usg::GFXDevice* pDevice);

		void Draw(usg::GFXContext* pContext, bool upper);

		void GPUUpdate(usg::GFXDevice* pDevice);
		void Update(float fElapsed);

		void StartFade(int type, bool bWipeLower = true);

		bool IsFading();

		void Blackout();

		bool IsBlackout();

		void ForceAlpha(float fAlpha);

	private:
		usg::PipelineStateHndl	m_pipelineState;
		usg::DescriptorSet		m_descriptorSet;
		usg::ConstantSet		m_constants;
		usg::VertexBuffer		m_VertexBuffer;
		bool					m_bWipeLower;
	};

}

#endif // _FADER_H
