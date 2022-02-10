/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Interface for creating the various render states
//  FIXME: Move all of this into the graphics render context
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_RENDERSTATE_H_
#define _USG_GRAPHICS_DEVICE_RENDERSTATE_H_

#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Resource/ResourceDecl.h"
#include "Engine/Graphics/Device/StateEnums.h"
#include "Engine/Graphics/Device/GFXHandles.h"

namespace usg {

class AlphaStateDecl
{
public:
	AlphaStateDecl();
	~AlphaStateDecl();

	bool operator==(const AlphaStateDecl& rhs) const;

	void SetDepthOnly();
	void SetColor0Only();
	void EnableMultipleTargets(uint32 uCount);
	bool UsesBlendColor() const;

	void InitFromDefinition(const AlphaStateGroup &def);

	bool			bBlendEnable;
	uint32			uColorTargets;
	uint32			uColorMask[MAX_COLOR_TARGETS];
	BlendFunc		srcBlend;
	BlendFunc		dstBlend;
	BlendEquation	blendEq;
	BlendFunc		srcBlendAlpha;
	BlendFunc		dstBlendAlpha;
	BlendEquation	blendEqAlpha;

	// Alpha test
	AlphaTest		eAlphaTest;
	uint8			uAlphaRef;
};

class RasterizerStateDecl
{
public:
	RasterizerStateDecl();
	~RasterizerStateDecl();

	bool operator==(const RasterizerStateDecl& rhs) const;

	CullFace		eCullFace;
	float			fDepthBias;
	float			fLineWidth;
	bool			bLineSmooth;
	bool			bUseDepthBias;
	bool			bMultisample;
	bool			bWireframe;
	// 0 indicates no tessellation
	uint32			uPatchControlPoints;	
};


// For getting things up and running 
class RenderPassDecl
{
public:
	RenderPassDecl();
	~RenderPassDecl();

	enum AttachmentFlags
	{
		AF_MEMORY_REUSE = (1<<0),	// Sharing memory with another target		
	};

	enum StageFlags
	{
		SF_BOTTOM_OF_PIPE = (1 << 0),
		SF_COLOR_ATTACHMENT_OUTPUT = (1 << 1)
	};

	enum AccessFlags
	{
		AC_MEMORY_READ = (1 << 0),
		AC_COLOR_ATTACHMENT_READ = (1 << 1),
		AC_COLOR_ATTACHMENT_WRITE = (1 << 2),
		AC_SHADER_READ_BIT = (1<<3),
		AC_SHADER_WRITE_BIT = (1<<4),	// Image store rather than write as attachment
		AC_TRANSFER_WRITE_BIT = (1<<5)
	};

	enum AttachmentLoadOp
	{
		LOAD_OP_LOAD_MEMORY,
		LOAD_OP_CLEAR_MEMORY,
		LOAD_OP_DONT_CARE
	};

	enum AttachmentStoreOp
	{
		STORE_OP_STORE,
		STORE_OP_DONT_CARE
	};

	enum AttachmentType
	{
		ATTACH_COLOR,
		ATTACH_DEPTH
	};

	enum AttachmentLayout
	{
		LAYOUT_UNDEFINED = 0,
		LAYOUT_COLOR_ATTACHMENT,
		LAYOUT_DEPTH_STENCIL_ATTACHMENT,
		LAYOUT_SHADER_READ_ONLY,
		LAYOUT_TRANSFER_SRC,
		LAYOUT_PRESENT_SRC
	};

	enum Defines
	{
		SUBPASS_EXTERNAL = USG_INVALID_ID
	};
	
	struct Attachment
	{
		Attachment();
		AttachmentType		eAttachType;
		union
		{
			ColorFormat eColor;
			DepthFormat eDepth;
		} format;
		uint32				uAttachFlags;
		AttachmentLoadOp	eLoadOp;
		AttachmentStoreOp	eStoreOp;
		SampleCount			eSamples;
		AttachmentLayout	eInitialLayout;	// The format the texture is in when the pass begins
		AttachmentLayout	eFinalLayout;	// The layout the image should be in at the end of the render pass
	};

	struct AttachmentReference
	{
		uint32			 uIndex;
		AttachmentLayout eLayout;
	};

	// Keep all pointers ordered for the offset
	struct SubPass
	{
		SubPass();

		// Attachements which can be read during this sub pass
		const AttachmentReference*	pInputAttachments;	
		uint32						uInputCount;
		// Bound render target information
		const AttachmentReference*	pColorAttachments;
		uint32						uColorCount;
		const AttachmentReference*	pDepthAttachment;
		// Multi-sample resolves
		const AttachmentReference*	pResolveAttachments;
		// Attachments not used by this subpass but which must not be altered
		uint32*						puPreserveIndices;
		uint32						uPreserveCount;
	};

