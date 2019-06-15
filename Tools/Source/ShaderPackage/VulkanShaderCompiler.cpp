#include "Engine/Common/Common.h"
#include "VulkanShaderCompiler.h"


const shaderc_shader_kind g_shaderKinds[] =
{
	shaderc_shader_kind::shaderc_glsl_vertex_shader,
	shaderc_shader_kind::shaderc_glsl_fragment_shader,
	shaderc_shader_kind::shaderc_glsl_geometry_shader
};

VulkanShaderCompiler::VulkanShaderCompiler()
{
}


VulkanShaderCompiler::~VulkanShaderCompiler()
{
}


void VulkanShaderCompiler::Init()
{
	m_compiler = shaderc_compiler_initialize();
	m_compileOptions = shaderc_compile_options_initialize();

	// Enable debug
	shaderc_compile_options_set_generate_debug_info(m_compileOptions);

	shaderc_compile_options_set_optimization_level(m_compileOptions, shaderc_optimization_level_performance);

	shaderc_compile_options_set_warnings_as_errors(m_compileOptions);
}


#if 1
bool VulkanShaderCompiler::Compile(const std::string& inputFileName, const std::string& setDefines, const std::string& tempFileName, const std::string& includes,
	ShaderEntry& shader, const class MaterialDefinitionExporter* pMaterialDef, std::vector<std::string>& referencedFiles, usg::ShaderType eType)
{
	std::string shaderCode;
	std::string defines = setDefines;
	if (setDefines.size() > 0)
		defines += " ";
	defines += "PLATFORM_PC ";
	defines += "API_VULKAN";


	if (ParseManually(inputFileName.c_str(), defines.c_str(), pMaterialDef, includes, shaderCode, referencedFiles, eType))
	{
		// Want to move away from manual parsing, when we do we'll need to pass in the defines like this
		//shaderc_compile_options_clone
	//SHADERC_EXPORT void shaderc_compile_options_add_macro_definition(
//		shaderc_compile_options_t options, const char* name, size_t name_length,
	//	const char* value, size_t value_length);

		shaderc_compilation_result_t result = shaderc_compile_into_spv(m_compiler, shaderCode.c_str(), shaderCode.size(), g_shaderKinds[(uint32)shader.entry.eShaderType], inputFileName.c_str(), "main", m_compileOptions);
		shaderc_compilation_status status = shaderc_result_get_compilation_status(result);
		if (status == shaderc_compilation_status_success)
		{
			shader.binary = new uint8[shaderc_result_get_length(result)];
			memcpy(shader.binary, shaderc_result_get_bytes(result), shaderc_result_get_length(result));
			shader.binarySize = (uint32)shaderc_result_get_length(result);
			shaderc_result_release(result);
			return true;
		}
		else
		{
			const char* msg = shaderc_result_get_error_message(result);
			ASSERT_MSG(false, msg);
			return false;
		}
	}


	return false;
}

#else
bool VulkanShaderCompiler::Compile(const std::string& inputFileName, const std::string& setDefines, const std::string& tempFileName, const std::string& includes,
	ShaderEntry& shader, std::vector<std::string>& referencedFiles)
{
	// Get the input file name
	std::string outputFileName = tempFileName;

	std::string defines = "-DPLATFORM_PC -DAPI_VULKAN";
	// TODO: Handle multiple include directories
	std::string defineList = setDefines;
	size_t nextDefine = std::string::npos;
	while (!defineList.empty())
	{
		nextDefine = defineList.find_first_of(' ');
		if (nextDefine != std::string::npos)
		{
			defines += std::string(" -D") + defineList.substr(0, nextDefine);
			defineList = defineList.substr(nextDefine + 1);
		}
		else
		{
			// The last define
			defines += std::string(" -D") + defineList;
			defineList.clear();
		}

	} while (nextDefine != std::string::npos);

	std::stringstream command;
	// Delete the last instance of this file
	DeleteFile(outputFileName.c_str());
	std::replace(outputFileName.begin(), outputFileName.end(), '/', '\\');
	std::string outputDir = outputFileName.substr(0, outputFileName.find_last_of("\\/"));
	CreateDirectory(outputDir.c_str(), NULL);

	command << "glslc " << inputFileName.c_str() << " -o" << outputFileName.c_str() << " -MD -std=450 -Werror " << defines << " " << includes;
	//glslang::TShader* shader = new glslang::TShader(g_glslLangLang[j]);
	// FIXME: code for glslang natively, but for now it is cleaner to use the command line
	system(command.str().c_str());

	FILE* pFileOut = nullptr;
	fopen_s(&pFileOut, outputFileName.c_str(), "rb");
	// FIXME: Should do cleanup
	if (!pFileOut)
		return false;
	fseek(pFileOut, 0, SEEK_END);
	shader.binarySize = ftell(pFileOut);
	fseek(pFileOut, 0, SEEK_SET);
	shader.binary = new uint8[shader.binarySize];
	fread(shader.binary, 1, shader.binarySize, pFileOut);


	std::string depFileName = outputFileName + ".d";
	std::ifstream depFile(depFileName);

	std::string intermediateDep;
	std::getline(depFile, intermediateDep, ':');
	while (depFile >> intermediateDep)
	{
		std::replace(intermediateDep.begin(), intermediateDep.end(), '\\', '/');
		if (std::find(referencedFiles.begin(), referencedFiles.end(), intermediateDep) == referencedFiles.end())
		{
			referencedFiles.push_back(intermediateDep);
		}
	}

	fclose(pFileOut);
	depFile.close();

	return true;
}
#endif

void VulkanShaderCompiler::CleanUp()
{
	shaderc_compiler_release(m_compiler);
	shaderc_compile_options_release(m_compileOptions);
}