/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: simple cube renderer
 *****************************************************************************/
#ifndef _USG_CUBERENDER_H_
#define _USG_CUBERENDER_H_

#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Scene/Common/Mesh.h"
#include "Engine/Graphics/Materials/Material.h"

namespace usg
{

class Scene;

class CubeRender
{
public:

	static const VertexElement VertexElements[];

	CubeRender(void);
	~CubeRender(void);
    
	bool    Init(GFXDevice* pDevice, Scene* pScene, ResourceMgr* pResMgr, uint32 uMaxCubes, bool bHideInside=false);
	void	Remove();
	void	FreeAllocation();
	void	Create(GFXDevice* pDevice, bool bHideInside = false);
    
    void    Clear();
    void    AddCube(const Matrix4x4& mat, const Color& clr);
    void    Flush(GFXDevice* pDevice);
   

    struct Cube
    {
        Matrix4x3  mat;
        float r, g, b, a;
    };

    
private:
   
	Mesh                    m_mesh;
	RenderGroup*			m_pRenderGroup;
	Scene*					m_pScene;
    
	Cube*                   m_pVerts;
    uint32                  m_uNumCubes;
    uint32                  m_uMaxCubes;
    
    
};

}	//	namespace usg

#endif
