/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Handles the setting of lights for a scene context
*****************************************************************************/
#ifndef _USG_GRAPHICS_LIGHT_SCENE_LIGHTING_H_
#define _USG_GRAPHICS_LIGHT_SCENE_LIGHTING_H_
#include "Engine/Common/Common.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Graphics/Lights/DirLight.h"
#include "Engine/Graphics/Lights/PointLight.h"
#include "Engine/Graphics/Lights/SpotLight.h"
#include "Engine/Graphics/Lights/ProjectionLight.h"
#include "Engine/Core/stl/vector.h"

namespace usg{

class GFXContext;
class SceneContext;

class LightingContext
{
public:
	LightingContext();
	~LightingContext();

	void	Init(GFXDevice* pDevice);
	void	Cleanup(GFXDevice* pDevice);
	void	Update(GFXDevice* pDevice, SceneContext* pCtxt);

	// TODO: Enable and disable lights on a per-model basis?
	// void SetLighting(uint64 uLightFlags);

	const List<DirLight>& 	GetActiveDirLights() const; 
	const List<PointLight>&	GetPointLightsInView() const;
	const List<SpotLight>&	GetSpotLightsInView() const;
	const List<ProjectionLight>& GetProjLightsInView() const;
	const vector<TextureHndl>& GetCascadeTextures() const;

	void ClearLists();

	const Color& GetAmbient() { return m_ambient; }
	
	void AddConstantsToDescriptor(DescriptorSet& desc, uint32 uIndex) const;
	void SetPrimaryShadowDesc(GFXContext* pContext);

private:
	//PRIVATIZE_COPY(LightingContext)

	enum
	{
		MAX_LIGHT_LUTS = 6
	};

	ConstantSet			m_lightingConstants;

	Color					m_ambient;

	List<DirLight>			m_visDirLights;
	List<PointLight>		m_visPointLights;
	List<SpotLight>			m_visSpotLights;
	List<ProjectionLight>	m_visProjLights;
	vector<TextureHndl>		m_cascadeTextures;
};

}

#endif
