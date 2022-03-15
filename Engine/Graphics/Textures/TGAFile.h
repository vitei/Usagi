/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_TGAFILE_H
#define _USG_GRAPHICS_TGAFILE_H

#include "Engine/Graphics/RenderConsts.h"

// TODO: Add loading as a texture?

namespace usg
{

	class TGAFile
	{
	public:
		TGAFile();
		~TGAFile();

		struct Header;

		void PrepareImage(const Header& header);
		void SetData(void* pData, usg::ColorFormat eFormat, uint32 uWidth, uint32 uHeight);
		void CopyData(const TGAFile* pSrc, const GFXBounds& srcBounds, const GFXBounds& dstBounds);

		void Save(const char* szFileName, FILE_TYPE eFileType = FILE_TYPE_DEBUG_DATA);
		bool Load(const char* szFileName, FILE_TYPE eFileType = FILE_TYPE_DEBUG_DATA);

		void SetFlipImage(bool bFlipImage) { m_bFlipImage = bFlipImage; }
		bool FlipVertical();

		memsize GetFileSize() { return m_uFileSize;  }
		enum TGA_TYPE
		{
			NO_IMAGE = 0,
			PALETTE = 1,
			RGB = 2,
			BW = 3,
			RLE_PALETTE = 9,
			RLE_RGB = 10,
			RLE_BW = 11,
		};

		PACK(
			struct Header {
			uint8    uIdLength;
			uint8    uColorMapType;
			uint8    uDataTypeCode;
			uint8	 colorMapSpec[5];
			uint16   uOriginX;
			uint16   uOriginY;
			uint16   uWidth;
			uint16   uHeight;
			uint8    uBitsPerPixel;
			uint8    uImageDescriptor;
		});

		Header& GetHeader() { return m_header; }
		uint8* GetData() { return m_pData; }

	private:

		uint8*	m_pData;
		Header	m_header;
		memsize	m_uFileSize;
		bool	m_bFlipImage;
	};

}

#endif
