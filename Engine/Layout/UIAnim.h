/****************************************************************************
//	Filename: UIUtiles.h
*****************************************************************************/
#pragma once

#include "Engine/Scene/RenderNode.h"
#include "Engine/Maths/Vector2f.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Core/stl/map.h"
#include "Engine/Core/stl/string.h"
#include "Engine/Layout/UI.pb.h"

namespace usg
{

	class UIAnim
	{
	public:
		UIAnim();
		virtual ~UIAnim();

		void Init(const char* szImageName, class UI* pUI, uint32 uFramesX, uint32 uFramesY);
		void SetAnimation(uint32 uStartFrame, uint32 uEndFrame, float fFrameRate, bool bLoop);
		void SetUVRange(const usg::Vector2f& vUVMin, const usg::Vector2f& vUVMax);
		void Update(float fDelta, class UI* pUI);
		void SetDelay(float fStartDelay, float fEndDelay);
		bool IsFinished() const;
		uint32 GetTotalFrames() const;

	private:
		float GetTotalDuration() const;
		UIItemRef		m_itemRef;
		usg::Vector2f	m_vUVMin;
		usg::Vector2f	m_vUVWidth;
		uint32			m_uFramesX;
		uint32			m_uFramesY;
		uint32 			m_uStartFrame;
		uint32			m_uEndFrame;
		bool			m_bLoop;
		float			m_fFrameRate;
		float			m_fAccum;
		float			m_fStartDelay;
		float			m_fEndDelay;
		uint32			m_uCachedFrameId;
	};

}
