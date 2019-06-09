/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_SPRITE_EMITTER_H_
#define _USG_SPRITE_EMITTER_H_

#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Particles/ParticleEmitter.h"
#include "Engine/Memory/Allocator.h"

namespace usg{

class SpriteEmitter : public ParticleEmitter
{
	typedef usg::ParticleEmitter Inherited;
public:
    SpriteEmitter();
    virtual ~SpriteEmitter();

	// Metadata is used for the CPU update (should be able to remove this when abandoning the 3Ds)
	// Effects such as 
    void Alloc(GFXDevice* pDevice, uint32 uMaxCount, uint32 uVertexSize, uint32 uMetaDataSize = 0, uint32 uVerticesPerSprite = 1);
	
	virtual void Init(usg::GFXDevice* pDevice, const class ParticleEffect* pParent);
	virtual void CleanUp(GFXDevice* pDevice);
	virtual bool Update(float fElapsed);
	virtual void UpdateBuffers(GFXDevice* pDevice);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext) override;

	virtual void CalculateMaxBoundingArea(usg::Sphere& sphereOut) = 0;
	
	virtual bool ActiveParticles() { return m_uActivePart != m_uTailPart; }
	virtual void UpdateParticleCPUData(float fElapsed);

	void SetMaxCount(uint32 uMaxCount);
protected:
	virtual float InitParticleData(void* pData, void* pMetaData, float fLerp) = 0;
	// TODO: Add a CPU pointer to this so we can perform more complex updates
	virtual void UpdateParticleCPUDataInt(void* pGPUData, void* pMetaData, float fElapsed) {}
	virtual void SetMaterial(GFXContext* pContext) = 0;
	uint32 GetMaxCount() const { return m_uMaxVerts; }
	uint32 EmitParticle(uint32 uCount);
	bool Kill(uint32 uParticle);

private:
	uint8*					m_pCpuData;
	uint8*					m_pMetaData;
	float*					m_pLifeTimes;
	VertexBuffer			m_vertices;
	uint32					m_uVertexSize;
	uint32					m_uMaxVerts;
	uint32					m_uActivePart;
	uint32					m_uTailPart;
	uint32					m_uMetaDataSize;

	uint32					m_uInUse;
	bool					m_bDirty;
	SamplerHndl				m_samplerHndl;
};

}

#endif // _USG_PARTICLE_EMITTER_H_
