/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include "Engine/Memory/MemUtil.h"

namespace usg
{

	TGAFile::TGAFile()
	{
		m_uFileSize = 0;
		m_pData = NULL;
		m_bFlipImage = false;
	}
	
	TGAFile::~TGAFile()
	{
		if(m_pData)
		{
			mem::Free(MEMTYPE_STANDARD, m_pData);
		}
	}

	void TGAFile::SetData(void* pData, usg::ColorFormat eFormat, uint32 uWidth, uint32 uHeight)
	{
		MemSet(&m_header, 0, sizeof(m_header));
		m_header.uDataTypeCode = RGB;
		m_header.uWidth = uWidth;
		m_header.uHeight = uHeight;
		m_header.uBitsPerPixel = 24;	
		ASSERT(eFormat == CF_RGBA_8888 || eFormat == CF_RGB_888);
		m_uFileSize = uWidth * uHeight * 3;

		ASSERT(m_pData == NULL);
		m_pData = (uint8*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_DEBUG, m_uFileSize);
		uint8* pSrc = (uint8*)pData;

		uint32 uSrcOffset = eFormat == CF_RGBA_8888 ? 4 : 3;


		uint32 i = 0;
		while (i < m_uFileSize)
		{
			m_pData[i+0] = pSrc[i+2];       //grab blue
			m_pData[i+1] = pSrc[i+1];		//assign red to blue
			m_pData[i+2] = pSrc[i+0];		//assign blue to red

			i += 3;     //skip to next blue byte
		}
	}

	void TGAFile::Save(const char* szFileName)
	{
		File fileOut(szFileName, FILE_ACCESS_WRITE, FILE_TYPE_DEBUG_DATA);
		if(!fileOut.IsOpen())
			return;

		fileOut.Write(sizeof(m_header), &m_header);
		fileOut.Write(m_uFileSize, m_pData);
	}

	bool TGAFile::Load(const char* szFileName)
	{
		ASSERT(m_pData == NULL);
		File fileIn(szFileName, FILE_ACCESS_WRITE, FILE_TYPE_DEBUG_DATA);
		if (!fileIn.IsOpen())
			return false;
		m_uFileSize = fileIn.GetSize() - sizeof(m_header);
		fileIn.Read(sizeof(m_header), &m_header);

		m_pData = (uint8*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_DEBUG, m_uFileSize);
		fileIn.Read(m_uFileSize, m_pData);

		return true;
	}

}