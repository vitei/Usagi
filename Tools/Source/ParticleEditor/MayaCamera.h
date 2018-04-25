#ifndef _USG_PARTICLE_EDITOR_MAYA_CAMERA_H_
#define _USG_PARTICLE_EDITOR_MAYA_CAMERA_H_
#include "Engine/Common/Common.h"
#include "Engine/Maths/Quaternionf.h"
#include "Engine/Scene/Camera/StandardCamera.h"

class MayaCamera
{
public:
	MayaCamera() {}
	~MayaCamera() {}

	void Init(float fAspect);
	void Update(float fElapsed);
	usg::Camera& GetCamera() { return m_camera; }
	
private:
	void BuildCameraMatrix();
	
	usg::StandardCamera	m_camera;
	usg::Vector3f		m_vEyePosition;
	usg::Matrix4x4		m_rotation;
	usg::Vector3f		m_vLookAtPos;
};


#endif
