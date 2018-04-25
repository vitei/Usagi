/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_OPENGLCONTEXT
#define	_USG_GRAPHICS_PC_OPENGLCONTEXT

class OpenGLContext
{
public:
	OpenGLContext();
	~OpenGLContext();

	void Init(const class Window* pWindow);
	void Destroy();
	void PerformBufferSwap(const Window* pWindow);
	void SetActive(const Window* pWindow);	// Made need to switch between contexts for multiple windows
	void Deactivate();
private:

	void reset();

	struct PIMPL;
	PIMPL*	m_pImpl;
	
	double m_lastSwapTime;
};


#endif