/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: PakFile for multiple files merged into one
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/stl/vector.h"
#include "ResourceDecl.h"
#include "PakFile.h"
#include "PakDecl.h"
#include "FileDependencies.h"

namespace usg
{
	FileDependencies::FileDependencies()
	{

	}

	FileDependencies::~FileDependencies()
	{

	}

	void FileDependencies::Init(class PakFile* pCurrentFile, const PakFileDecl::Dependency* pDependencies, uint32 uDependencyCount)
	{
		m_dependencies.resize(uDependencyCount);
		for (uint32 i = 0; i < uDependencyCount; i++)
		{
			FileDependency& dep = m_dependencies[i];
			if (pDependencies[i].PakIndex != USG_INVALID_ID)
			{
				dep.resHandle = pCurrentFile->GetResource(pDependencies[i].FileCRC);
			}
			else
			{
				// TODO: Dependencies from other files
				ASSERT(false);
			}
			dep.uFileCRC = pDependencies[i].FileCRC;
			dep.uUsageCRC = pDependencies[i].UsageCRC;
		}
	}
	
	BaseResHandle FileDependencies::GetDependencyByCRC(uint32 uFileCRC) const
	{
		for (auto& dep : m_dependencies)
		{
			if (dep.uFileCRC == uFileCRC)
			{
				return dep.resHandle;
			}
		}
		return BaseResHandle(nullptr);
	}
	
	BaseResHandle FileDependencies::GetDependencyByIndex(uint32 uFileIndex) const
	{
		return m_dependencies[uFileIndex].resHandle;
	}

	BaseResHandle FileDependencies::GetDependencyByType(uint32 uUsageCRC) const
	{
		for (auto& dep : m_dependencies)
		{
			if (dep.uUsageCRC == uUsageCRC)
			{
				return dep.resHandle;
			}
		}
		return BaseResHandle(nullptr);
	}

	void FileDependencies::GetAllDependenciesOfType(uint32 uUsageCRC, usg::vector<const FileDependency*>& depOut) const
	{
		for (auto& dep : m_dependencies)
		{
			if (dep.uUsageCRC == uUsageCRC)
			{
				depOut.push_back(&dep);
			}
		}
	}


}