/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_RENDER_PASS_INIT_DATA_H_
#define _USG_GRAPHICS_DEVICE_RENDER_PASS_INIT_DATA_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Device/StateEnums.h"
#include "Engine/Graphics/Device/RenderState.h"

namespace usg {


	class RenderPassInitData
	{
	public:
		RenderPassInitData();
		~RenderPassInitData();

		void InitForComparison(const RenderPassDecl& decl);

		// Bit of a hack, we override the assignement operator so we copy the memory
		void operator=(const RenderPassInitData &copyData);

		bool operator==(const RenderPassInitData& rhs) const;

		const RenderPassDecl& GetDecl() const { return m_decl; }

		uint32 GetReferenceCount() const;
		uint32 GetPreserveCount() const;
	private:

		RenderPassDecl					m_decl;
		void*							m_pData;

		// Only set if owns the data
		RenderPassDecl::AttachmentReference*m_pReferences;
		uint32*								m_pPreserveData;

		uint32								m_uReferenceCount;
		uint32								m_uPreserveCount;

		bool				m_bOwnsDeclaration;

	};

}


#endif

