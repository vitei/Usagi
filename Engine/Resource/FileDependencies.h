/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: PakFile for multiple files merged into one
*****************************************************************************/
#pragma once
HEADER_GUARD_OPEN(RESOURCE_FILE_DEPENDENCIES)
#include "Engine/Common/Common.h"
#include "Engine/Core/stl/vector.h"
#include "ResourceDecl.h"
#include "PakDecl.h"

namespace usg
{

	class FileDependencies
	{
	public:
		FileDependencies();
		~FileDependencies();

		struct FileDependency
		{
			BaseResHandle	resHandle;
			uint32			uFileCRC;
			uint32			uUsageCRC;
		};

		void Init(class PakFile* pCurrentFile, const PakFileDecl::Dependency* pDependencies, uint32 uDependencyCount);
		uint32 GetDependencyCount() const { return (uint32)m_dependencies.size(); }
		BaseResHandle GetDependencyByCRC(uint32 uFileCRC) const;
		BaseResHandle GetDependencyByIndex(uint32 uFileIndex) const;
		BaseResHandle GetDependencyByType(uint32 uUsageCRC) const;
		void GetAllDependenciesOfType(uint32 uUsageCRC, usg::vector<const FileDependency*>& depOut) const;

	private:

		usg::vector<FileDependency>	m_dependencies;

	};

}

HEADER_GUARD_CLOSE
