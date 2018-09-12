#pragma once
#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"
#include "Engine/Resource/PakDecl.h"
#include <vector>


struct ResourceEntry
{
	std::string name;
	std::string srcName;	// There may be multiple files with the same srcName (e.g. model + animations from a single fbx)
	std::vector<std::string> dependencies;

	ResourceEntry() {}
	virtual ~ResourceEntry() {}

	virtual void* GetData() = 0;
	virtual uint32 GetDataSize() = 0;
	virtual void* GetCustomHeader() = 0;
	virtual uint32 GetCustomHeaderSize() = 0;

};

namespace ResourcePakExporter
{
	inline bool Export(const char* szFileName, std::vector<ResourceEntry*>& entries)
	{
		uint32 uDataOffset = sizeof(usg::PakFileDecl::ResourcePakHdr);
		for (uint32 i = 0; i < entries.size(); i++)
		{
			uDataOffset += sizeof(usg::PakFileDecl::FileInfo);
			uDataOffset += entries[i]->GetCustomHeaderSize();
			uDataOffset += (uint32)(entries[i]->dependencies.size() * sizeof(usg::PakFileDecl::Dependency));
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

		fwrite(&hdr, sizeof(hdr), 1, pFileOut);

		for (uint32 i = 0; i < entries.size(); i++)
		{
			usg::PakFileDecl::FileInfo fileInfo;
			strcpy_s(fileInfo.szName, entries[i]->name.c_str());
			fileInfo.CRC = utl::CRC32(fileInfo.szName);
			fileInfo.uCustomHeaderSize = entries[i]->GetCustomHeaderSize();
			fileInfo.uDependenciesCount = (uint32)entries[i]->dependencies.size();
			fileInfo.uDataSize = entries[i]->GetDataSize();
			fileInfo.uDataOffset = fileInfo.uDataSize > 0 ? uDataOffset : USG_INVALID_ID;
			// TODO: Probably want to have an alignment value for the data
			fileInfo.uTotalFileInfoSize = (uint32)(sizeof(fileInfo) + entries[i]->GetCustomHeaderSize() + (sizeof(usg::PakFileDecl::Dependency) * entries[i]->dependencies.size()));
			uDataOffset += fileInfo.uDataSize;
			fwrite(&fileInfo, sizeof(fileInfo), 1, pFileOut);

			if (fileInfo.uCustomHeaderSize > 0)
			{
				fwrite(entries[i]->GetCustomHeader(), entries[i]->GetCustomHeaderSize(), 1, pFileOut);

			}

			if (fileInfo.uDependenciesCount > 0)
			{
				for (auto& itr : entries[i]->dependencies)
				{
					usg::PakFileDecl::Dependency dep;
					strcpy_s(dep.szName, itr.c_str());
					dep.FileCRC = utl::CRC32(dep.szName, (uint32)itr.size());
					fwrite(&dep, sizeof(dep), 1, pFileOut);
				}
			}
		}


		// Now do the data
		for (uint32 i = 0; i < entries.size(); i++)
		{
			fwrite(entries[i]->GetData(), entries[i]->GetDataSize(), 1, pFileOut);
		}

		fclose(pFileOut);

		return true;
	}
};