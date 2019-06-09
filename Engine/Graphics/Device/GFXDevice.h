/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The graphics object (avoiding a static class as we want to
//	have strit control over where in code this can be accessed)
*****************************************************************************/
#ifndef _USG_GRAPHICS_GFXDEVICE_H_
#define _USG_GRAPHICS_GFXDEVICE_H_

#include "Engine/Graphics/Device/GFXHandles.h"


namespace usg {

class ConstantSet;
class TextureSet;
class GFXContext;
class Display;
class IHeadMountedDisplay;
struct DisplaySettings;
class GFXDevice_ps;
class PipelineStateDecl;
class RenderPassDecl;
class SamplerDecl;
struct DescriptorDeclaration;
class PipelineStateDecl;

class GFXDevice
{
public:
	GFXDevice();
	~GFXDevice();

	void Init();

	void InitAllHardwareDisplays();
	void InitDisplay(WindHndl hndl);
	void SetHeadMountedDisplay(IHeadMountedDisplay* pDisplay);

	Display* GetDisplay(uint32 uIndex);
	IHeadMountedDisplay* GetHMD();
	uint32 GetValidDisplayCount() const { return m_uDisplayCount;  }
	uint32 GetFrameCount() const { return m_uFrameCount; }

	uint32 GetHardwareDisplayCount();
	float GetGPUTime() const;
	const DisplaySettings* GetDisplayInfo(uint32 uIndex);

	void Begin();
	void End();

	PipelineStateHndl		GetPipelineState(const RenderPassHndl& renderPass, const PipelineStateDecl& decl);
	// Returns true if changed, if render pass was equal to the old one or invalid returns false
	bool					ChangePipelineStateRenderPass(const RenderPassHndl& renderPass, PipelineStateHndl& hndlInOut);
	RenderPassHndl			GetRenderPass(const RenderPassDecl& decl);
	SamplerHndl				GetSampler(const SamplerDecl& decl);
	DescriptorSetLayoutHndl GetDescriptorSetLayout(const DescriptorDeclaration* pDecl);

	void GetPipelineDeclaration(const PipelineStateHndl pipeline, PipelineStateDecl& out, RenderPassHndl& passOut);

	void PostUpdate();
	bool Is3DEnabled() const;
	void WaitIdle();


	void FinishedStaticLoad();
	void ClearDynamicResources();
	GFXDevice_ps&	GetPlatform();
	GFXContext*		GetImmediateCtxt();
	GFXContext*		CreateDeferredContext(uint32 uSizeMul);

private:
	enum 
	{
		MAX_DISPLAY_COUNT = 6
	};
	PRIVATIZE_COPY(GFXDevice);

	struct PIMPL;
	PIMPL*					m_pImpl;
		
	Display*				m_pDisplays[MAX_DISPLAY_COUNT];
	IHeadMountedDisplay*	m_pHMD;
	uint32					m_uDisplayCount;
	uint32					m_uFrameCount;
};

inline Display* GFXDevice::GetDisplay(uint32 uIndex)
{
	if (uIndex < m_uDisplayCount)
	{
		return m_pDisplays[uIndex];
	}
	return nullptr;
}


}

#endif
