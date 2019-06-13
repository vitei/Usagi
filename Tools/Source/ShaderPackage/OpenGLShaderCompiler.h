#pragma once
#include "IShaderCompiler.h"
class OpenGLShaderCompiler :
	public IShaderCompiler
{
public:
	OpenGLShaderCompiler();
	~OpenGLShaderCompiler();

	virtual bool Compile(const std::string& inputFileName, const std::string& setDefines, const std::string& tempFileName, const std::string& includes,
		ShaderEntry& shader, std::vector<std::string>& referencedFiles) override;
};

