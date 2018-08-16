/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Scene/Scene.h"
#include "SpriteEmitter.h"

namespace usg{

SpriteEmitter::SpriteEmitter()
{
	m_pCpuData		= NULL;
	m_uActivePart 	= 0;
	m_uTailPart		= 0;
	m_uMaxVerts		= 0;
	m_bDirty		= false;


	SetLayer(LAYER_TRANSLUCENT);
}

SpriteEmitter::~SpriteEmitter()
{
	if(m_pCpuData)
	{
		mem::Free(MEMTYPE_STANDARD, (void*)m_pCpuData);
		mem::Free(MEMTYPE_STANDARD, (void*)m_pLifeTimes);
	}
	if(m_pMetaData)
	{
		mem::Free(MEMTYPE_STANDARD, (void*)m_pMetaData);
	}
}

void SpriteEmitter::Alloc(GFXDevice* pDevice, uint32 uMaxCount, uint32 uVertexSize, uint32 uMetaDataSize, uint32 uVerticesPerSprite)
{
	m_pCpuData = (uint8*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_PARTICLES, uVertexSize*uMaxCount);
	m_pLifeTimes = (float*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_PARTICLES, sizeof(float)*uMaxCount);
	m_vertices.Init( pDevice, NULL, uVertexSize, uMaxCount, "Particle", GPU_USAGE_DYNAMIC );
	m_uMaxVerts		= uMaxCount;
	m_uVertexSize	= uVertexSize;

	m_uMetaDataSize = uMetaDataSize;
	if(m_uMetaDataSize > 0)
	{
		m_pMetaData = (uint8*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_PARTICLES, m_uMetaDataSize*uMaxCount);
	}
	else
	{
		m_pMetaData = NULL;
	}

	SamplerDecl depthSamp(SF_POINT, SC_WRAP);
	m_samplerHndl = pDevice->GetSampler(depthSamp);
}

void SpriteEmitter::Init(usg::GFXDevice* pDevice, const ParticleEffect* pParent)
{
	m_uActivePart = 0;
	m_uTailPart = 0;

	Inherited::Init(pDevice, pParent);
}

void SpriteEmitter::CleanUp(GFXDevice* pDevice)
{
	m_vertices.CleanUp(pDevice);
	Inherited::CleanUp(pDevice);
}

uint32 SpriteEmitter::EmitParticle(uint32 uCount)
{
	if(!CanEmit())
		return 0;

	float fLerpInc = 1.0f/(float)uCount;
	float fLerp = 0.0f;
	while(uCount)
	{
		bool bFound = false;
		uint32 uWritePart = 0;
		if(m_uTailPart > 0)
		{
			uWritePart = m_uTailPart-1;
			m_uTailPart--;
			bFound = true;
		}
		else if(m_uActivePart < m_uMaxVerts)
		{
			 uWritePart = m_uActivePart;
			 m_uActivePart++;
			 bFound = true;
		}
		else
		{
			for(uint32 i=m_uMaxVerts-1; i>m_uTailPart; i--)
			{
				if(m_pLifeTimes[i] <= 0.0f)
				{
					uWritePart = i;
					bFound = true;
					break;
				}
			}
		}
		if(!bFound)
			break;
		// initialize the current particle and increase the count
		void* pMetaData = m_pMetaData ? m_pMetaData + (m_uMetaDataSize*uWritePart) : NULL;
		m_pLifeTimes[uWritePart] = InitParticleData(m_pCpuData + (m_uVertexSize*uWritePart), pMetaData, fLerp);
		
		m_bDirty = true;
		fLerp += fLerpInc;
		uCount--;
  }

  return uCount;
}

bool SpriteEmitter::Kill(uint32 uParticle)
{
	bool bShift = false;
	ASSERT(uParticle < m_uActivePart);
	if(uParticle != (m_uActivePart-1))
	{
		{
			uint32 uDstOffset = m_uVertexSize * uParticle;
			uint32 uSrcOffset = m_uVertexSize * (m_uActivePart-1);
			// Over-write an earlier particle
			MemCpy((void*)(m_pCpuData + uDstOffset), (void*)(m_pCpuData + uSrcOffset), m_uVertexSize);
		}
	
		if(m_pMetaData)
		{
			uint32 uDstOffset = m_uMetaDataSize * uParticle;
			uint32 uSrcOffset = m_uMetaDataSize * (m_uActivePart-1);
			MemCpy((void*)(m_pMetaData + uDstOffset), (void*)(m_pMetaData + uSrcOffset), m_uVertexSize);
		}
		m_pLifeTimes[uParticle] = m_pLifeTimes[m_uActivePart - 1];
		m_pLifeTimes[m_uActivePart-1] = 0.0f;
		bShift = true;
		m_bDirty = true;
	}
	m_uActivePart--;

	return bShift;
}

bool SpriteEmitter::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	if (m_uTailPart!=m_uActivePart)
	{
		ASSERT(!m_bDirty);
		SetMaterial(pContext);
		// For now just hard coding the depth texture to a certain slot for particle effects
		//pContext->BindTexture(5, pPostFXSys->GetPlatform().GetLinearDepthTex(), m_samplerHndl);
		pContext->SetVertexBuffer(&m_vertices);
		pContext->DrawImmediate(m_uActivePart-m_uTailPart, m_uTailPart);
	}

	return true;
}



bool SpriteEmitter::Update(float fElapsed)
{
	// Update the lifetimes
	for (sint32 sPart = m_uTailPart; sPart < (sint32)m_uActivePart; sPart++)
	{
		m_pLifeTimes[sPart] -= fElapsed;
	}

	// See if we've lost any particles from the top
	for (sint32 sPart = m_uActivePart-1; sPart >= (sint32)0; sPart--)
	{
		if (m_pLifeTimes[sPart] < 0.0f)
		{
			m_uActivePart = (uint32)sPart;
		}
		else
		{
			break;
		}
	}

	m_uTailPart = Math::Clamp(m_uTailPart, (uint32)0, m_uActivePart);

	// Now check the back
	for (sint32 sPart = m_uTailPart; sPart < (sint32)m_uActivePart; sPart++)
	{
		if (m_pLifeTimes[sPart] < 0.0f)
			m_uTailPart = (uint32)sPart;
		else
			break;
	}
	
	return true;
}

void SpriteEmitter::UpdateBuffers(GFXDevice* pDevice)
{
	if ((m_bDirty || RequiresCPUUpdate()) && m_uActivePart)
	{
		m_vertices.SetContents(pDevice, m_pCpuData, m_uActivePart);
		m_bDirty = false;
	}
}

void SpriteEmitter::UpdateParticleCPUData(float fElapsed)
{
	if(RequiresCPUUpdate())
	{
		for(uint32 i=m_uTailPart; i<m_uActivePart; i++)
		{
			if(m_pLifeTimes[i] > 0.0f)
			{
				UpdateParticleCPUDataInt(m_pCpuData + (m_uVertexSize*i), m_pMetaData ? m_pMetaData + (m_uMetaDataSize*i) : NULL, fElapsed);
			}
		}
	}

}

void SpriteEmitter::SetMaxCount(uint32 uMaxCount)
{
	m_uMaxVerts = usg::Math::Clamp(uMaxCount, (uint32)0, m_vertices.GetCount());
}

}


