/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A the interface for a headmounted display
*****************************************************************************/
#pragma once
#include "Engine/Common/Common.h"
#include "Engine/Core/Modules/ModuleInterfaces.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Maths/Quaternionf.h"
#include "Engine/Maths/Vector4f.h"

namespace usg
{
	class GFXDevice;
	class GFXContext;
	class RenderTarget;
	class Display;

	class IHeadMountedDisplay : public ModuleInterface
	{
	public:
		enum class Eye
		{
			Left,
			Right,
			Count
		};
		
		enum class ExtensionType
		{
			Instance,
			Device,
			Count
		};

		IHeadMountedDisplay() {}
		virtual ~IHeadMountedDisplay() {}

		static uint32 GetModuleTypeNameStatic() { return 'HMD'; }
		virtual const uint32 GetModuleTypeName() const override { return GetModuleTypeNameStatic(); }


	    virtual bool Init(GFXDevice* pDevice) = 0;
	    virtual void Cleanup(GFXDevice* pDevice) = 0;

		virtual void Update() = 0;
		
		virtual void ResetTracking(bool bPos, bool bOri) = 0;

		virtual void GetRenderTargetDim(Eye eye, float pixelDensity, uint32 &uWidthOut, uint32 &uHeightOut) const = 0;

		virtual Matrix4x4 GetProjectionMatrix(Eye eye, float fNear, float fFar) const = 0;

		virtual void Transfer(GFXContext* pContext, Eye eye, RenderTarget* pTarget) = 0;
		virtual void TransferSpectatorDisplay(GFXContext* pContext, Display* pDisplay) = 0;

		virtual void SubmitFrame() = 0;
		virtual char16* GetAudioDeviceName() = 0;

		virtual void GetHMDTransform(usg::Matrix4x4& matOut) const = 0;
		virtual void GetEyeTransform(Eye eye, usg::Matrix4x4& mMatOut) const = 0;

		virtual const uint32 GetRequiredAPIExtensionCount(ExtensionType extType) const { return 0; }
		virtual const char* GetRequiredAPIExtension(ExtensionType extType, uint32 uIndex) const { return nullptr; }

	private:
		
	
	};


}
