/****************************************************************************
//	Filename: ShipController.h
//	Description: Manages input for the ships
*****************************************************************************/
#pragma once
#include "Engine/HID/Controller.h"

namespace usg
{

class DebugCameraController : public usg::Controller
{
	typedef Controller Inherited;

public:
	DebugCameraController( uint32 uPadId =0 );
	virtual ~DebugCameraController();

	void	Init();

	virtual void Update(float timeDelta );


	enum Button
	{
		DEBUG_CAM_BOOL_INCR_SPEED = 0,
		DEBUG_CAM_BOOL_DECR_SPEED,
		DEBUG_CAM_BOOL_COUNT
	};

	enum Axis
	{
		DEBUG_CAM_AXIS_ROTATE_X,
		DEBUG_CAM_AXIS_ROTATE_Y,
		DEBUG_CAM_AXIS_ROTATE_Z,
		DEBUG_CAM_AXIS_MOVE_X,
		DEBUG_CAM_AXIS_MOVE_Y,
		DEBUG_CAM_AXIS_MOVE_Z,
		DEBUG_CAM_AXIS_COUNT
	};

	float GetFloat(Axis eAxis) const { return m_axes[eAxis].GetFloat(); }
	float GetBool(Button eButton) const { return m_buttons[eButton].GetBool(); }

private:
	void ClearInputMappings();

	MappingOutput		m_buttons[DEBUG_CAM_BOOL_COUNT];
	MappingOutput		m_axes[DEBUG_CAM_AXIS_COUNT];
};

}
