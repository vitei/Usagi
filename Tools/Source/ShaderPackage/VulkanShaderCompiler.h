#pragma once
#include "IShaderCompiler.h"
#include <shaderc/shaderc.h>

class VulkanShaderCompiler :
	public IShaderCompiler
{
public:
	VulkanShaderCompiler();
	~VulkanShaderCompiler();

	virtual void Init() override;

	virtual bool Compile(const std::string& inputFileName, const std::string& setDefines, const std::string& tempFileName, const std::string& includes,
		ShaderEntry& shader, const class MaterialDefinitionExporter* pMaterialDef, std::vector<std::string>& referencedFiles, usg::ShaderType eType) override;

	virtual void CleanUp() override;

private:
	shaderc_compiler_t m_compiler;
	shaderc_compile_options_t m_compileOptions;
};

