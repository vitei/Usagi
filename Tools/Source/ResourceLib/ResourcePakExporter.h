#pragma once

#include "Engine/Core/Utility.h"
#include "Engine/Resource/PakDecl.h"
#include <algorithm>
#include <vector>

struct DependencyEntry
{
	std::string fileName;
	std::string usage;
	uint32 fileNameCRC;
	uint32 usageCRC;
};

struct ResourceEntry
{
	std::string srcName;	// There may be multiple files with the same srcName (e.g. model + animations from a single fbx)

	ResourceEntry() {}
	virtual ~ResourceEntry() {}

	virtual const void* GetData() = 0;
	virtual uint32 GetDataSize() = 0;
	// Data which is going to be transfered to the GPU should return false, other data
	// should return true so as to avoid copying memory
	virtual bool KeepDataAfterLoading() { return false; }
	virtual const void* GetCustomHeader() = 0;
	virtual uint32 GetCustomHeaderSize() = 0;

	void SetName(const std::string& fileName, usg::ResourceType eType)
	{
		name = fileName;
		uFileCRC = utl::CRC32(fileName.c_str());
		resourceType = eType;
	}
	void AddDependency(const std::string& fileName, const std::string& usage)
	{
		DependencyEntry entry;
		entry.fileName = fileName;
		entry.usage = usage;
		entry.fileNameCRC = utl::CRC32(fileName.c_str());
		entry.usageCRC = utl::CRC32(usage.c_str());
		dependencies.push_back(entry);
	}
	const std::vector<DependencyEntry>& GetDeps() const { return dependencies; }
	const std::string& GetName() const { return name; }
	uint32 GetNameCRC() const { return uFileCRC; }
	usg::ResourceType GetResourceType() const { return resourceType; }

	bool operator<(const ResourceEntry &rhs) const
	{
		const auto& rhsDeps = rhs.GetDeps();
		if (dependencies.size() == 0 && rhs.dependencies.size() > 0)
			return true;
		for (size_t i = 0; i < rhsDeps.size(); i++)
		{
			if (rhsDeps[i].fileNameCRC == uFileCRC)
			{
				return true;
			}
		}
		return false;
	}

private:
	std::vector<DependencyEntry> dependencies;
	std::string name;
	usg::ResourceType resourceType;
	uint32 uFileCRC;
};

inline bool ComparePointers(const ResourceEntry * const & a, const ResourceEntry * const & b)
{
	return *a < *b;
}

namespace ResourcePakExporter
{
	// Note entries may be re-ordered by this function
	inline bool Export(const char* szFileName, std::vector<ResourceEntry*>& entries)
	{
		// Sort these entries so that dependencies come before other files
		std::sort(entries.begin(), entries.end(), ComparePointers);

		uint32 uCPUDataOffset = sizeof(usg::PakFileDecl::ResourcePakHdr);
		for (uint32 i = 0; i < entries.size(); i++)
		{
			uCPUDataOffset += sizeof(usg::PakFileDecl::FileInfo);
			uCPUDataOffset += entries[i]->GetCustomHeaderSize();
			uCPUDataOffset += (uint32)(entries[i]->GetDeps().size() * sizeof(usg::PakFileDecl::Dependency));
		}

		uint32 uGPUDataOffset = uCPUDataOffset;
		for (uint32 i = 0; i < entries.size(); i++)
		{
			if (entries[i]->KeepDataAfterLoading())
			{
				uGPUDataOffset += entries[i]->GetDataSize();
			}
		}

		FILE* pFileOut = nullptr;
		fopen_s(&pFileOut, szFileName, "wb");

		if (!pFileOut)
		{
			DEBUG_PRINT("Unable to open file %s", szFileName);
			return false;
		}

		usg::PakFileDecl::ResourcePakHdr hdr;
		hdr.uVersionId = usg::PakFileDecl::CURRENT_VERSION;
		hdr.uFileCount = (uint32)entries.size();
		hdr.uResDataOffset = uCPUDataOffset;
		hdr.uTempDataOffset = uGPUDataOffset;

		fwrite(&hdr, sizeof(hdr), 1, pFileOut);

		for (uint32 i = 0; i < entries.size(); i++)
		{
			usg::PakFileDecl::FileInfo fileInfo;
			strcpy_s(fileInfo.szName, entries[i]->GetName().c_str());
			fileInfo.CRC = utl::CRC32(fileInfo.szName);
			fileInfo.uCustomHeaderSize = entries[i]->GetCustomHeaderSize();
			fileInfo.uDependenciesCount = (uint32)entries[i]->GetDeps().size();
			fileInfo.uDataSize = entries[i]->GetDataSize();
			fileInfo.uResourceType = (uint32)entries[i]->GetResourceType();
			// TODO: Probably want to have an alignment value for the data
			fileInfo.uTotalFileInfoSize = (uint32)(sizeof(fileInfo) + entries[i]->GetCustomHeaderSize() + (sizeof(usg::PakFileDecl::Dependency) * entries[i]->GetDeps().size()));
			if (fileInfo.uDataSize > 0)
			{
				if (entries[i]->KeepDataAfterLoading())
				{
					fileInfo.uDataOffset = uCPUDataOffset;
					uCPUDataOffset += fileInfo.uDataSize;
				}
				else
				{
					fileInfo.uDataOffset = uGPUDataOffset;
					uGPUDataOffset += fileInfo.uDataSize;
				}
			}
			else
			{
				fileInfo.uDataOffset = USG_INVALID_ID;
			}
			fwrite(&fileInfo, sizeof(fileInfo), 1, pFileOut);

			if (fileInfo.uCustomHeaderSize > 0)
			{
				fwrite(entries[i]->GetCustomHeader(), entries[i]->GetCustomHeaderSize(), 1, pFileOut);

			}

			if (fileInfo.uDependenciesCount > 0)
			{
				for (auto& depItr : entries[i]->GetDeps())
				{
					usg::PakFileDecl::Dependency dep;
					dep.FileCRC = depItr.fileNameCRC;
					dep.UsageCRC = depItr.usageCRC;
					dep.PakIndex = USG_INVALID_ID;
					for (size_t depId = 0; depId < entries.size(); depId++)
					{
						if (entries[depId]->GetNameCRC() == depItr.fileNameCRC)
						{
							dep.PakIndex = (uint32)i;
						}
					}
					fwrite(&dep, sizeof(dep), 1, pFileOut);
				}
			}
		}


		// Now do the CPU data
		for (uint32 i = 0; i < entries.size(); i++)
		{
			if (entries[i]->KeepDataAfterLoading())
			{
				fwrite(entries[i]->GetData(), entries[i]->GetDataSize(), 1, pFileOut);
			}
		}

		// Now do the GPU data
		for (uint32 i = 0; i < entries.size(); i++)
		{
			if (!entries[i]->KeepDataAfterLoading())
			{
				fwrite(entries[i]->GetData(), entries[i]->GetDataSize(), 1, pFileOut);
			}
		}

		fclose(pFileOut);

		return true;
	}
};