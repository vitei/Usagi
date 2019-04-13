#ifndef _USG_PARTICLE_EDITOR_COLOR_SELECTION_H_
#define _USG_PARTICLE_EDITOR_COLOR_SELECTION_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/StateEnums.pb.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Viewports/Viewport.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/GuiItems.h"
#include "EmitterModifier.h"
#include "FloatAnim.h"

namespace usg
{
	class IMGuiRenderer;
}

class ColorSelection
{
public:
	ColorSelection();
	~ColorSelection();

	void Init(usg::GFXDevice* pDevice, usg::Scene& scene);
	void CleanUp(usg::GFXDevice* pDevice);
	void SetPosition(float fX, float fY);
	void SetSize(float fX, float fY);
	void Update(usg::GFXDevice* pDevice, float fElapsed);
	bool Draw(usg::GFXContext* pContext, usg::PostFXSys* pPostFXSys);
	void SetColor(usg::GFXDevice* pDevice, const usg::Color& color);
	void SaveColor(const usg::Color& color);

	const usg::Color GetColor() const { return m_color; }
	
private:

	enum CURSOR_TYPE
	{
		CURSOR_HUE = 0,
		CURSOR_SATURATION,
		CURSOR_POINTER,
		CURSOR_COUNT
	};

	float GetHue(const usg::Color& color) const;
	usg::Color GetRGBBaseFromHue(float fHue) const;
	void SetPreviewColor(const usg::Color& color);
	void SetHueColor(const usg::Color& color);
	void UpdateCursorVerts(usg::GFXDevice* pDevice, CURSOR_TYPE eType);

	enum 
	{
		HUE_RGB_STACKS = 7,
		SV_INDEX = 0,
		PREVIEW_INDEX = 4,
		PREVIOUS_INDEX = 8,
		HISTORY_INDEX = 12,
		HISTORY_COUNT = 10,
		VERTEX_COUNT = (HISTORY_INDEX + (HISTORY_COUNT*4)),
		INDEX_COUNT = (VERTEX_COUNT/4)*6
	};


	// AAAAAARGH It's bloating into a horrifying monster
	struct Image
	{
		usg::Material		material;
		usg::VertexBuffer	vertices;
		usg::PositionUVColVertex	verts[4];
		usg::Vector2f		vSize;
		usg::Vector2f		vPosition;
	};
	
	usg::SamplerHndl		m_sampler;
	Image				m_cursors[CURSOR_COUNT];
	usg::Color			m_hueColors[HUE_RGB_STACKS];
	usg::PositionDiffuseVertex	m_hueSatCPUVerts[VERTEX_COUNT];
	usg::DescriptorSet	m_globalDescriptor;
	usg::VertexBuffer	m_hueSatVertices;
	usg::VertexBuffer	m_rgbVertices;
	usg::IndexBuffer	m_indices;
	usg::IndexBuffer	m_hueSatIndices;
	usg::IndexBuffer	m_imageIndices;
	usg::Material		m_material;
	usg::ConstantSet	m_constants;
	usg::Color			m_color;
	usg::Color			m_prevColor;
	usg::Color			m_saturatedColor;
	usg::Color			m_historyColors[HISTORY_COUNT];
	usg::Viewport		m_viewport;
	bool				m_bVertsDirty;

};


#endif
