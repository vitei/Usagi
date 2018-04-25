/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once
#ifndef USG_GRAPHICS_PC_OPENGLCONTEXT
#define	USG_GRAPHICS_PC_OPENGLCONTEXT
#include "Engine/Common/Common.h"

namespace usg {

class OpenGLContext
{
public:
	OpenGLContext();
	~OpenGLContext();

	void Init(WindHndl hndl, uint32 uSyncInterval);
	void Destroy();
	void PerformBufferSwap(HDC hdc);
	void SetActive(WindHndl hndl);	// Made need to switch between contexts for multiple windows
	void Deactivate();
private:

	void reset();
	void CreateGLXXContext(const int iMajorVersion, const int iMinorVersion);

	struct PIMPL;
	PIMPL*	m_pImpl;
};

} // namespace usg

#endif // USG_GRAPHICS_PC_OPENGLCONTEXT