#include "Engine/Common/Common.h"
#include "OpenGLShaderCompiler.h"
#include "Parser.h"


OpenGLShaderCompiler::OpenGLShaderCompiler()
{
}


OpenGLShaderCompiler::~OpenGLShaderCompiler()
{
}


bool OpenGLShaderCompiler::Compile(const std::string& inputFileName, const std::string& setDefines, const std::string& tempFileName, const std::string& includes,
	ShaderEntry& shader, const class MaterialDefinitionExporter* pMaterialDef, std::vector<std::string>& referencedFiles, usg::ShaderType eType)
{
	std::string shaderCode;
	std::string defines = setDefines;
	if (setDefines.size() > 0)
		defines += " ";
	defines += "PLATFORM_PC ";
	defines += "API_OGL";

	if (ParseManually(inputFileName.c_str(), defines.c_str(), pMaterialDef, includes, shaderCode, referencedFiles, eType))
	{
		shader.binary = new uint8[shaderCode.size() + 1];
		memcpy(shader.binary, shaderCode.data(), shaderCode.size() + 1);
		shader.binarySize = (uint32)shaderCode.size() + 1;
		return true;
	}
	return false;
}