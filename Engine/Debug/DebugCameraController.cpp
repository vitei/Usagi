#include "Engine/Common/Common.h"
#include "Engine/Framework/SystemCoordinator.h"
#include "DebugCameraController.h"


using namespace usg;

DebugCameraController::DebugCameraController(uint32 uPadId)
	: Controller(uPadId)
{
	MemSet(m_buttons, 0, sizeof(usg::Controller::MappingOutput) * DEBUG_CAM_BOOL_COUNT);
	MemSet(m_axes, 0, sizeof(usg::Controller::MappingOutput) * DEBUG_CAM_AXIS_COUNT);
}

DebugCameraController::~DebugCameraController()
{
}

void DebugCameraController::Init()
{
	ResetDetails();

	Inherited::CreateAxisMapping(GAMEPAD_AXIS_RIGHT_X, AXIS_TYPE_ABSOLUTE, m_axes[DEBUG_CAM_AXIS_ROTATE_X]);
	Inherited::CreateAxisMapping(GAMEPAD_AXIS_RIGHT_Y, AXIS_TYPE_ABSOLUTE, m_axes[DEBUG_CAM_AXIS_ROTATE_Y]);
	Inherited::CreateAxisFromButtonPair(GAMEPAD_BUTTON_L, GAMEPAD_BUTTON_R, m_axes[DEBUG_CAM_AXIS_ROTATE_Z]);
	Inherited::CreateAxisMapping(GAMEPAD_AXIS_LEFT_X, AXIS_TYPE_ABSOLUTE, m_axes[DEBUG_CAM_AXIS_MOVE_X]);
	Inherited::CreateAxisMapping(GAMEPAD_AXIS_LEFT_Y, AXIS_TYPE_ABSOLUTE, m_axes[DEBUG_CAM_AXIS_MOVE_Y]);
	Inherited::CreateAxisFromButtonPair(GAMEPAD_BUTTON_DOWN, GAMEPAD_BUTTON_UP, m_axes[DEBUG_CAM_AXIS_MOVE_Z]);
	Inherited::CreateAxisFromButtonPair(GAMEPAD_BUTTON_ZL, GAMEPAD_BUTTON_ZR, m_axes[DEBUG_CAM_AXIS_MOVE_Z]);

	Inherited::CreateKeyMapping(GAMEPAD_BUTTON_THUMB_R, m_buttons[DEBUG_CAM_BOOL_INCR_SPEED]);
	Inherited::CreateKeyMapping(GAMEPAD_BUTTON_RIGHT, m_buttons[DEBUG_CAM_BOOL_INCR_SPEED]);
	Inherited::CreateKeyMapping(GAMEPAD_BUTTON_LEFT, m_buttons[DEBUG_CAM_BOOL_DECR_SPEED]);
}

void DebugCameraController::Update(float timeDelta)
{
	ClearInputMappings();
	Inherited::Update(timeDelta);
}


void DebugCameraController::ClearInputMappings()
{
	for (int i = 0; i < DEBUG_CAM_BOOL_COUNT; i++)
	{
		m_buttons[i].Clear();
	}
	for (int i = 0; i < DEBUG_CAM_AXIS_COUNT; i++)
	{
		m_axes[i].Clear();
	}
}
