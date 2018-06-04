/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "RenderPassInitData.h"
#include "Engine/Core/stl/vector.h"

namespace usg {

	RenderPassInitData::RenderPassInitData()
	{
		m_pReferences = NULL;
		m_pPreserveData = NULL;
		m_bOwnsDeclaration = false;
		m_pData = nullptr;
	}

	RenderPassInitData::~RenderPassInitData()
	{
		if (m_bOwnsDeclaration)
		{
			if (m_pData)
			{
				mem::Free(m_pData);
				m_pData = nullptr;
			}
		}
	}

	void RenderPassInitData::InitForComparison(const RenderPassDecl& decl)
	{
		m_decl = decl;
		m_bOwnsDeclaration = false;
	}

	// Bit of a hack, we override the assignement operator so we copy the memory
	void RenderPassInitData::operator=(const RenderPassInitData &copyData)
	{
		m_decl = copyData.m_decl;

		// Set up our temporary data
		uint32 uReferences = 0;
		uint32 uPreserve = 0;
		{
			for (uint32 i = 0; i < m_decl.uSubPasses; i++)
			{
				uReferences += m_decl.pSubPasses[i].uColorCount;
				uReferences += m_decl.pSubPasses[i].uInputCount;
				uReferences += m_decl.pSubPasses[i].pResolveAttachments ? m_decl.pSubPasses[i].uColorCount : 0;
				if (m_decl.pSubPasses[i].pDepthAttachment)
				{
					uReferences++;
				}
				uPreserve += m_decl.pSubPasses[i].uPreserveCount;
			}

			m_uReferenceCount = uReferences;
			m_uPreserveCount = uPreserve;
		}

		uint32 uAttachSize = sizeof(RenderPassDecl::Attachment) * m_decl.uAttachments;
		uint32 uSubPassSize = sizeof(RenderPassDecl::SubPass) * m_decl.uSubPasses;
		uint32 uDependencySize = sizeof(RenderPassDecl::Dependency) * m_decl.uDependencies;
		uint32 uReferencesSize = sizeof(RenderPassDecl::AttachmentReference) * uReferences;
		uint32 uPreserveSize = sizeof(uint32)*uPreserve;
		uint32 uTotalSize = uAttachSize + uSubPassSize + uDependencySize + uReferencesSize + uPreserveSize;

		m_pData = mem::Alloc(MEMTYPE_STANDARD, ALLOC_GFX_INTERNAL, uTotalSize);
		uint8* pData = (uint8*)m_pData;


		// FIXME: Switch to a single allocation
		ASSERT(copyData.m_bOwnsDeclaration == false);
		if (m_decl.uAttachments)
		{
			RenderPassDecl::Attachment* pCpyData = (usg::RenderPassDecl::Attachment*)pData;
			m_decl.pAttachments = pCpyData;
			MemCpy(pCpyData, copyData.m_decl.pAttachments, uAttachSize);
			pData += uAttachSize;
		}

		RenderPassDecl::SubPass* pSubPasses = (usg::RenderPassDecl::SubPass*)pData;
		if (m_decl.uSubPasses)
		{
			m_decl.pSubPasses = pSubPasses;
			MemCpy(pSubPasses, copyData.m_decl.pSubPasses, sizeof(RenderPassDecl::SubPass) * m_decl.uSubPasses);
			pData += uSubPassSize;
		}

		if (m_decl.uDependencies)
		{
			RenderPassDecl::Dependency* pCpyData = (usg::RenderPassDecl::Dependency*)pData;
			m_decl.pDependencies = pCpyData;
			MemCpy(pCpyData, copyData.m_decl.pDependencies, sizeof(RenderPassDecl::Dependency) * m_decl.uDependencies);
			pData += uDependencySize;
		}

		if (uReferences)
		{
			m_pReferences = (RenderPassDecl::AttachmentReference*)pData;
			pData += uReferencesSize;
		}

		if (uPreserve)
		{
			m_pPreserveData = (uint32*)pData;
			pData += uPreserveSize;
		}

		ASSERT((pData - uTotalSize) == m_pData);


		uint32 uReferenceId = 0;
		uint32 uPreserveId = 0;
		// Now attach that data
		for (uint32 i = 0; i < m_decl.uSubPasses; i++)
		{
			if (copyData.m_decl.pSubPasses[i].uColorCount)
			{
				pSubPasses[i].pColorAttachments = &m_pReferences[uReferenceId];
				MemCpy((void*)pSubPasses[i].pColorAttachments, copyData.m_decl.pSubPasses[i].pColorAttachments, sizeof(RenderPassDecl::AttachmentReference) * m_decl.pSubPasses[i].uColorCount);
				uReferenceId += m_decl.pSubPasses[i].uColorCount;
			}

			if (copyData.m_decl.pSubPasses[i].uInputCount)
			{
				pSubPasses[i].pInputAttachments = &m_pReferences[uReferenceId];
				MemCpy((void*)pSubPasses[i].pInputAttachments, copyData.m_decl.pSubPasses[i].pInputAttachments, sizeof(RenderPassDecl::AttachmentReference) * m_decl.pSubPasses[i].uInputCount);
				uReferenceId += m_decl.pSubPasses[i].uInputCount;
			}

			if (copyData.m_decl.pSubPasses[i].pResolveAttachments)
			{
				pSubPasses[i].pResolveAttachments = &m_pReferences[uReferenceId];
				MemCpy((void*)m_decl.pSubPasses[i].pResolveAttachments, copyData.m_decl.pSubPasses[i].pResolveAttachments, sizeof(RenderPassDecl::AttachmentReference) * m_decl.pSubPasses[i].uColorCount);
				uReferenceId += m_decl.pSubPasses[i].uColorCount;
			}

			if (copyData.m_decl.pSubPasses[i].pDepthAttachment != nullptr)
			{
				pSubPasses[i].pDepthAttachment = &m_pReferences[uReferenceId++];
				MemCpy((void*)pSubPasses[i].pDepthAttachment, copyData.m_decl.pSubPasses[i].pDepthAttachment, sizeof(RenderPassDecl::AttachmentReference));
			}

			if (copyData.m_decl.pSubPasses[i].uPreserveCount)
			{
				pSubPasses[i].puPreserveIndices = &m_pPreserveData[uPreserveId];
				MemCpy((void*)pSubPasses[i].puPreserveIndices, copyData.m_decl.pSubPasses[i].puPreserveIndices, sizeof(uint32) * m_decl.pSubPasses[i].uPreserveCount);
				uPreserveId += m_decl.pSubPasses[i].uPreserveCount;
			}
		}

		m_bOwnsDeclaration = true;

	}

	bool RenderPassInitData::operator==(const RenderPassInitData& rhs) const
	{
		return rhs.m_decl == m_decl;
	}


	uint32 RenderPassInitData::GetReferenceCount() const
	{
		ASSERT(m_bOwnsDeclaration);
		return m_uReferenceCount;
	}

	uint32 RenderPassInitData::GetPreserveCount() const
	{
		ASSERT(m_bOwnsDeclaration);
		return m_uPreserveCount;
	}
}


