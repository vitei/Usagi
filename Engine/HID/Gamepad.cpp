/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemUtil.h"
#include "IGamepad.h"
#include "Gamepad.h"

namespace usg{

Gamepad::Gamepad()
	: m_pIGamepad(nullptr)
	, m_uCaps(0)
	, m_bConnected(false)
{
	MemClear(&m_deviceState, sizeof(m_deviceState));
	m_deviceState.leftHand.LoadIdentity();
	m_deviceState.rightHand.LoadIdentity();
}

Gamepad::~Gamepad()
{

}

const char* Gamepad::GetName() const
{
	if (m_pIGamepad)
	{
		return m_pIGamepad->GetProductName();
	}
	return "";
}


bool Gamepad::GetAnyButtonDown(ButtonState eState) const
{
	uint64 uButtonMask = 0xFFFFFFFFFFFFFFFF;
	for(uint32 uIndex=0; uIndex<ARRAY_SIZE(m_deviceState.uButtonsDown); uIndex++)
	{
		switch (eState)
		{
		case BUTTON_STATE_PRESSED:
		{
			if(((m_deviceState.uButtonsDown[uIndex] & uButtonMask) != 0) && ((m_deviceState.uButtonsPrevDown[uIndex] & uButtonMask) == 0))
				return true;
		}
		case BUTTON_STATE_RELEASED:
		{
			if(((m_deviceState.uButtonsPrevDown[uIndex] & uButtonMask) != 0) && ((m_deviceState.uButtonsDown[uIndex] & uButtonMask) == 0))
				return true;
		}
		case BUTTON_STATE_HELD:
		{
			if((m_deviceState.uButtonsDown[uIndex] & uButtonMask) != 0)
				return true;
		}
		default:
			break;
		}
	}
	return false;
}


bool Gamepad::GetButtonDown(uint32 uButton, ButtonState eState) const
{
	uButton -= 1;	// 0 = NONE
	uint32 uIndex = uButton / 32;
	uButton -= (uIndex * 32);

	uint32 uButtonMask = 1 << (uButton);

	switch(eState)
	{
	case BUTTON_STATE_PRESSED:
		{
			return ((m_deviceState.uButtonsDown[uIndex] & uButtonMask) != 0) && ((m_deviceState.uButtonsPrevDown[uIndex] & uButtonMask) == 0);
		}
	case BUTTON_STATE_RELEASED:
		{
			return ((m_deviceState.uButtonsPrevDown[uIndex] &uButtonMask)!=0)&&((m_deviceState.uButtonsDown[uIndex] &uButtonMask)==0);
		}
	case BUTTON_STATE_HELD:
		{
			return ((m_deviceState.uButtonsDown[uIndex]&uButtonMask)!=0);
		}
	case BUTTON_STATE_CONTACT:
		{
		return ((m_deviceState.uButtonsContact&uButtonMask) != 0);
		}
	default:
		ASSERT(false);	// Unhandled state
	}
	return false;
}

float Gamepad::GetAxisValue(uint32 uAxis) const
{
	ASSERT(uAxis < _GamepadAxis_count);
	return m_deviceState.fAxisValues[uAxis];
}

void Gamepad::GetMatrix(Matrix4x4& out, GamepadHand eHand)
{
	if (eHand == GamepadHand::Left)
	{
		out = m_deviceState.leftHand;
	}

	out = m_deviceState.rightHand;
}

void Gamepad::Update(usg::GFXDevice* pDevice, float fDelta)
{
	float fLeft = 0.0f; 
	float fRight = 0.0f;
	vector<uint32> eraseList;
	for (auto& itr : m_vibrations)
	{
		fLeft += itr.second.fLeftStrength;//Math::Max(fLeft, itr.second.fLeftStrength);
		fRight += itr.second.fRightStrength;//Math::Max(fLeft, itr.second.fRightStrength);

		if (itr.second.fDuration >= 0.0f)
		{
			itr.second.fDuration -= fDelta;
			if (itr.second.fDuration <= 0.0f)
			{
				eraseList.push_back(itr.first);
			}
		}
	}

	fLeft = Math::Clamp01(fLeft);
	fRight = Math::Clamp01(fRight);

	for (auto itr : eraseList)
	{
		m_vibrations.erase(itr);
	}

	if (m_pIGamepad)
	{
		m_pIGamepad->Update(pDevice, m_deviceState);
		m_pIGamepad->Vibrate(fLeft, fRight);
		m_bConnected = m_pIGamepad->IsConnected();

	}
	else
	{
		m_uCaps = 0;
	}
}

void Gamepad::ResetGyro()
{
	m_pIGamepad->ResetGyro();
}

void Gamepad::ResetGyroDirection()
{
	m_pIGamepad->ResetGyroDirection();
}


void Gamepad::BindHardware(IGamepad* pGamepad)
{
	m_pIGamepad = pGamepad;
	if (m_pIGamepad)
	{
		m_uCaps = m_pIGamepad->GetCaps();
		m_bConnected = m_pIGamepad->IsConnected();
	}
}

uint32 Gamepad::Vibrate(float fLeft, float fRight, float fDuration)
{
	VibrationData data;
	data.fLeftStrength = fLeft;
	data.fRightStrength = fRight;
	data.fDuration = fDuration;
	uint32 uId = Math::Rand();
	m_vibrations[uId] = data;

	return uId;
}

void Gamepad::UpdateVibration(uint32 uHandle, float fLeft, float fRight)
{
	auto& itr = m_vibrations.find(uHandle);
	if (itr != m_vibrations.end())
	{
		itr->second.fLeftStrength = fLeft;
		itr->second.fRightStrength = fRight;
	}
}

void Gamepad::StopEffect(uint32 uHandle)
{
	auto& itr = m_vibrations.find(uHandle);
	if (itr != m_vibrations.end())
	{
		m_vibrations.erase(itr);
	}
}

void Gamepad::StopAllVibrations()
{
	m_vibrations.clear();
}

}
