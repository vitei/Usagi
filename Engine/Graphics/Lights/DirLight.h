/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A visible light
*****************************************************************************/
#ifndef _USG_GRAPHICS_LIGHT_DIR_LIGHT_H_
#define _USG_GRAPHICS_LIGHT_DIR_LIGHT_H_

#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Light.h"

namespace usg {

class ShadowCascade;
class Camera;

class DirLight : public Light
{
public:
	DirLight();
	virtual ~DirLight();

	void Init(GFXDevice* pDevice, Scene* pScene, bool bSupportsShadow) override;
	void Cleanup(GFXDevice* pDevice, Scene* pScene) override;

	void UpdateCascade(const usg::Camera& camera, uint32 uContextId);
	void GPUUpdate(GFXDevice* pDevice) override;
	bool ShadowRender(GFXContext* pContext) override;

	virtual void	SetDirection(const Vector4f &direction);
	virtual const Vector4f&	GetDirection() const;

	const ShadowCascade* GetCascade() const { return m_pShadowCascade; }
	ShadowCascade* GetCascade() { return m_pShadowCascade; }

	void SetNonShadowFlags(uint32 uFlags);

	bool operator < (const DirLight& rhs) const;
protected:
	//	ConstantSet		m_constants;
	// x, y, z, -1 = spot light, 0 = positional, 1 = directional,
	Vector4f		m_direction;

private:
	DirLight(DirLight &rhs) : Light(LIGHT_TYPE_DIR) { ASSERT(false); }
	DirLight& operator=(DirLight &rhs) { ASSERT(false); return *this; }

	ShadowCascade* m_pShadowCascade;
};




}

#endif
