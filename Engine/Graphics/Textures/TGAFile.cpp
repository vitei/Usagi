/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include "Engine/Memory/MemUtil.h"

namespace usg
{

#define BOTTOM_LEFT  0x00	// first pixel is bottom left corner
#define BOTTOM_RIGHT 0x10	// first pixel is bottom right corner
#define TOP_LEFT     0x20	// first pixel is top left corner
#define TOP_RIGHT    0x30	// first pixel is top right corner

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

	struct rgba_t
	{
		uint8 r;
		uint8 g;
		uint8 b;
		uint8 a;
	};

	struct rgb_t
	{
		uint8 r;
		uint8 g;
		uint8 b;
	};

	void TGAFile::CopyData(const TGAFile* pSrc, const GFXBounds& srcBounds, const GFXBounds& dstBounds)
	{
		// Fill in
		sint32 iLineLength = m_header.uWidth * (m_header.uBitsPerPixel / 8);
		sint32 iLineLengthSrc = pSrc->m_header.uWidth * (pSrc->m_header.uBitsPerPixel / 8);
		sint32 iXOffset = dstBounds.x * (m_header.uBitsPerPixel / 8);
		sint32 iXOffsetSrc = srcBounds.x * (m_header.uBitsPerPixel / 8);

		for (sint32 i = 0; i < dstBounds.height; i++)
		{
			sint32 srcOffset = iXOffsetSrc + (iLineLengthSrc * (i + srcBounds.y));
			sint32 dstOffset = iXOffset + (iLineLength * (i + dstBounds.y));
			sint32 copyLength = srcBounds.width * (m_header.uBitsPerPixel / 8);

			ASSERT(srcOffset < pSrc->m_uFileSize);
			ASSERT(dstOffset < m_uFileSize);

			uint8* pDstData = m_pData + dstOffset;
			uint8* pSrcData = pSrc->m_pData + srcOffset;

			memcpy(pDstData, pSrcData, copyLength);
		}
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
		uint32 uSrcOffset = eFormat == ColorFormat::RGBA_8888 ? 4 : 3;
		uint32 uBpp = eFormat == ColorFormat::RGBA_8888 ? 4 : 3;
		if (eFormat == ColorFormat::R_8)
		{
			uSrcOffset = 1;
		}
		m_header.uBitsPerPixel = eFormat == ColorFormat::RGBA_8888 ? 32 : 24;
		ASSERT(eFormat == ColorFormat::RGBA_8888 || eFormat == ColorFormat::RGB_888 || eFormat == ColorFormat::R_8);
		m_uFileSize = (memsize)(uWidth * uHeight * uBpp);

		ASSERT(m_pData == NULL);
		m_pData = (uint8*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_DEBUG, m_uFileSize);
		uint8* pSrc = (uint8*)pData;


		uint32 i = 0;
		uint32 j = 0;
		while (i < m_uFileSize)
		{
			if(eFormat != ColorFormat::R_8)
			{
				m_pData[i+0] = pSrc[j+2];       //grab blue
				m_pData[i+1] = pSrc[j+1];		//assign red to blue
				m_pData[i+2] = pSrc[j+0];		//assign blue to red
				if (uBpp > 3)
				{
					m_pData[i+3] = pSrc[j + 3];
				}
			}
			else
			{
				m_pData[i + 0] = pSrc[j];
				m_pData[i + 1] = pSrc[j];
				m_pData[i + 2] = pSrc[j];
			}

			i += uBpp;     //skip to next blue byte
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
		usg::File file(szFileName, usg::FILE_ACCESS_READ);

		if (!file.IsOpen())
			return false;

		// read the TGA header
		file.Read(sizeof(Header), &m_header);

		// see if the image type is one that we support (RGB, RGB RLE, GRAYSCALE, GRAYSCALE RLE)
		if (((m_header.uDataTypeCode != RGB) && (m_header.uDataTypeCode != BW) &&
			(m_header.uDataTypeCode != RLE_RGB) && (m_header.uDataTypeCode != RLE_BW)) ||
			m_header.uColorMapType != 0)
		{
			return false;
		}


		int colorMode = m_header.uBitsPerPixel / 8;

		// we don't handle less than 24 bit
		if (colorMode < 3)
		{
			return false;
		}

		m_uFileSize = m_header.uWidth * m_header.uHeight * colorMode;

		// allocate memory for TGA image data
		m_pData = (uint8*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_DEBUG, m_uFileSize);

		// skip past the id if there is one
		if (m_header.uIdLength > 0)
			file.AdvanceBytes(m_header.uIdLength);

		// read image data
		if (m_header.uDataTypeCode == RGB || m_header.uDataTypeCode == BW)
		{
			file.Read(m_uFileSize, m_pData);
		}
		else
		{
			// this is an RLE compressed image
			uint8 id;
			uint8 length;
			rgba_t color = { 0, 0, 0, 0 };
			uint32 i = 0;

			while (i < m_uFileSize)
			{
				id = file.ReadByte();

				// see if this is run length data
				if (id >= 128)// & 0x80)
				{
					// find the run length
					length = (uint8)(id - 127);

					// next 3 (or 4) bytes are the repeated values
					color.b = file.ReadByte();
					color.g = file.ReadByte();
					color.r = file.ReadByte();

					if (colorMode == 4)
						color.a = file.ReadByte();

					// save everything in this run
					while (length > 0)
					{
						m_pData[i++] = color.b;
						m_pData[i++] = color.g;
						m_pData[i++] = color.r;

						if (colorMode == 4)
							m_pData[i++] = color.a;

						--length;
					}
				}
				else
				{
					// Get the RLE
					length = (uint8)(id + 1);

					while (length > 0)
					{
						color.b = file.ReadByte();
						color.g = file.ReadByte();
						color.r = file.ReadByte();

						if (colorMode == 4)
							color.a = file.ReadByte();

						m_pData[i++] = color.b;
						m_pData[i++] = color.g;
						m_pData[i++] = color.r;

						if (colorMode == 4)
							m_pData[i++] = color.a;

						--length;
					}
				}
			}
		}

		// Update the header to reflect out new format
		if (m_header.uDataTypeCode == RLE_RGB)
		{
			m_header.uDataTypeCode = RGB;
		}
		if (m_header.uDataTypeCode == RLE_BW)
		{
			m_header.uDataTypeCode = BW;
		}

		if ((m_header.uImageDescriptor & TOP_LEFT) != TOP_LEFT)
			FlipVertical();

		file.Close();

		return true;
	}

	bool TGAFile::FlipVertical()
	{
		if (!m_pData)
			return false;

		if (m_header.uBitsPerPixel == 32)
		{

			rgba_t* tmpBits;
			usg::ScratchObj<rgba_t> scratchObj(tmpBits, m_header.uWidth);

			if (!tmpBits)
				return false;

			int lineWidth = m_header.uWidth * 4;

			rgba_t* top = (rgba_t*)m_pData;
			rgba_t* bottom = (rgba_t*)(m_pData + lineWidth * (m_header.uHeight - 1));

			for (int i = 0; i < (m_header.uHeight / 2); ++i)
			{
				memcpy(tmpBits, top, lineWidth);
				memcpy(top, bottom, lineWidth);
				memcpy(bottom, tmpBits, lineWidth);

				top = (rgba_t*)((uint8*)top + lineWidth);
				bottom = (rgba_t*)((uint8*)bottom - lineWidth);
			}

			tmpBits = 0;
		}
		else if (m_header.uBitsPerPixel == 24)
		{
			rgb_t* tmpBits;
			usg::ScratchObj<rgb_t> scratchObj(tmpBits, m_header.uWidth);

			if (!tmpBits)
				return false;

			int lineWidth = m_header.uWidth * 3;

			rgb_t* top = (rgb_t*)m_pData;
			rgb_t* bottom = (rgb_t*)(m_pData + lineWidth * (m_header.uHeight - 1));

			for (int i = 0; i < (m_header.uHeight / 2); ++i)
			{
				memcpy(tmpBits, top, lineWidth);
				memcpy(top, bottom, lineWidth);
				memcpy(bottom, tmpBits, lineWidth);

				top = (rgb_t*)((uint8*)top + lineWidth);
				bottom = (rgb_t*)((uint8*)bottom - lineWidth);
			}

			tmpBits = 0;
		}


		if ((m_header.uImageDescriptor & TOP_RIGHT) == TOP_RIGHT)
		{
			m_header.uImageDescriptor &= ~0x40;
			m_header.uImageDescriptor |= BOTTOM_RIGHT;
		}
		if ((m_header.uImageDescriptor & TOP_LEFT) == TOP_LEFT)
		{
			m_header.uImageDescriptor &= ~0x40;
			m_header.uImageDescriptor |= BOTTOM_LEFT;
		}
		if ((m_header.uImageDescriptor & BOTTOM_RIGHT) == BOTTOM_RIGHT)
		{
			m_header.uImageDescriptor &= ~0x40;
			m_header.uImageDescriptor |= TOP_RIGHT;
		}
		if ((m_header.uImageDescriptor & BOTTOM_LEFT) == BOTTOM_LEFT)
		{
			m_header.uImageDescriptor &= ~0x40;
			m_header.uImageDescriptor |= TOP_LEFT;
		}

		return true;
	}

}