/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma  once

#ifndef EXCEPTION_H
#define EXCEPTION_H

namespace usg
{
	class GFXDevice;
	namespace Exception
	{
		void Init(usg::GFXDevice* pDevice, const char* szBuildId);
		void DumpBacktrace();
	}
	
} // namespace usg

#endif // EXCEPTION_H