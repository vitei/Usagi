/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Represents an RGBA colour
*****************************************************************************/
#ifndef _USG_GRAPHICS_COLOR_H_
#define _USG_GRAPHICS_COLOR_H_

#include "Engine/Maths/Vector4f.h"

namespace usg {

class Color
{
public:
	Color(void) { Assign(0.0f, 0.0f, 0.0f, 0.0f); }
	Color(const Vector4f& v4) { Assign(v4); }
	Color(float32 rgba[]);
	Color(float32 r, float32 g, float32 b, float32 a = 1.0f);
	Color(uint8 r, uint8 g, uint8 b, uint8 a = 255);
	~Color(void) {}
	//Getter
	const float* rgba() const { return m_rgba; }
	//Setter
	inline void Assign(float32 r, float32 g, float32 b, float32 a = 1.0f);
	inline void Assign(uint8 r, uint8 g, uint8 b, uint8 a = 255);
	inline void Assign(const Vector4f& v4);
	inline void AssignRGBA32(const uint32 rgba);

	void operator *=(const Color& cScaleColor);

	float& r() { return m_fR; }
	float& g() { return m_fG; }
	float& b() { return m_fB; }
	float& a() { return m_fA; }

	uint8 r8() const;
	uint8 g8() const;
	uint8 b8() const;
	uint8 a8() const;

	inline void r8(uint8 r) {
		m_rgba[0] = ((float)r) / 255.f;
	}
	inline void g8(uint8 g) {
		m_rgba[1] = ((float)g) / 255.f;
	}
	inline void b8(uint8 b) {
		m_rgba[2] = ((float)b) / 255.f;
	}
	inline void a8(uint8 a) {
		m_rgba[3] = ((float)a) / 255.f;
	}

	void FillV4(Vector4f& v4) const;
	void FillU8(uint8 &r, uint8 &g, uint8 &b, uint8 &a) const;

	const float& r() const { return m_fR; }
	const float& g() const { return m_fG; }
	const float& b() const { return m_fB; }
	const float& a() const { return m_fA; }

	void Copy(float* pOut) const;

	Color operator*(float fMul) const;
	Color operator*(const Color& color) const;
	bool operator!=(const Color &color) const;
	bool operator==(const Color &color) const;
	const Color& operator*=(float fMul);
	Color operator+(const Color& rhs) const;

	uint8 GetValueAsU8(uint32 uId) const;
	bool CompareRGB(const Color &color) const;
	union
	{
		float32 m_rgba[4];
		struct
		{
			float32 m_fR;
			float32 m_fG;
			float32 m_fB;
			float32 m_fA;
		};
	};

	static const Color Red;
	static const Color Green;
	static const Color Blue;
	static const Color Black;
	static const Color Grey;
	static const Color Yellow;
	static const Color Purple;
	static const Color White;
};

inline bool Color::operator!=(const Color &color) const
{
	return(	m_fR != color.m_fR ||
			m_fG != color.m_fG ||
			m_fB != color.m_fB ||
			m_fA != color.m_fA );
}

inline bool Color::operator==(const Color &color) const
{
	return(	m_fR == color.m_fR &&
		m_fG == color.m_fG &&
		m_fB == color.m_fB &&
		m_fA == color.m_fA );
}

inline Color::Color(float32 r, float32 g, float32 b, float32 a)
{
	Assign(r, g, b, a);
}

inline Color::Color(uint8 r, uint8 g, uint8 b, uint8 a)
{
	Assign(r, g, b, a);
}

inline void Color::AssignRGBA32(const uint32 rgba)
{
	uint32 a = (rgba&0xFF000000)>>24;
	uint32 b = (rgba&0x00FF0000)>>16;
	uint32 g = (rgba&0x0000FF00)>>8;
	uint32 r = (rgba&0x000000FF);

	Assign((uint8)r, (uint8)g, (uint8)b, (uint8)a);
}

inline Color::Color(float32 rgba[])
{
	Assign(rgba[0], rgba[1], rgba[2], rgba[3]);
}

inline void Color::Assign(const Vector4f& v4)
{
	Assign(v4.x, v4.y, v4.z, v4.w);
}

inline void Color::Assign(float32 r, float32 g, float32 b, float32 a)
{ 
	m_rgba[0] = r;
	m_rgba[1] = g; 
	m_rgba[2] = b;
	m_rgba[3] = a;
}

inline void Color::Assign(uint8 r, uint8 g, uint8 b, uint8 a)
{ 
	m_rgba[0] = ((float)r)/255.f;
	m_rgba[1] = ((float)g)/255.f; 
	m_rgba[2] = ((float)b)/255.f;
	m_rgba[3] = ((float)a)/255.f;
}


inline void Color::Copy(float* pOut) const
{
	pOut[0] = m_rgba[0];
	pOut[1] = m_rgba[1];
	pOut[2] = m_rgba[2];
	pOut[3] = m_rgba[3];
}

inline Color Color::operator*(float fMul) const
{
	Color tmp(m_rgba[0]*fMul, m_rgba[1]*fMul, m_rgba[2]*fMul, m_rgba[3]*fMul);
	return tmp;
}

inline Color Color::operator*(const Color& rhs) const
{
	Color tmp(m_rgba[0]*rhs.m_rgba[0], m_rgba[1]*rhs.m_rgba[1], m_rgba[2]*rhs.m_rgba[2], m_rgba[3]*rhs.m_rgba[3]);
	return tmp;	
}

inline const Color& Color::operator*=(float fMul)
{
	m_rgba[0]*=fMul;
	m_rgba[1]*=fMul;
	m_rgba[2]*=fMul;
	m_rgba[3]*=fMul;

	return *this;
}

inline Color Color::operator+(const Color& rhs) const
{
	Color tmp(m_rgba[0]+rhs.m_rgba[0], m_rgba[1]+rhs.m_rgba[1], m_rgba[2]+rhs.m_rgba[2], m_rgba[3]+rhs.m_rgba[3]);
	return tmp;	
}

inline bool Color::CompareRGB(const Color &color) const
{
	return (color.m_rgba[0] == m_rgba[0] && color.m_rgba[1] == m_rgba[1] && color.m_rgba[2] == m_rgba[2]);
}

inline uint8 Color::GetValueAsU8(uint32 uId) const
{
	float fVal = m_rgba[uId];
	fVal = fVal > 1.0f ? 1.0f : fVal;
	fVal = fVal < 0.0f ? 0.0f : fVal;
	fVal = (fVal*255.f)+0.5f;
	return (uint8)(fVal);
}

inline uint8 Color::r8() const
{
	return GetValueAsU8(0);
}

inline uint8 Color::g8() const
{
	return GetValueAsU8(1);
}

inline uint8 Color::b8() const
{
	return GetValueAsU8(2);
}

inline uint8 Color::a8() const
{
	return GetValueAsU8(3);
}

inline void Color::FillV4(Vector4f& v4) const
{
	v4.x = m_rgba[0];
	v4.y = m_rgba[1];
	v4.z = m_rgba[2];
	v4.w = m_rgba[3];
}

inline void Color::FillU8(uint8 &r, uint8 &g, uint8 &b, uint8 &a) const
{
	r = r8();
	g = g8();
	b = b8();
	a = a8();
}

inline void Color::operator *=(const Color& cScaleColor)
{
	for (int i = 0; i < 4; i++)
	{
		m_rgba[i] *= cScaleColor.m_rgba[i];
	}
}

}

#endif
