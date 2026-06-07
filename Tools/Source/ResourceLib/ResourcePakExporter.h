#pragma once

#include "Engine/Core/Utility.h"
#include "Engine/Resource/PakDecl.h"
#include <algorithm>
#include <string>
#include <vector>

struct DependencyEntry
{
	std::string fileName;
	std::string usage;
	uint32 fileNameCRC;
	uint32 usageCRC;
	uint32 fileNameCRCNoExt;
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
		std::string noExt = fileName.substr(0, fileName.find_last_of("."));

		uFileCRC = utl::CRC32(fileName.c_str());
		uFileCRCNoExt = utl::CRC32(noExt.c_str());
		resourceType = eType;
	}
	void AddDependency(const std::string& fileName, const std::string& usage)
	{
		DependencyEntry entry;
		entry.fileName = fileName;
		entry.usage = usage;
		entry.fileNameCRC = utl::CRC32(fileName.c_str());

		std::string noExt = fileName.substr(0, fileName.find_last_of("."));
		entry.fileNameCRCNoExt = utl::CRC32(noExt.c_str());

		entry.usageCRC = utl::CRC32(usage.c_str());
		dependencies.push_back(entry);
	}
	const std::vector<DependencyEntry>& GetDeps() const { return dependencies; }
	const std::string& GetName() const { return name; }
	uint32 GetNameCRC() const { return uFileCRC; }
	uint32 GetNameCRCNoExt() const { return uFileCRCNoExt;  }
	usg::ResourceType GetResourceType() const { return resourceType; }

private:
	std::vector<DependencyEntry> dependencies;
	std::string name;
	usg::ResourceType resourceType;
	uint32 uFileCRC;
	uint32 uFileCRCNoExt;
};

namespace ResourcePakExporter
{
	inline bool ResourceEntryLess(const ResourceEntry* pLhs, const ResourceEntry* pRhs)
	{
		const int nameCmp = pLhs->GetName().compare(pRhs->GetName());
		if (nameCmp != 0)
		{
			return nameCmp < 0;
		}

		return pLhs->GetNameCRC() < pRhs->GetNameCRC();
	}

	inline int FindResourceIndex(const std::vector<ResourceEntry*>& entries, uint32 uFileCRC)
	{
		for (uint32 i = 0; i < entries.size(); i++)
		{
			if (entries[i]->GetNameCRC() == uFileCRC)
			{
				return (int)i;
			}
		}

		return -1;
	}

	inline bool SortAndValidateDependencies(const char* szFileName, std::vector<ResourceEntry*>& entries)
	{
		std::sort(entries.begin(), entries.end(), ResourceEntryLess);

		for (uint32 i = 0; i < entries.size(); i++)
		{
			for (uint32 j = i + 1; j < entries.size(); j++)
			{
				if (entries[i]->GetNameCRC() == entries[j]->GetNameCRC())
				{
					RELEASE_WARNING("Resources %s and %s have duplicate CRCs in pak %s\n", entries[i]->GetName().c_str(), entries[j]->GetName().c_str(), szFileName);
					return false;
				}
			}
		}

		std::vector<std::vector<uint32>> dependents(entries.size());
		std::vector<uint32> dependencyCounts(entries.size(), 0);

		for (uint32 i = 0; i < entries.size(); i++)
		{
			for (auto& depItr : entries[i]->GetDeps())
			{
				const int depId = FindResourceIndex(entries, depItr.fileNameCRC);
				if (depId < 0)
				{
					RELEASE_WARNING("Dependency %s for resource %s was not found in pak %s\n", depItr.fileName.c_str(), entries[i]->GetName().c_str(), szFileName);
					return false;
				}

				dependents[depId].push_back(i);
				dependencyCounts[i]++;
			}
		}

		for (auto& dependentList : dependents)
		{
			std::sort(dependentList.begin(), dependentList.end(), [&](uint32 lhs, uint32 rhs)
			{
				return ResourceEntryLess(entries[lhs], entries[rhs]);
			});
		}

		std::vector<uint32> ready;
		for (uint32 i = 0; i < entries.size(); i++)
		{
			if (dependencyCounts[i] == 0)
			{
				ready.push_back(i);
			}
		}

		std::vector<ResourceEntry*> sortedEntries;
		sortedEntries.reserve(entries.size());

		while (!ready.empty())
		{
			std::sort(ready.begin(), ready.end(), [&](uint32 lhs, uint32 rhs)
			{
				return ResourceEntryLess(entries[lhs], entries[rhs]);
			});

			const uint32 entryId = ready.front();
			ready.erase(ready.begin());

			sortedEntries.push_back(entries[entryId]);

			for (uint32 dependentId : dependents[entryId])
			{
				ASSERT(dependencyCounts[dependentId] > 0);
				dependencyCounts[dependentId]--;
				if (dependencyCounts[dependentId] == 0)
				{
					ready.push_back(dependentId);
				}
			}
		}

		if (sortedEntries.size() != entries.size())
		{
			for (uint32 i = 0; i < entries.size(); i++)
			{
				if (dependencyCounts[i] > 0)
				{
					RELEASE_WARNING("Resource %s has cyclic dependencies in pak %s\n", entries[i]->GetName().c_str(), szFileName);
				}
			}
			return false;
		}

		entries.swap(sortedEntries);
		return true;
	}

