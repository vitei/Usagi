/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Constants required for the rendering class
*****************************************************************************/
#pragma once

#ifndef USG_RENDERING_CONSTS
#define USG_RENDERING_CONSTS

#include "Engine/Graphics/RenderConsts.pb.h"

namespace usg {

const int GFX_NUM_DYN_BUFF = 2;

const int MAX_VERTEX_ATTRIBUTES = 16;
const int MAX_DESCRIPTOR_SETS = 6;

class VertexDeclaration;

// TODO: Find a better location for these defines, and settle on a better naming convention for enums

const uint8	STENCIL_GEOMETRY		= 0x80;
const uint8	STENCIL_MASK_GEOMETRY	= 0x80;
const uint8	STENCIL_MASK_EFFECT		= 0x7F;


static const int MAX_COLOR_TARGETS = 8;
static const int MAX_TEXTURES		= 8;
static const int MAX_VERTEX_BUFFERS = 4;


enum ViewType
{
	VIEW_CENTRAL = 0,
	VIEW_LEFT_EYE,
	VIEW_RIGHT_EYE,
	VIEW_COUNT
};


enum FogType
{
	FOG_TYPE_LINEAR,
	FOG_TYPE_EXPONENT,
	FOG_TYPE_EXPONENT_SQUARE
};
	
enum PrimitiveType
{
	PT_POINTS = 0,
	PT_TRIANGLES,
	PT_LINES,
	PT_TRIANGLES_ADJ,
	PT_LINES_ADJ,
	PT_LINE_STRIP,
	PT_PATCH_LIST,
	PT_COUNT
	//PT_TRIANGLESTRIPS,
};


enum DeferredTargets
{
	DT_DIFFUSE_COL = 0,
	DT_LINEAR_DEPTH,
	DT_NORMAL,
	DT_EMISSIVE,
	DT_SPEC_COL
	//DT_VELOCITY,
};

enum class ShaderType : uint32
{
	VS,
	PS,
	GS,
	TC,
	TE,
	COUNT
};



enum TextureUsageFlags
{
	TU_FLAG_NONE = 0,
	TU_FLAG_TRANSFER_SRC = (1 << 0),
	TU_FLAG_FAST_MEM = (1 << 1),
	TU_FLAG_SHADER_READ = (1 << 2),
	TU_FLAG_COLOR_ATTACHMENT = (1 << 3),
	TU_FLAG_DEPTH_ATTACHMENT = (1 << 4),
	TU_FLAG_USE_HI_Z = (1 << 5),	// Depth-stencil textures only
	TU_FLAG_TRANSFER_DST = (1 << 6),	// Images updated via CPU, e.g. video images
	TU_FLAG_STORAGE_BIT = (1 << 7),		// Images used for unordered access/reads (imageLoad/ imageStore) rather than colorOut 
	TU_FLAGS_OFFSCREEN_COLOR = TU_FLAG_COLOR_ATTACHMENT | TU_FLAG_SHADER_READ | TU_FLAG_FAST_MEM,
	TU_FLAGS_FINAL_COLOR = TU_FLAG_COLOR_ATTACHMENT | TU_FLAG_TRANSFER_SRC | TU_FLAG_FAST_MEM,
	TU_FLAGS_DEPTH_BUFFER = TU_FLAG_DEPTH_ATTACHMENT | TU_FLAG_FAST_MEM | TU_FLAG_USE_HI_Z,
};

enum ColorFormat
{
	CF_RGBA_8888 = 0,
	CF_RGBA_5551,
	CF_RGB_565,
	CF_RGBA_4444,
	CF_RGB_888,
	CF_SHADOW,
	CF_RGBA_16F,
	CF_RGB_HDR,
	CF_R_32F,
	CF_R_32,
	CF_RG_32F,
	CF_R_16F,
	CF_RG_16F,
	CF_R_8,
	CF_RG_8,
	CF_NORMAL,
	CF_SRGBA,
	CF_UNDEFINED,
	CF_COUNT,
	CF_INVALID
};

enum DepthFormat
{
	DF_DEPTH_24,
	DF_DEPTH_24_S8,
	DF_DEPTH_16,
	DF_DEPTH_32F,
	DF_DEPTH_32,
	DF_COUNT,
	DF_INVALID
};

enum SamplerFilter
{
	SF_POINT = 0,
	SF_LINEAR,
	SF_COUNT
};

enum MipFilter
{
	MF_POINT = 0,
	MF_LINEAR,
	MF_COUNT
};

enum SamplerClamp
{
	SC_WRAP = 0,
	SC_MIRROR,
	SC_CLAMP,
	SC_COUNT
};


enum CompareFunc
{
	CF_NEVER = 0,
	CF_ALWAYS,
	CF_EQUAL,
	CF_NOTEQUAL,
	CF_LESS,
	CF_LEQUAL,
	CF_GREATER,
	CF_GEQUAL
};

enum ConstantType
{
	CT_MATRIX_44 = 0,
	CT_MATRIX_43,
	CT_VECTOR_4,
	CT_VECTOR_3,
	CT_VECTOR_2,
	CT_FLOAT,
	CT_INT,
	CT_VECTOR2I,
	CT_VECTOR4I,
	CT_VECTOR4U,
	CT_BOOL,
	CT_STRUCT,		// Used for arrays of objects
	CT_COUNT,
	CT_INVALID,
	CT_FORCE_SIZE = 0xFFFFFFFF
};

enum GPUUsage
{
	GPU_USAGE_STATIC = 0,
	GPU_USAGE_DYNAMIC,
	GPU_USAGE_SUBRESOURCE_UPDATE,
	GPU_USAGE_CONST_REG	// Specific to vertex buffers where we want to use the same element for every vertex
};


enum VertexInputRate
{
	VERTEX_INPUT_RATE_VERTEX = 0,
	VERTEX_INPUT_RATE_INSTANCE
};

enum DescriptorType
{
	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 0,
	DESCRIPTOR_TYPE_CONSTANT_BUFFER,
	DESCRIPTOR_TYPE_CONSTANT_BUFFER_DYNAMIC,
	DESCRIPTOR_TYPE_STORAGE_IMAGE,
	DESCRIPTOR_TYPE_INVALID
};

enum SampleCount
{
	SAMPLE_COUNT_1_BIT = 0,
	SAMPLE_COUNT_2_BIT,
	SAMPLE_COUNT_4_BIT,
	SAMPLE_COUNT_8_BIT,
	SAMPLE_COUNT_16_BIT,
	SAMPLE_COUNT_32_BIT,
	SAMPLE_COUNT_64_BIT,
	SAMPLE_COUNT_INVALID
};


// These are the CPU side sizes, the GPU sizes will vary
const uint32 g_uConstantSize[CT_COUNT] =
{
	sizeof(float)*16,	// CT_MATRIX_44 = 0,
	sizeof(float)*12,	// CT_MATRIX_43
	sizeof(float)*4,	// CT_VECTOR_4,
	sizeof(float)*3,	// CT_VECTOR_3,
	sizeof(float)*2,	// CT_VECTOR_2,
	sizeof(float),		// CT_FLOAT
	sizeof(int),		// CT_INT
	sizeof(int)*2,		// CT_VECTOR2I
	sizeof(int)*4,		// CT_VECTOR4I
	sizeof(uint32)*4,	// CT_VECTOR4U
	sizeof(bool),		// CT_BOOL
	(uint32)(-1),		// CT_STRUCT <- Size is invalid
	// CT_COUNT,
};

const VertexElementType g_veMapping[CT_COUNT] =
{
	VE_FLOAT,	// CT_MATRIX_44 = 0,
	VE_FLOAT,	// CT_MATRIX_43
	VE_FLOAT,	// CT_VECTOR_4,
	VE_FLOAT,	// CT_VECTOR_3,
	VE_FLOAT,	// CT_VECTOR_2,
	VE_FLOAT,	// CT_FLOAT
	VE_INT,		// CT_INT
	VE_INT,		// CT_VECTOR2I
	VE_INT,		// CT_VECTOR4I
	VE_UINT,	// CT_VECTOR4U
	VE_INT,		// CT_BOOL
	VE_INVALID	// CT_STRUCT
	// CT_COUNT,
};


const uint32 g_veCountMapping[CT_COUNT] =
{
	16,	// CT_MATRIX_44 = 0,
	12,	// CT_MATRIX_43
	4,	// CT_VECTOR_4,
	3,	// CT_VECTOR_3,
	2,	// CT_VECTOR_2,
	1,	// CT_FLOAT
	1,	// CT_INT
	2,	// CT_VECTOR2I
	4,	// CT_VECTOR4I
	4,	// CT_VECTOR4U
	1,		// CT_BOOL
	(uint32)(-1),		// CT_STRUCT <- Size is invalid
	// CT_COUNT,
};

const uint32 g_uConstantCPUAllignment[CT_COUNT] =
{
	sizeof(float),		// CT_MATRIX_44
	sizeof(float),		// CT_MATRIX_43
	sizeof(float),		// CT_VECTOR_4
	sizeof(float),		// CT_VECTOR_3
	sizeof(float),		// CT_VECTOR_2
	sizeof(float),		// CT_FLOAT
	sizeof(int),		// CT_INT
	sizeof(int),		// CT_VECTOR2I
	sizeof(int),		// CT_VECTOR4I
	sizeof(uint32),		// CT_VECTOR4U
	1,					// CT_BOOL
	sizeof(float) * 4	// CT_STRUCT (Not a valid size on it's own)
};


const uint32 g_uVertexElementSizes[VE_INVALID] =
{
	sizeof(sint8),  // VE_BYTE = 0,
	sizeof(uint8),  // VE_UBYTE = 1,
	sizeof(sint16), // VE_SHORT = 2,
	sizeof(uint16), // VE_USHORT = 3,
	sizeof(float),  // VE_FLOAT = 4,
	sizeof(sint32), // VE_INT = 5,
	sizeof(uint32) // VE_UINT = 6,
};

static_assert(ARRAY_SIZE(g_uConstantCPUAllignment) == CT_COUNT, "Entries in CPU alignment don't match the constant count in the ConstantType enum, has an extra value been added?");

struct ShaderConstantDecl
{
	char		 				szName[USG_IDENTIFIER_LEN];
	ConstantType				eType;
	uint32						uiCount;
	memsize						uiOffset;
	memsize						uiSize;
	const ShaderConstantDecl	*pSubDecl;
};


struct VertexElement
{
	uint32				uAttribId;
	memsize				uOffset;
	VertexElementType	eType;
	uint32				uCount;
	bool				bNormalised;
	bool				bIntegerReg;
};

struct DescriptorDeclaration
{
	uint32				uBinding;
	DescriptorType      eDescriptorType;
	uint32              uCount;
	ShaderTypeFlags	    shaderType;
};


struct GFXBounds
{
	sint32 x;
	sint32 y;
	sint32 width;
	sint32 height;
};


#define VERTEX_DATA_ELEMENT_NAME( vs_name, struct_name, element, type, count, normalised ) \
	{ vs_name, offsetof(struct_name, element), type, count, normalised }

#define VERTEX_DATA_END() \
	{ (uint32)(-1), (memsize)(-1), usg::VE_INVALID, 0, false }

#define DESCRIPTOR_ELEMENT( binding, descriptor, count, type ) \
	{ binding, descriptor, count, type }

#define DESCRIPTOR_END() \
	{ (uint32)(-1), usg::DESCRIPTOR_TYPE_INVALID, 0, usg::SHADER_FLAG_ALL }


extern const VertexElement VERTEX_ELEMENT_CAP;
extern const DescriptorDeclaration DESCRIPTOR_CAP;

} // namespace usg

#endif // USG_RENDERING_CONSTS
