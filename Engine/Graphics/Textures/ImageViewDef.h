/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_IMAGE_VIEW_DEF_H
#define _USG_GRAPHICS_IMAGE_VIEW_DEF_H


namespace usg {


class ImageViewDef
{
public:

	uint32 uBaseMip = 0;
	// Negative value means all mips
	uint32 uMipCount = USG_INVALID_ID;
	uint32 uBaseLayer = 0;
	uint32 uLayerCount = USG_INVALID_ID;
	bool bStencilRead = false;

	bool IsDefault() const
	{
		return *this == Default();
	}

	bool operator==(const ImageViewDef& rhs) const
	{
		return (uBaseMip == rhs.uBaseMip
				&& uMipCount == rhs.uMipCount
				&& uBaseLayer == rhs.uBaseLayer
				&& uLayerCount == rhs.uLayerCount
				&& bStencilRead == rhs.bStencilRead);
	}

	static ImageViewDef Default() { return ImageViewDef(); }

};


}

#endif
