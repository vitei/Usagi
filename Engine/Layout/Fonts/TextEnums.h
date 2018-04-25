/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef __usg_graphics_textenums__
#define __usg_graphics_textenums__

namespace usg
{
	enum TextAlign
	{
		kTextAlignLeft   = (1 << 1), /**< Left-aligns multiple lines of text. */
		kTextAlignCenter = (1 << 2), /**< Centers multiple lines of text. */
		kTextAlignRight  = (1 << 3), /**< Right-aligns multiple lines of text. */
		//
		kTextAlignHOriginLeft   = (1 << 4), /**< Places the origin at the left edge of the text. */
		kTextAlignHOriginCenter = (1 << 5), /**< Places the origin at the horizontal center of the text */
		kTextAlignHOriginRight  = (1 << 6), /**< Places the origin at the right edge of the text. */
		//
		kTextAlignVOriginTop      = (1 << 7), /**< Places the origin at the top of the text. */
		kTextAlignVOriginMiddle   = (1 << 8), /**< Places the origin at the vertical center of the text. */
		kTextAlignVOriginBottom   = (1 << 9), /**< Places the origin at the baseline of the first line of text. */
		kTextAlignVOriginBaseline = (1 << 10), /**< Places the origin at the bottom of the text. */

		kDefaultDrawFlag = kTextAlignLeft | kTextAlignHOriginLeft | kTextAlignVOriginTop
	};
}

#endif /* __usg_graphics_textenums__ */
