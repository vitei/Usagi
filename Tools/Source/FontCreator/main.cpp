#include "Engine/Common/Common.h"
#include "msdfgen.h"
#include "msdfgen-ext.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include "gli/gli.hpp"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Layout/Fonts/TextStructs.pb.h"
#include <yaml-cpp/yaml.h>
#include <pb.h>

using namespace msdfgen;

struct FontDefinition
{
	std::string sourceFont;
	std::string fontName;
	std::string letterList;
	uint32		size;
	double		spacing;
	double		scale;
	bool		fixedWidth;
	bool		bold;
	bool		underscore;
};

struct TexInfo
{
	uint8* pData;
	uint32 uWidth;
	uint32 uHeight;
	uint32 uBpp;
	uint32 uCharPadding;

	uint32 uCurrentX;
	uint32 uCurrentY;
};

struct Bounds
{
	double l, b, r, t;
};

bool GetFontInfo(const char* path, FontDefinition& defOut)
{
	YAML::Node mainNode = YAML::LoadFile(path);

	defOut.sourceFont = mainNode["SourceFont"].as<std::string>();

	defOut.fontName = mainNode["FontName"].as<std::string>();
	defOut.letterList = mainNode["LetterList"].as<std::string>();
	defOut.size = mainNode["Size"].as<uint32>();
	defOut.spacing = mainNode["Spacing"].as <double> ();
	defOut.bold = mainNode["Bold"].as<bool>();
	defOut.underscore = mainNode["Underscore"].as<bool>();
	defOut.fixedWidth = mainNode["FixedWidth"].as<bool>();

	return true;
}

// Returns the offset
void EncodeCharacter(TexInfo& texDef, const Bitmap<FloatRGB> &bitmap, const Bitmap<float> &sdfBitmap, usg::text::FontDefinition& fontDef)
{
	uint32 uLineBytes = texDef.uWidth * texDef.uBpp;
	uint32 uXOffset = texDef.uCurrentX * texDef.uBpp;
	uint32 uLine = texDef.uCurrentY;
	uint32 uPadding = texDef.uCharPadding;;
	uint32 uCharacterWidth = bitmap.width() + uPadding;
	uint32 uCharacterHeight = bitmap.width() + uPadding;
	for (int y = bitmap.height() - 1; y >= 0; --y)
	{
		uint8* pDst = texDef.pData + (uLineBytes * ((texDef.uHeight-1) - uLine)) + uXOffset;
		for (int x = 0; x < bitmap.width(); ++x)
		{
			*pDst++ = clamp(int(bitmap(x, y).b * 0x100), 0xff);
			*pDst++ = clamp(int(bitmap(x, y).g * 0x100), 0xff);
			*pDst++ = clamp(int(bitmap(x, y).r * 0x100), 0xff);
			*pDst++ = clamp(int(sdfBitmap(x, y) * 0x100), 0xff);
		}
		uLine++;
	}

	if ((uCharacterWidth * 2 + texDef.uCurrentX) > texDef.uWidth)
	{
		texDef.uCurrentX = 0;
		texDef.uCurrentY += uCharacterHeight;
		ASSERT(texDef.uCurrentY + uCharacterHeight <= texDef.uHeight);
	}
	else
	{
		texDef.uCurrentX += uCharacterWidth;
	}
}

void WriteKTX(TexInfo& info, const char* szFileName)
{
	gli::texture2d Texture(gli::FORMAT_BGRA8_UNORM_PACK8, gli::texture2d::extent_type(info.uWidth, info.uHeight), 1);
	glm::u8vec4 * LinearAddress0 = Texture[0].data<glm::u8vec4>();

	usg::MemCpy(LinearAddress0, info.pData, info.uBpp*info.uWidth*info.uHeight);
	Texture = gli::flip(Texture);
	bool bResult = gli::save_ktx(Texture, szFileName);
	ASSERT(bResult);
}

void WriteTga(TexInfo& info, const char* szFileName)
{
	usg::TGAFile::Header header;
	uint32 uFileSize = 0;
	memset(&header, 0, sizeof(header));
	header.uDataTypeCode = usg::TGAFile::RGB;
	header.uWidth = info.uWidth;
	header.uHeight = info.uHeight;
	header.uBitsPerPixel = 32;
	uFileSize = header.uWidth * header.uHeight * 4;

	uint8* pSrc = (uint8*)info.pData;


	FILE* pFile;
	fopen_s(&pFile, szFileName, "wb");
	fwrite(&header, 1, sizeof(header), pFile);
	fwrite(pSrc, 1, uFileSize, pFile);
}

void EncodeUVs(const TexInfo& info, const Bounds& bounds, usg::text::CharDefinition& chrDefOut)
{
	usg::Vector2f vTL((float)bounds.l, (float)bounds.t);
	usg::Vector2f vBR((float)bounds.r, (float)bounds.b);
	usg::Vector2f vDim((float)info.uWidth, (float)info.uHeight);
	usg::Vector2f vCelLoc((float)info.uCurrentX, (float)info.uCurrentY);
	vTL += vCelLoc;
	vBR += vCelLoc;

	// Padding
	vTL.x -= 0.25f;
	vBR.x += 0.25f;

	vTL = vTL / vDim;
	vBR = vBR / vDim;


	chrDefOut.UV_TopLeft = usg::Vector2f(vTL.x, 1.0f-vTL.y);
	chrDefOut.UV_BottomRight = usg::Vector2f(vBR.x, 1.0f - vBR.y);
}


