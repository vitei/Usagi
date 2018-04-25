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
	}

	RenderPassInitData::~RenderPassInitData()
	{
		if (m_bOwnsDeclaration)
		{
			if (m_decl.pAttachments)
			{
				vdelete[] m_decl.pAttachments;
				m_decl.pAttachments = NULL;
			}

			if (m_pReferences)
			{
				vdelete[] m_pReferences;
				m_pReferences = NULL;
			}

			if (m_pPreserveData)
			{
				vdelete[] m_pPreserveData;
				m_pPreserveData = NULL;
			}

			if (m_decl.pSubPasses)
			{
				vdelete[] m_decl.pSubPasses;
				m_decl.pSubPasses = NULL;
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

		ASSERT(copyData.m_bOwnsDeclaration == false);
		if (m_decl.uAttachments)
		{
			m_decl.pAttachments = vnew(ALLOC_OBJECT) RenderPassDecl::Attachment[m_decl.uAttachments];
			MemCpy(m_decl.pAttachments, copyData.m_decl.pAttachments, sizeof(RenderPassDecl::Attachment) * m_decl.uAttachments);
		}

		if (m_decl.uSubPasses)
		{
			m_decl.pSubPasses = vnew(ALLOC_OBJECT) RenderPassDecl::SubPass[m_decl.uSubPasses];
			MemCpy(m_decl.pSubPasses, copyData.m_decl.pSubPasses, sizeof(RenderPassDecl::SubPass) * m_decl.uSubPasses);
		}


		// Set up our temporary data
		{
			uint32 uReferences = 0;
			uint32 uPreserve = 0;
			for (uint32 i = 0; i < m_decl.uSubPasses; i++)
			{
				uReferences += m_decl.pSubPasses[i].uColorCount;
				uReferences += m_decl.pSubPasses[i].uInputCount;
				uReferences += m_decl.pSubPasses[i].uResolveCount;
				if (m_decl.pSubPasses[i].pDepthAttachment)
				{
					uReferences++;
				}
				uPreserve += m_decl.pSubPasses[i].uPreserveCount;
			}

			if (uReferences)
			{
				m_pReferences = vnew(ALLOC_OBJECT) RenderPassDecl::AttachmentReference[uReferences];
			}

			if (uPreserve)
			{
				m_pPreserveData = vnew(ALLOC_OBJECT) uint32[uPreserve];
			}

			m_uReferenceCount = uReferences;
			m_uPreserveCount = uPreserve;
		}

		uint32 uReferenceId = 0;
		uint32 uPreserveId = 0;
		// Now attach that data
		for (uint32 i = 0; i < m_decl.uSubPasses; i++)
		{
			if (copyData.m_decl.pSubPasses[i].uColorCount)
			{
				m_decl.pSubPasses[i].pColorAttachments = &m_pReferences[uReferenceId];
				MemCpy(m_decl.pSubPasses[i].pColorAttachments, copyData.m_decl.pSubPasses[i].pColorAttachments, sizeof(RenderPassDecl::AttachmentReference) * m_decl.pSubPasses[i].uColorCount);
				uReferenceId += m_decl.pSubPasses[i].uColorCount;
			}

			if (copyData.m_decl.pSubPasses[i].uInputCount)
			{
				m_decl.pSubPasses[i].pInputAttachments = &m_pReferences[uReferenceId];
				MemCpy(m_decl.pSubPasses[i].pInputAttachments, copyData.m_decl.pSubPasses[i].pInputAttachments, sizeof(RenderPassDecl::AttachmentReference) * m_decl.pSubPasses[i].uInputCount);
				uReferenceId += m_decl.pSubPasses[i].uInputCount;
			}

			if (copyData.m_decl.pSubPasses[i].uResolveCount)
			{
				m_decl.pSubPasses[i].pResolveAttachments = &m_pReferences[uReferenceId];
				MemCpy(m_decl.pSubPasses[i].pResolveAttachments, copyData.m_decl.pSubPasses[i].pResolveAttachments, sizeof(RenderPassDecl::AttachmentReference) * m_decl.pSubPasses[i].uResolveCount);
				uReferenceId += m_decl.pSubPasses[i].uResolveCount;
			}

			if (copyData.m_decl.pSubPasses[i].pDepthAttachment != nullptr)
			{
				m_decl.pSubPasses[i].pDepthAttachment = &m_pReferences[uReferenceId++];
				MemCpy(m_decl.pSubPasses[i].pDepthAttachment, copyData.m_decl.pSubPasses[i].pDepthAttachment, sizeof(RenderPassDecl::AttachmentReference));
			}

			if (copyData.m_decl.pSubPasses[i].uPreserveCount)
			{
				m_decl.pSubPasses[i].puPreserveIndices = &m_pPreserveData[uPreserveId];
				MemCpy(m_decl.pSubPasses[i].puPreserveIndices, copyData.m_decl.pSubPasses[i].puPreserveIndices, sizeof(uint32) * m_decl.pSubPasses[i].uPreserveCount);
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


