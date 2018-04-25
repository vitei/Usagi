#include "DependencyTracker.h"
#include "common.h"
#include <algorithm>

DependencyTracker::DependencyTracker() 
{

}

DependencyTracker::~DependencyTracker() 
{

}

int DependencyTracker::StartRecord(const char* dependencyFile, const char* outputFile )
{
	m_dependencyFile = dependencyFile;
	m_outputFile = ParsePath(outputFile);
	return 0;
}

void DependencyTracker::LogDependency(const char* dependency)
{
	std::string depString = ParsePath(dependency);
	for(uint32 i=0; i<m_dependencies.size(); i++)
	{
		if(m_dependencies[i] == depString)
		{
			return;
		}
	}

	m_dependencies.push_back(depString);
}

void DependencyTracker::ExportDependencies()
{
	if (m_dependencyFile.size() == 0)
	{
		// Dependencies not enabled
		return;
	}

	FILE* handle = fopen(m_dependencyFile.c_str(), "w" );
	if(!handle)
	{
		ASSERT(false);
		return;
	}


	fputs(m_outputFile.c_str(), handle);
	fputs(":", handle);

	for(uint32 i=0; i<m_dependencies.size(); i++)
	{
		fputs(" \\\n", handle);
		fputs(m_dependencies[i].c_str(), handle);
	}

	fclose(handle);

}

std::string DependencyTracker::ParsePath(const char* path)
{
	std::string out = path;
	std::replace( out.begin(), out.end(), '\\', '/'); // replace all backslashes with forward slashes
	return out;
}

