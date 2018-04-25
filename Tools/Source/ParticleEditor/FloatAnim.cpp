#include "Engine/Common/Common.h"
#include "FloatAnim.h"


void FloatAnim::Init(usg::GUIWindow* pWindow, const char* szName)
{
	int frameCount = 1;
	m_frameCount.Init("Frame count", &frameCount, 1, 1, usg::particles::FloatAnim::frames_max_count);

	usg::Vector2f vPos(0.0f, 0.0f);
	usg::Vector2f vScale(270.f, 80.f);
	m_childWindow.Init(szName, vPos, vScale, 11, usg::GUIWindow::WINDOW_TYPE_CHILD );
	pWindow->AddItem(&m_childWindow);

	m_title.Init(szName);
	m_childWindow.AddItem(&m_title);
	m_childWindow.AddItem(&m_frameCount);
	char szBoxName[USG_IDENTIFIER_LEN];
	float defaults[2] = { 0.0f, 0.0f };
	for(uint32 i=0; i<MAX_FRAME_COUNT; i++)
	{
		str::ParseVariableArgsC(szBoxName, USG_IDENTIFIER_LEN, "Key %d", i);
		m_frameInfo[i].Init(szBoxName, 0.0f, 10.f, defaults, 2);
		m_frameInfo[i].SetVisible(false);
		m_childWindow.AddItem(&m_frameInfo[i]);
	}

	m_frameInfo[0].SetVisible(true);
}

void FloatAnim::SetVisible(bool bVisible)
{
	m_childWindow.SetVisible(bVisible);
}

void FloatAnim::SetFromDefinition(usg::particles::FloatAnim &src)
{
	uint32 iFrameIndex = 0;
	int frameCount = (int)src.frames_count;
	m_frameCount.SetValues(&frameCount);
	
	// Force the first frame to be time 0
	src.frames[0].fTimeIndex = 0.0f;

	for(iFrameIndex = 0; iFrameIndex < src.frames_count; iFrameIndex++)
	{
		m_frameInfo[iFrameIndex].SetValue(src.frames[iFrameIndex].fTimeIndex, 0);
		m_frameInfo[iFrameIndex].SetValue(src.frames[iFrameIndex].fValue, 1);
	}
}


bool FloatAnim::Update(usg::particles::FloatAnim &src)
{
	bool bAltered = false;
	int frameCount = m_frameCount.GetValue()[0];
	usg::Math::Clamp(frameCount, 1, 10);
	m_frameCount.SetValues(&frameCount);
	if(frameCount != src.frames_count)
	{
		bAltered = true;
		src.frames_count = frameCount;
	}

	uint32 iFrameIndex = 0;
	for(iFrameIndex = 0; iFrameIndex < frameCount; iFrameIndex++)
	{
		if(m_frameInfo[iFrameIndex].GetValue(0) != src.frames[iFrameIndex].fTimeIndex 
			|| m_frameInfo[iFrameIndex].GetValue(1) != src.frames[iFrameIndex].fValue )
		{
			bAltered = true;
		}
		
		{
			if(iFrameIndex > 0)
			{
				if(src.frames[iFrameIndex-1].fTimeIndex > src.frames[iFrameIndex].fTimeIndex)
				{
					m_frameInfo[iFrameIndex].SetValue(src.frames[iFrameIndex-1].fTimeIndex);
				}
			}
			else
			{
				m_frameInfo[iFrameIndex].SetValue(0.0f, 0);
			}

			src.frames[iFrameIndex].fTimeIndex = m_frameInfo[iFrameIndex].GetValue(0);
			src.frames[iFrameIndex].fValue = m_frameInfo[iFrameIndex].GetValue(1);
		}
		m_frameInfo[iFrameIndex].SetVisible(true);
	}

	for(;iFrameIndex<MAX_FRAME_COUNT;iFrameIndex++)
	{
		m_frameInfo[iFrameIndex].SetVisible(false);
	}

	return bAltered;
}