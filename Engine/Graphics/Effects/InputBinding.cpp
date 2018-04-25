/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Resource/ResourceMgr.h"
#include "InputBinding.h"

namespace usg {

InputBinding::InputBinding()
{
	for(int i=0; i<MAX_VERTEX_BUFFERS; i++)
	{
		m_uDeclId[i] = USG_INVALID_ID;
	}
	m_uDeclCount = 0;
}

InputBinding::~InputBinding()
{

}

void InputBinding::InitPlatform(GFXDevice* pDevice)
{
	const VertexDeclaration* pDecls[MAX_VERTEX_BUFFERS];

	for(uint32 i=0; i<m_uDeclCount; i++)
	{
		pDecls[i] = VertexDeclaration::GetDecl(m_uDeclId[i]);
	}

	m_platform.Init(pDevice, pDecls, m_uDeclCount);

	// FIXME: Better ID system once the system is proved
	static uint32 sBindingId = 0;
	m_uBindingId = sBindingId;
	sBindingId++;
}

void InputBinding::InitMultiStream(GFXDevice* pDevice, uint32* puDeclIds, uint32 uBufferCount)
{
	m_uDeclCount	= uBufferCount;
	for(uint32 i=0; i<m_uDeclCount; i++)
	{
		m_uDeclId[i]	= puDeclIds[i];
	}
	m_uDeclCount = uBufferCount;

	InitPlatform(pDevice);
}

void InputBinding::Init(GFXDevice* pDevice, uint32 uDecl)
{
	for(int i=0; i<MAX_VERTEX_BUFFERS; i++)
	{
		m_uDeclId[i] = USG_INVALID_ID;
	}
	m_uDeclCount = 0;

	m_uDeclId[0]	= uDecl;
	m_uDeclCount	= 1;

	InitPlatform(pDevice);
}

bool InputBinding::IsDecl(uint32* puDeclId, uint32 uDeclCount) const
{
	if(( m_uDeclCount != uDeclCount) )
	{
		return false;
	}
	
	for(uint32 i=0; i<m_uDeclCount; i++)
	{
		if(m_uDeclId[i] != puDeclId[i])
		{
			return false;
		}
	}

	return true;
}

}


