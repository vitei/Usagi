#ifndef DependencyTracker_h__
#define DependencyTracker_h__
#include <vector>
#include <string>

class DependencyTracker
{
public:
	DependencyTracker();
	~DependencyTracker();

	int StartRecord(const char* dependencyFile, const char* outputFile );
	void LogDependency(const char* dependency);
	void ExportDependencies();

private:
	std::string ParsePath(const char* path);

	std::string m_dependencyFile;
	std::string m_outputFile;
	std::vector<std::string> m_dependencies;
};

#endif // DependencyTracker_h__
