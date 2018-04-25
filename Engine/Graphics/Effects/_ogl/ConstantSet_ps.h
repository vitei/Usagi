/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_SHADER_CONSTANT
#define _USG_GRAPHICS_PC_SHADER_CONSTANT
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Maths/Matrix4x3.h"
#include "Engine/Maths/Vector4f.h"
#include "Engine/Maths/Vector2f.h"
#include "Engine/Graphics/Effects/EffectVariables.h"
#include "Engine/Graphics/RenderConsts.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class ConstantSet;
class Effect;
class GFXDevice;

class ConstantSet_ps
{
public:
	ConstantSet_ps();
	~ConstantSet_ps();

	void Init(GFXDevice* pDevice, const ConstantSet& owner);
	void CleanUp(GFXDevice* pDevice);
	void UpdateBuffer(GFXDevice* pDevice, bool bDoubleUpdate);
	void Bind(ShaderConstantType eConstType) const;

private:
	void InitOffsetsAndGPUData(const ShaderConstantDecl* pDecl);
	void AppendOffsets(const ShaderConstantDecl* pDecl, memsize uOffset, memsize& uSize, uint32& uVarCount);

	// This may all seem wasteful on the PC, but when you have to switch endian for the benefit
	// of the GPU anyway it becomes necessary
	void WriteMatrix44(const Matrix4x4* pMat, uint32 uCount, uint8* pGPUTarget);
	void WriteMatrix43(const Matrix4x3* pMat, uint32 uCount, uint8* pGPUTarget);
	void WriteVector4(const Vector4f* pVec, uint32 uCount, uint8* pGPUTarget);
	void WriteVector2(const Vector2f* pVec, uint32 uCount, uint8* pGPUTarget);
	void WriteFloat(const float* pFloat, uint32 uCount, uint8* pGPUTarget);
	void WriteBool(const bool* pVal, uint32 uCount, uint8* pGPUTarget);
	void WriteBool4(const Vector4b* pVal, uint32 uCount, uint8* pGPUTarget);
	void WriteInt(const int* pVal, uint32 uCount, uint8* pGPUTarget);
	void WriteInt4(const Vector4i* pVec, uint32 uCount, uint8* pGPUTarget);

	enum
	{
		BUFFER_COUNT = 3
	};

	struct VariableData
	{
		memsize			uOffsetDst;
		memsize			uOffsetSrc;
		ConstantType	eType;
		uint32			uCount;
	};

	const ConstantSet*			m_pOwner;
	bool						m_bDynamic;

	GLuint						m_buffers[BUFFER_COUNT];
	uint32						m_uActiveBuffer;
	void*						m_pGPUData;
	memsize						m_uGPUSize;
	bool						m_bDataValid;

	VariableData*				m_pVarData;
};


inline void ConstantSet_ps::WriteMatrix44(const Matrix4x4* pMat, uint32 uCount, uint8* pGPUTarget)
{
	Matrix4x4* pDst = (Matrix4x4*)pGPUTarget;
	for(uint32 i=0; i<uCount; i++)
	{
		//*pDst = *pMat;
		// FIXME: The transpose isn't actually necessary as GLSL uses column major storage, but now all of our shader
		// multiplications are transposed... should fix this
		pMat->GetTranspose(*pDst);
		pDst++;
		pMat++;
	}
}

inline void ConstantSet_ps::WriteMatrix43(const Matrix4x3* pMat, uint32 uCount, uint8* pGPUTarget)
{
	Matrix4x3* pDst = (Matrix4x3*)pGPUTarget;
	for(uint32 i=0; i<uCount; i++)
	{
		*pDst = *pMat;
		pDst++;
		pMat++;
	}
}

inline void ConstantSet_ps::WriteVector4(const Vector4f* pVec, uint32 uCount, uint8* pGPUTarget)
{
	Vector4f* pDst = (Vector4f*)pGPUTarget;
	for(uint32 i=0; i<uCount; i++)
	{
		*pDst = *pVec;
		pDst++;
		pVec++;
	}
}

inline void ConstantSet_ps::WriteVector2(const Vector2f* pVec, uint32 uCount, uint8* pGPUTarget)
{
	Vector2f* pDst = (Vector2f*)pGPUTarget;
	for(uint32 i=0; i<uCount; i++)
	{
		*pDst = *pVec;
		pDst++;
		pVec++;
	}
}

inline void ConstantSet_ps::WriteFloat(const float* pFloat, uint32 uCount, uint8* pGPUTarget)
{
	float* pDst = (float*)pGPUTarget;
	for(uint32 i=0; i<uCount; i++)
	{
		*pDst = *pFloat;
		pDst++;
		pFloat++;
	}
}


inline void ConstantSet_ps::WriteBool(const bool* pVal, uint32 uCount, uint8* pGPUTarget)
{
	int* pDst = (int*)pGPUTarget;
	for(uint32 i=0; i<uCount; i++)
	{
		*pDst = (*pVal) == true ? 1 : 0;
		pDst++;
		pVal++;
	}
}

inline void ConstantSet_ps::WriteBool4(const Vector4b* pVal, uint32 uCount, uint8* pGPUTarget)
{
	int* pDst = (int*)pGPUTarget;
	for(uint32 i=0; i<uCount; i++)
	{
		pDst[0] = pVal->x == true ? 1 : 0;
		pDst[1] = pVal->y == true ? 1 : 0;
		pDst[2] = pVal->z == true ? 1 : 0;
		pDst[3] = pVal->w == true ? 1 : 0;
		pDst+=4;
		pVal++;
	}
}

inline void ConstantSet_ps::WriteInt(const int* pVal, uint32 uCount, uint8* pGPUTarget)
{
	int* pDst = (int*)pGPUTarget;
	for(uint32 i=0; i<uCount; i++)
	{
		*pDst = *pVal;
		pDst++;
		pVal++;
	}
}

inline void ConstantSet_ps::WriteInt4(const Vector4i* pVec, uint32 uCount, uint8* pGPUTarget)
{
	int* pDst = (int*)pGPUTarget;
	for(uint32 i=0; i<uCount; i++)
	{
		pDst[0] = pVec->x;
		pDst[1] = pVec->y;
		pDst[2] = pVec->z;
		pDst[3] = pVec->w;
		pDst+=4;
		pVec++;
	}
}

}


#endif