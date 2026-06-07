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

	void FileDependencies::Init(class PakFile* pCurrentFile, const char* szPakName, const PakFileDecl::FileInfo* pOwnerFile, const PakFileDecl::Dependency* pDependencies, uint32 uDependencyCount)
	{
		m_dependencies.resize(uDependencyCount);
		for (uint32 i = 0; i < uDependencyCount; i++)
		{
			FileDependency& dep = m_dependencies[i];
			if (pDependencies[i].PakIndex != USG_INVALID_ID)
			{
				dep.resHandle = pCurrentFile->GetResource(pDependencies[i].FileCRC);
				if (!dep.resHandle)
				{
					DEBUG_PRINT("Missing pak dependency: pak=%s owner=%s owner_crc=0x%08x owner_type=%u dependency=%u file_crc=0x%08x file_crc_no_ext=0x%08x pak_index=%u usage_crc=0x%08x\n",
						szPakName ? szPakName : "<unknown>",
						pOwnerFile ? pOwnerFile->szName : "<unknown>",
						pOwnerFile ? pOwnerFile->CRC : 0,
						pOwnerFile ? pOwnerFile->uResourceType : 0,
						i,
						pDependencies[i].FileCRC,
						pDependencies[i].FileCRCNoExt,
						pDependencies[i].PakIndex,
						pDependencies[i].UsageCRC);
				}
			}
			else
			{
				// TODO: Dependencies from other files
				DEBUG_PRINT("Unsupported external pak dependency: pak=%s owner=%s owner_crc=0x%08x owner_type=%u dependency=%u file_crc=0x%08x file_crc_no_ext=0x%08x usage_crc=0x%08x\n",
					szPakName ? szPakName : "<unknown>",
					pOwnerFile ? pOwnerFile->szName : "<unknown>",
					pOwnerFile ? pOwnerFile->CRC : 0,
					pOwnerFile ? pOwnerFile->uResourceType : 0,
					i,
					pDependencies[i].FileCRC,
					pDependencies[i].FileCRCNoExt,
					pDependencies[i].UsageCRC);
			}
			dep.uFileCRC = pDependencies[i].FileCRC;
			dep.uUsageCRC = pDependencies[i].UsageCRC;
			dep.uFileCRCNoExt = pDependencies[i].FileCRCNoExt;
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
		if (uFileIndex >= m_dependencies.size())
		{
			DEBUG_PRINT("Dependency index %u out of range. Dependency count is %u\n", uFileIndex, (uint32)m_dependencies.size());
			return BaseResHandle(nullptr);
		}
		return m_dependencies[uFileIndex].resHandle;
	}

	BaseResHandle FileDependencies::GetDependencyByUsageCRC(uint32 uUsageCRC) const
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

	void FileDependencies::GetAllDependenciesWithUsageCRC(uint32 uUsageCRC, usg::vector<const FileDependency*>& depOut) const
	{
		for (auto& dep : m_dependencies)
		{
			if (dep.uUsageCRC == uUsageCRC)
			{
				depOut.push_back(&dep);
			}
		}
	}

	BaseResHandle FileDependencies::GetDependencyByFileType(ResourceType eType) const
	{
		for (auto& dep : m_dependencies)
		{
			if (dep.resHandle && dep.resHandle->GetResourceType() == eType)
			{
				return dep.resHandle;
			}
		}
		return BaseResHandle(nullptr);
	}

	BaseResHandle FileDependencies::GetDependencyByCRCAndType(uint32 uFileCRCNoExt, ResourceType eType) const
	{
		for (auto& dep : m_dependencies)
		{
			if (dep.resHandle && dep.resHandle->GetResourceType() == eType && (uFileCRCNoExt == dep.uFileCRCNoExt))
			{
				return dep.resHandle;
			}
		}
		return BaseResHandle(nullptr);
	}

	void FileDependencies::GetAllDependenciesWithFileType(ResourceType eType, usg::vector<const FileDependency*>& depOut) const
	{
		for (auto& dep : m_dependencies)
		{
			if (dep.resHandle && dep.resHandle->GetResourceType() == eType)
			{
				depOut.push_back(&dep);
			}
		}
	}


}
