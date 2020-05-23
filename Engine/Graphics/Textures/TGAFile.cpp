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

	void TGAFile::CopyData(const TGAFile* pSrc, const GFXBounds& srcBounds, const GFXBounds& dstBounds)
	{
		// Fill in
	}

	void TGAFile::PrepareImage(const Header& header)
	{
		MemCpy(&m_header, &header, sizeof(header));
		m_uFileSize = header.uWidth * header.uHeight * (header.uBitsPerPixel/8);
		m_pData = (uint8*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_DEBUG, m_uFileSize);
		MemSet(m_pData, 0, m_uFileSize);
	}

	void TGAFile::SetData(void* pData, usg::ColorFormat eFormat, uint32 uWidth, uint32 uHeight)
	{
		MemSet(&m_header, 0, sizeof(m_header));
		m_header.uDataTypeCode = RGB;
		m_header.uWidth = uWidth;
		m_header.uHeight = uHeight;
		uint32 uSrcOffset = eFormat == CF_RGBA_8888 ? 4 : 3;
		if (eFormat == CF_R_8)
		{
			uSrcOffset = 1;
		}
		m_header.uBitsPerPixel = 24;
		ASSERT(eFormat == CF_RGBA_8888 || eFormat == CF_RGB_888 || eFormat == CF_R_8);
		m_uFileSize = (memsize)(uWidth * uHeight * 3);

		ASSERT(m_pData == NULL);
		m_pData = (uint8*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_DEBUG, m_uFileSize);
		uint8* pSrc = (uint8*)pData;


		uint32 i = 0;
		uint32 j = 0;
		while (i < m_uFileSize)
		{
			if(eFormat != CF_R_8)
			{
				m_pData[i+0] = pSrc[j+2];       //grab blue
				m_pData[i+1] = pSrc[j+1];		//assign red to blue
				m_pData[i+2] = pSrc[j+0];		//assign blue to red
			}
			else
			{
				m_pData[i + 0] = pSrc[j];
				m_pData[i + 1] = pSrc[j];
				m_pData[i + 2] = pSrc[j];
			}

			i += 3;     //skip to next blue byte
			j += uSrcOffset;
		}
	}

	void TGAFile::Save(const char* szFileName, FILE_TYPE eFileType)
	{
		File fileOut(szFileName, FILE_ACCESS_WRITE, eFileType);
		if(!fileOut.IsOpen())
			return;

		fileOut.Write(sizeof(m_header), &m_header);
		fileOut.Write(m_uFileSize, m_pData);
	}

	bool TGAFile::Load(const char* szFileName, FILE_TYPE eFileType)
	{
		ASSERT(m_pData == NULL);
		File fileIn(szFileName, FILE_ACCESS_WRITE, eFileType);
		if (!fileIn.IsOpen())
			return false;
		m_uFileSize = fileIn.GetSize() - sizeof(m_header);
		fileIn.Read(sizeof(m_header), &m_header);

		m_pData = (uint8*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_DEBUG, m_uFileSize);
		fileIn.Read(m_uFileSize, m_pData);

		return true;
	}

}