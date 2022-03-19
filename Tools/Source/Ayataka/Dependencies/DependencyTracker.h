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

	const std::vector<std::string>& GetDependencies() { return m_dependencies; }

private:
	std::string ParsePath(const char* path);

	std::string m_dependencyFile;
	std::string m_outputFile;
	std::vector<std::string> m_dependencies;
};

#endif // DependencyTracker_h__
