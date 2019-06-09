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

		void SetData(void* pData, usg::ColorFormat eFormat, uint32 uWidth, uint32 uHeight);

		void Save(const char* szFileName);
		bool Load(const char* szFileName);

		void SetFlipImage(bool bFlipImage) { m_bFlipImage = bFlipImage; }

		memsize GetFileSize() { return m_uFileSize;  }
		enum TGA_TYPE
		{
			NO_IMAGE = 0x0000,
			PALETTE = 0x0001,
			RGB = 0x0002,
			BW = 0x0003,
			RLE_PALETTE = 0x1001,
			RLE_RGB = 0x1002,
			CMP_BW = 0x1003,
			CMP_PALETTE = 0x100000,
			CMP_4PASS = 0x100001
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