void InitTexInfo(const FontDefinition& fontDefIn, TexInfo& texInfoOut)
{
	texInfoOut.uCharPadding = 2;
	texInfoOut.uWidth = 128;
	texInfoOut.uHeight = 128;
	uint32 uCharWidth = fontDefIn.size + texInfoOut.uCharPadding;
	uint32 uCharHeight = fontDefIn.size + texInfoOut.uCharPadding;
	
	uint16 uTotalChars = 0;
	// Get the number of UTF-8 encoded characters
	for (uint32 i = 0; i < fontDefIn.letterList.size(); ++i)
	{
		if ((fontDefIn.letterList.c_str()[i] & 0x80) == 0 || (fontDefIn.letterList.c_str()[i] & 0xc0) == 0xc0)
			++uTotalChars;
	}

	// Get the smallest texture they can fit in
	bool bCanFit = false;
	bool bIncWidth = true;
	while (!bCanFit)
	{
		uint32 uCharsPerLine = texInfoOut.uWidth / uCharWidth;
		uint32 uLines = texInfoOut.uHeight / uCharHeight;
		uint32 uCharCount = uCharsPerLine * uLines;
		if (uCharCount > uTotalChars)
		{
			bCanFit = true;
		}
		else
		{
			if (bIncWidth)
			{
				texInfoOut.uWidth *= 2;
			}
			else
			{
				texInfoOut.uHeight *= 2;
			}
			bIncWidth = !bIncWidth;
		}
	}
	texInfoOut.uBpp = 4;
	texInfoOut.uCurrentX = 0;
	texInfoOut.uCurrentY = 0;
}

#define LARGE_VALUE 1e240

int ConvertUTF8ToUnicode(const char* utf8)
{
	wchar_t uni;
	int cc = 0;
	// get length (cc) of the new widechar excluding the \0 terminator first
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &uni, 1);
	return (int)uni;
}


void AutoDetermineScale(const Vector2& vTrans, FontDefinition& fontDef, FontHandle *font)
{
	float fMinScale = FLT_MAX;
	const char* szLetter = fontDef.letterList.c_str();
	Shape shape;
	// Always leave a pixel at the side
	float maxRight = (float)fontDef.size - (float)vTrans.x;
	float maxTop = (float)fontDef.size - (float)vTrans.y;

	while (*szLetter != '\0')
	{
		uint32 uByteCount = usg::U8Char::GetByteCount(szLetter);
		usg::U8Char thisChar(szLetter, uByteCount);
		uint32 uniChar = *szLetter;
		if (uByteCount > 1)
		{
			uniChar = ConvertUTF8ToUnicode(szLetter);
		}

		if (loadGlyph(shape, font, uniChar))
		{
			shape.normalize();
			Bounds bounds = {
				LARGE_VALUE, LARGE_VALUE, -LARGE_VALUE, -LARGE_VALUE
			};

			if (szLetter[0] != ' ' || uByteCount != 1)
			{
				shape.bounds(bounds.l, bounds.b, bounds.r, bounds.t);
				float fWidthScale = maxRight / (float)bounds.r;
				float fHeightScale  = bounds.t > 0.0f ? maxTop / (float)bounds.t : FLT_MAX;
				fMinScale = usg::Math::Min(fWidthScale, fMinScale);
				fMinScale = usg::Math::Min(fHeightScale, fMinScale);
		
			}
		}
		szLetter += uByteCount;
	}
	fontDef.scale = fMinScale;
}

