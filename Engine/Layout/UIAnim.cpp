/****************************************************************************
//	Filename: UIUtiles.h
*****************************************************************************/
#pragma once

#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector2f.h"
#include "Engine/Maths/MathUtil.h"
#include "UI.h"
#include "UIAnim.h"

namespace usg
{

	UIAnim::UIAnim()
		: m_vUVMin(0.0f, 0.0f)
		, m_vUVWidth(1.0f, 1.0f)
		, m_uFramesX(0)
		, m_uFramesY(0)
		, m_uStartFrame(0)
		, m_uEndFrame(0)
		, m_bLoop(false)
		, m_fFrameRate(30.0f)
		, m_fAccum(0.0f)
		, m_fStartDelay(0.0f)
		, m_fEndDelay(0.0f)
		, m_uCachedFrameId(USG_INVALID_ID)
	{

	}

	UIAnim::~UIAnim()
	{

	}

	void UIAnim::SetUVRange(const usg::Vector2f& vUVMin, const usg::Vector2f& vUVMax)
	{
		m_vUVMin = vUVMin;
		m_vUVWidth = vUVMax - vUVMin;
	}

	void UIAnim::Init(const char* szImageName, class UI* pUI, uint32 uFramesX, uint32 uFramesY)
	{
		bool bRet = pUI->GetItemRef(szImageName, UI_ITEM_IMAGE, m_itemRef);
		ASSERT(bRet);
		m_uFramesX = uFramesX;
		m_uFramesY = uFramesY;
	}

	void UIAnim::SetAnimation(uint32 uStartFrame, uint32 uEndFrame, float fFrameRate, bool bLoop)
	{
		m_uStartFrame = uStartFrame;
		m_uEndFrame = uEndFrame;
		m_fFrameRate = fFrameRate;
		m_bLoop = bLoop;
		m_fAccum = 0.0f;
	}

	void UIAnim::Update(float fDelta, class UI* pUI)
	{
		uint32 uTotalFrames = GetTotalFrames();
		float fTotalDuration = GetTotalDuration();
		m_fAccum += fDelta;
		if (m_fAccum > fTotalDuration)
		{
			if (m_bLoop)
			{
				while (m_fAccum > fTotalDuration)
				{
					m_fAccum -= fTotalDuration;
				}
			}
			else
			{
				m_fAccum = fTotalDuration;
			}
		}

		float fFrame = usg::Math::Max((m_fAccum - m_fStartDelay) * m_fFrameRate, 0.0f);

		uint32 uFrame = (uint32)fFrame;

		if (uFrame != m_uCachedFrameId)
		{
			uFrame = usg::Math::Clamp(uFrame, 0U, uTotalFrames - 1);

			usg::Vector2f vUVMin;
			usg::Vector2f vUVMax;

			uint32 uXId = uFrame % m_uFramesX;
			uint32 uYId = uFrame / m_uFramesY;

			usg::Vector2f vCellWidth = m_vUVWidth / usg::Vector2f((float)m_uFramesX, (float)m_uFramesY);

			vUVMin.x = (float)uXId * vCellWidth.x;
			vUVMin.y = (float)uYId * vCellWidth.y;

			vUVMin += m_vUVMin;

			vUVMax = vUVMin + vCellWidth;;

			pUI->SetUVRange(m_itemRef, vUVMin, vUVMax);

			m_uCachedFrameId = uFrame;
		}

	}

	uint32 UIAnim::GetTotalFrames() const
	{
		uint32 uTotalFrames = (m_uEndFrame - m_uStartFrame) + 1;

		return uTotalFrames;
	}


	float UIAnim::GetTotalDuration() const
	{
		uint32 uTotalFrames = GetTotalFrames();

		float fTotalDuration = uTotalFrames / m_fFrameRate;
		fTotalDuration += m_fStartDelay + m_fEndDelay;
		return fTotalDuration;
	}

	bool UIAnim::IsFinished() const
	{
		return !m_bLoop && m_fAccum >= GetTotalDuration();
	}

	void UIAnim::SetDelay(float fStartDelay, float fEndDelay)
	{
		m_fStartDelay = fStartDelay;
		m_fEndDelay = fEndDelay;
	}

}