	struct Dependency
	{
		Dependency();

		// Set src or dst to SUBPASS_EXTERNAL if they are being referenced externally
		uint32	uSrcSubPass;
		uint32	uDstSubPass;
		uint32  uSrcStageFlags;
		uint32  uDstStageFlags;
		uint32  uSrcAccessFlags;
		uint32  uDstAccessFlags;
	};

	// All the attachments used by this render pass
	const Attachment*	pAttachments;
	uint32				uAttachments;
	// List of passes
	const SubPass* 		pSubPasses;
	uint32				uSubPasses;
	// Dependencies both internal and external. 
	const Dependency*	pDependencies;
	uint32				uDependencies;

	bool operator==(const RenderPassDecl& rhs) const;


	// Single subpass utility functions
	// Dependency on another subpass for input
	static const Dependency* ExternalColorDependencyIn();
	// Another subpass is dependent on this pass
	static const Dependency* ExternalColorDependencyOut();
	// Array of the above two dependencies basically forcing each pass to execute in order
	static const Dependency* ExternalColorDependenciesInAndOut();
};

class DepthStencilStateDecl
{
public:
	DepthStencilStateDecl();
	~DepthStencilStateDecl();

	void SetMask(uint8 uReadMask, uint8 uWriteMask, uint8 uRef, bool bFront = true, bool bBack = true);
	void SetOperation(StencilOp ePassOp, StencilOp eStencilFailOp, StencilOp eDepthFailOp, bool bFront = true, bool bBack = true);
	bool operator==(const DepthStencilStateDecl& rhs) const;

	// Depth operations
	bool			bDepthEnable;
	bool			bDepthWrite;
	DepthTest		eDepthFunc;

	// Stencil operations
	bool			bStencilEnable;
	StencilTest		eStencilTest;
	StencilOp		eStencilOps[SOP_TYPE_COUNT];

	// Stencil Ref
	uint8			uMask[STENCIL_REF_TYPE];
};


class PipelineLayoutDecl
{
public:
	PipelineLayoutDecl();
	~PipelineLayoutDecl();

	bool operator==(const PipelineLayoutDecl& rhs) const
	{
		if (uDescriptorSetCount != rhs.uDescriptorSetCount)
			return false;

		for (uint32 i = 0; i < uDescriptorSetCount; i++)
		{
			if (descriptorSets[i] != rhs.descriptorSets[i])
				return false;
		}

		return true;
	}

	enum
	{
		MAX_DESCRIPTOR_SETS = 5
	};

	DescriptorSetLayoutHndl		descriptorSets[MAX_DESCRIPTOR_SETS];
	uint32						uDescriptorSetCount;
};


class PipelineStateDecl
{
public:

	PipelineStateDecl();
	~PipelineStateDecl();

	class InputBinding
	{
	public:
		InputBinding();
		~InputBinding() {}
		void Init(const VertexElement* pElementIn, uint32 uBindSlotIn = 0, VertexInputRate eRateIn = VERTEX_INPUT_RATE_VERTEX, uint32 uStepIn = 0);
		uint32					uBindSlot;
		VertexInputRate			eStepRate;
		uint32					uStepSize;
		uint32					uVertexSize; // 0 for auto calculate
		const VertexElement*	pElements;
		// TODO: Add size including padding?
	};

	EffectHndl				pEffect;
	InputBinding			inputBindings[MAX_VERTEX_BUFFERS];
	uint32					uInputBindingCount;

	RasterizerStateDecl		rasterizerState;
	DepthStencilStateDecl   depthState;
	AlphaStateDecl			alphaState;
	PipelineLayoutDecl		layout;
	
	SampleCount				eSampleCount;

	PrimitiveType			ePrimType;

};

// TODO: Anisotropy etc
class SamplerDecl
{
public:
	SamplerDecl();
	SamplerDecl(SamplerFilter eFilterIn, SamplerWrap eClampIn);
	~SamplerDecl();

	void SetFilter(SamplerFilter eFilter);
	void SetClamp(SamplerWrap eClamp);

	bool operator==(const SamplerDecl& rhs) const;

	SamplerFilter	eFilterMin;
	SamplerFilter	eFilterMag;

	SamplerWrap	eClampU;
	SamplerWrap	eClampV;

	CompareFunc		eCmpFnc;
	Anisotropy		eAnisoLevel;
	MipFilter		eMipFilter;
	bool			bEnableCmp;

	bool			bProjection;

	float32			LodBias;
	uint32			LodMinLevel;
};


}


#endif

