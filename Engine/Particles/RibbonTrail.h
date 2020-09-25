/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_ENGINE_PARTICLES_RIBBON_TRAIL_H_
#define _USG_ENGINE_PARTICLES_RIBBON_TRAIL_H_

#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Particles/ParticleEmitter.h"
#include "Engine/Particles/Scripted/EffectGroup.pb.h"

namespace usg
{

class RibbonTrail : public usg::ParticleEmitter
{
	typedef usg::ParticleEmitter Inherited;
public:
    RibbonTrail();
    virtual ~RibbonTrail();

    void Alloc(usg::GFXDevice* pDevice, ParticleMgr* pMgr, const particles::RibbonData* pDecl, bool bDynamicResize = false);	// Dynamic resize should be used in the editor only);
	
	virtual void Init(usg::GFXDevice* pDevice, const usg::ParticleEffect* pEffect);
	virtual void Cleanup(usg::GFXDevice* pDevice) override;
	virtual bool Update(float fElapsed);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext) override;
	virtual bool ActiveParticles();
	virtual void FreeFromPool();
	virtual void UpdateBuffers(GFXDevice* pDevice);
	virtual void SetScale(float fScale);
	void SetDeclaration(GFXDevice* pDevice, const particles::RibbonData* pDecl);
	uint32 GetMaxCount() { return m_uMaxLength; }
	bool IsLargeEnough(const particles::RibbonData& decl) { return GetRequiredVerts(decl.fLifeTime) <= GetMaxCount();}
	uint32  GetRequiredVerts(float fLifeTime) { return Math::Max(2U, (uint32)(GetTargetFrameRate()*fLifeTime)); }
	float GetTargetFrameRate() { return 60.f;  }
	float GetEmissionInterval() { return 1.f/60.f; }

	struct RibbonTrailVertex
	{
		usg::Vector3f	vPos;
		float			fEmissionTime;
		float			fEmissionLength;
	};
protected:

	PRIVATIZE_COPY(RibbonTrail)
	//PARTICLE_ALLOC(RibbonTrail)

	struct RibbonTrailVertex*	m_pCpuData;
	usg::VertexBuffer			m_vertices;
	usg::IndexBuffer			m_indices;
	RibbonTrailVertex			m_capVertex;

	usg::Vector3f				m_vPrevPos;
	float						m_fEffectTime;
	float						m_fCountdown;
	float						m_fLenAccum;
	float						m_fTimeAccum;
	float						m_fScale;
	float						m_fBaseLineWidth;
	usg::Material				m_material;
	usg::ConstantSet			m_constantSet;
	uint32						m_uNextIndex;
	uint32						m_uSetVerts;
	uint32						m_uDeclLength;
	uint32						m_uMaxLength;

	ParticleMgr*				m_pOwner;
};

}

#endif // _USG_ENGINE_PARTICLES_RIBBON_TRAIL_H_