int main(int argc, char *argv[])
{
	usg::mem::InitialiseDefault();
	usg::mem::setConventionalMemManagement(true);

	FreetypeHandle *ft = initializeFreetype();
	FontDefinition fontDef;
	TexInfo texDef;


	std::string outputstub;
	std::string inputFile; 
	std::string outTga;
	std::string outKTX;
	std::string outVPB;

	if (argc != 3)
	{
		printf("Format should be FontCreator <input.yml> <output_dir>");
		return -1;
	}

	inputFile = argv[1];
	outputstub = argv[2];
	outTga = outputstub + ".tga";
	outKTX = outputstub + ".ktx";
	outVPB = outputstub + ".vpb";

	printf("Converting %s", inputFile.c_str());

	
	GetFontInfo(inputFile.c_str(), fontDef);
	InitTexInfo(fontDef, texDef);

	texDef.pData = vnew(usg::ALLOC_LOADING) uint8[texDef.uWidth*texDef.uHeight*texDef.uBpp];
	memset(texDef.pData, 0, texDef.uWidth*texDef.uHeight*texDef.uBpp);


	if (ft) {
		FontHandle *font = loadFont(ft, fontDef.sourceFont.c_str());
		//           image width, height
		Bitmap<FloatRGB> msdf(fontDef.size, fontDef.size);
		Bitmap<float> sdf(fontDef.size, fontDef.size);
		usg::text::FontDefinition protoDef;

		usg::text::CharDefinition* chrInfo = vnew(usg::ALLOC_LOADING) usg::text::CharDefinition[fontDef.letterList.length()];
		
		strcpy_s(protoDef.FontName, fontDef.fontName.c_str());
		if (font) {
			const double pxRange = 2.0;
			Shape shape;
			const char* szLetter = fontDef.letterList.c_str();
			Vector2 trans(pxRange, fontDef.size / 4.0);
			AutoDetermineScale(trans, fontDef, font);
			uint32 uChrCount = 0;
			double maxBoundT = 0.0;
			float maxWidth = 0.0;
			// TODO: Handle non ASCI characters
			while (*szLetter != '\0')
			{
				uint32 uByteCount = usg::U8Char::GetByteCount(szLetter);
				usg::U8Char thisChar(szLetter, uByteCount);
				uint32 uniChar = *szLetter;
				if (uByteCount > 1)
				{
					uniChar = ConvertUTF8ToUnicode(szLetter);
				}

				if (loadGlyph(shape, font, uniChar))
				{
					shape.normalize();
						Bounds bounds = {
							LARGE_VALUE, LARGE_VALUE, -LARGE_VALUE, -LARGE_VALUE
						};
			
					trans.x = fontDef.size / 8.0;
					if (szLetter[0] != ' ' || uByteCount!=1)
					{
						shape.bounds(bounds.l, bounds.b, bounds.r, bounds.t);
						bounds.l *= fontDef.scale;
						bounds.r *= fontDef.scale;
						bounds.t *= fontDef.scale;
						bounds.b *= fontDef.scale;

						maxBoundT = usg::Math::Max(maxBoundT, bounds.t);

						if ((bounds.r + trans.x * 2) > fontDef.size)
						{
							// Just center the image
							trans.x = fontDef.size - (bounds.r - bounds.l);
							trans.x /= 2.f;
							trans.x -= bounds.l;
						}
						bounds.l += trans.x;
						bounds.r += trans.x;
						// Forcing the full character size to come through for the moment
						bounds.t = fontDef.size; // 8.0;
						bounds.b = 0.0;// 8.0;
					}
					else
					{
						// Space doesn't give the correct info for shape bounds
						bounds.l = fontDef.size / 3.0;
						bounds.b = 0.0f;
						bounds.t = fontDef.size;
						bounds.r = fontDef.size - bounds.l;
					}
					if (bounds.r > fontDef.size)
					{
						printf("Not enough room for font");
						ASSERT(false);
					}

					// max. angle
					edgeColoringSimple(shape, 3.0);
					// range, scale, translation
					generateMSDF(msdf, shape, 4.0, fontDef.scale, trans/fontDef.scale, pxRange);
					generateSDF(sdf, shape, 4.0, fontDef.scale, trans / fontDef.scale);
					EncodeUVs(texDef, bounds, chrInfo[uChrCount]);
					EncodeCharacter(texDef, msdf, sdf, protoDef);

					if( szLetter[0] != ' ' )
					{
						maxWidth = usg::Math::Max(maxWidth, chrInfo[uChrCount].UV_BottomRight.x - chrInfo[uChrCount].UV_TopLeft.x);
					}

					chrInfo[uChrCount].CharData = thisChar.GetAsUInt32();
					uChrCount++;
				}
				szLetter += uByteCount;
			}

			if (fontDef.fixedWidth)
			{
				for (uint32 i = 0; i < uChrCount; i++)
				{
					// Force all characters to the same bounds
					float fChrWidth = chrInfo[i].UV_BottomRight.x - chrInfo[i].UV_TopLeft.x;
					fChrWidth = maxWidth - fChrWidth;
					if (fChrWidth > 0.0f)
					{
						chrInfo[i].UV_BottomRight.x += fChrWidth/2.0f;
						chrInfo[i].UV_TopLeft.x -= fChrWidth / 2.0f;
					}
				}
			}
			protoDef.Chars.m_decoderDelegate.data.array = chrInfo;
			protoDef.Chars.m_decoderDelegate.data.count = uChrCount;
			protoDef.LowerOffset = ((float)trans.y - 0.5f)/(float)fontDef.size;
			protoDef.DrawScale = (float)fontDef.size / (float)maxBoundT;
			protoDef.Spacing = (float)fontDef.spacing;

			usg::ProtocolBufferFile fileOut(outVPB.c_str(), usg::FILE_ACCESS_WRITE, usg::FILE_TYPE_RESOURCE);
			fileOut.Write(&protoDef);

			//WriteTga(texDef, outTga.c_str());
			WriteKTX(texDef, outKTX.c_str());
			destroyFont(font);

		}
		deinitializeFreetype(ft);
	}

	vdelete[] texDef.pData;
	texDef.pData = nullptr;

	return 0;
}