	// Note entries may be re-ordered by this function
	inline bool Export(const char* szFileName, std::vector<ResourceEntry*>& entries)
	{
		if (!SortAndValidateDependencies(szFileName, entries))
		{
			return false;
		}

		uint32 uTmpOffset = sizeof(usg::PakFileDecl::ResourcePakHdr);
		for (uint32 i = 0; i < entries.size(); i++)
		{
			uTmpOffset += sizeof(usg::PakFileDecl::FileInfo);
			uTmpOffset += entries[i]->GetCustomHeaderSize();
			uTmpOffset += (uint32)(entries[i]->GetDeps().size() * sizeof(usg::PakFileDecl::Dependency));
		}

		uint32 uKeepOffset = uTmpOffset;
		bool bHasPersistentData = false;
		for (uint32 i = 0; i < entries.size(); i++)
		{
			if (!entries[i]->KeepDataAfterLoading())
			{
				uKeepOffset += entries[i]->GetDataSize();
			}
			else if (entries[i]->GetDataSize() > 0)
			{
				bHasPersistentData = true;
			}
		}

		const uint32 uPersistentDataOffset = bHasPersistentData ? uKeepOffset : USG_INVALID_ID;

		FILE* pFileOut = nullptr;
		fopen_s(&pFileOut, szFileName, "wb");

		if (!pFileOut)
		{
			RELEASE_WARNING("Unable to open file %s", szFileName);
			return false;
		}

		usg::PakFileDecl::ResourcePakHdr hdr;
		hdr.uVersionId = usg::PakFileDecl::CURRENT_VERSION;
		hdr.uFileCount = (uint32)entries.size();
		hdr.uResDataOffset = uPersistentDataOffset;
		hdr.uTempDataOffset = uTmpOffset;

		fwrite(&hdr, sizeof(hdr), 1, pFileOut);

		for (uint32 i = 0; i < entries.size(); i++)
		{
			usg::PakFileDecl::FileInfo fileInfo;
			strcpy_s(fileInfo.szName, entries[i]->GetName().c_str());
			fileInfo.CRC = utl::CRC32(fileInfo.szName);
			fileInfo.CRCNoExt = entries[i]->GetNameCRCNoExt();
			fileInfo.uCustomHeaderSize = entries[i]->GetCustomHeaderSize();
			fileInfo.uDependenciesCount = (uint32)entries[i]->GetDeps().size();
			fileInfo.uDataSize = entries[i]->GetDataSize();
			fileInfo.uResourceType = (uint32)entries[i]->GetResourceType();
			fileInfo.uFileFlags = entries[i]->KeepDataAfterLoading() ? usg::PakFileDecl::FILE_FLAG_KEEP_DATA : 0;
			// TODO: Probably want to have an alignment value for the data
			fileInfo.uTotalFileInfoSize = (uint32)(sizeof(fileInfo) + entries[i]->GetCustomHeaderSize() + (sizeof(usg::PakFileDecl::Dependency) * entries[i]->GetDeps().size()));
			if (fileInfo.uDataSize > 0)
			{
				if (!entries[i]->KeepDataAfterLoading())
				{
					fileInfo.uDataOffset = uTmpOffset;
					uTmpOffset += fileInfo.uDataSize;
				}
				else
				{
					fileInfo.uDataOffset = uKeepOffset;
					uKeepOffset += fileInfo.uDataSize;
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
					dep.FileCRCNoExt = depItr.fileNameCRCNoExt;
					for (size_t depId = 0; depId < entries.size(); depId++)
					{
						if (entries[depId]->GetNameCRC() == depItr.fileNameCRC)
						{
							dep.PakIndex = (uint32)depId;
							break;
						}
					}
					if (dep.PakIndex == USG_INVALID_ID || dep.PakIndex >= i)
					{
						RELEASE_WARNING("Dependency %s for resource %s is not ordered before the resource in pak %s\n", depItr.fileName.c_str(), entries[i]->GetName().c_str(), szFileName);
						fclose(pFileOut);
						return false;
					}
					fwrite(&dep, sizeof(dep), 1, pFileOut);
				}
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

		// Now do the CPU data
		for (uint32 i = 0; i < entries.size(); i++)
		{
			if (entries[i]->KeepDataAfterLoading())
			{
				fwrite(entries[i]->GetData(), entries[i]->GetDataSize(), 1, pFileOut);
			}
		}

		fclose(pFileOut);

		return true;
	}
};
