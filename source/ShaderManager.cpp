/* Author: BAIRAC MIHAI */

#include "ShaderManager.h"
#include "CommonHeaders.h"
#include "GLConfig.h"
#include "FileUtils.h"
#include "GlobalConfig.h"
#include <sstream>
#include <assert.h>
#include <new>


ShaderManager::ShaderManager ( void )
	: m_Name("Default"), m_ShaderProgramID(0), m_VertexShaderID(0),
	  m_GeometryShaderID(0), m_FragmentShaderID(0), m_ComputeShaderID(0),
	  m_UseStrictVerification(false)
{
	LOG("Shader Manager [%s] successfully created!", m_Name.c_str());
}

ShaderManager::ShaderManager ( const std::string &i_Name )
	: m_ShaderProgramID(0), m_VertexShaderID(0),
	  m_GeometryShaderID(0), m_FragmentShaderID(0), m_ComputeShaderID(0),
	  m_UseStrictVerification(false)
{
	Initialize(i_Name);
}

ShaderManager::~ShaderManager ( void )
{
	Destroy();
}

void ShaderManager::Initialize ( const std::string &i_Name )
{
	m_Name = i_Name;

	LOG("Shader Manager [%s] successfully created!", m_Name.c_str());
}

void ShaderManager::Destroy ( void )
{
	glUseProgram(0);

	if (m_ShaderProgramID)
	{
		if (m_VertexShaderID)
		{
			glDetachShader(m_ShaderProgramID, m_VertexShaderID);
			glDeleteShader(m_VertexShaderID);
			m_VertexShaderID = 0;
		}

		if (m_GeometryShaderID)
		{
			glDetachShader(m_ShaderProgramID, m_GeometryShaderID);
			glDeleteShader(m_GeometryShaderID);
			m_GeometryShaderID = 0;
		}

		if (m_FragmentShaderID)
		{
			glDetachShader(m_ShaderProgramID, m_FragmentShaderID);
			glDeleteShader(m_FragmentShaderID);
			m_FragmentShaderID = 0;
		}

		if (m_ComputeShaderID)
		{
			glDetachShader(m_ShaderProgramID, m_ComputeShaderID);
			glDeleteShader(m_ComputeShaderID);
			m_ComputeShaderID = 0;
		}

		glDeleteProgram(m_ShaderProgramID);
		m_ShaderProgramID = 0;
	}

	LOG("Shader Manager [%s] successfully destroyed!", m_Name.c_str());
}

void ShaderManager::BuildRenderingProgram ( const std::string& i_VertexFileName, const std::string& i_FragmentFileName, const GlobalConfig& i_Config )
{
	if (!CreateRenderingProgram(i_VertexFileName, i_FragmentFileName, i_Config))
	{
		ERR("Rendering program creation failed!");
		return;
	}

	if (!LinkProgram())
	{
		ERR("Rendering program linking failed!");
		return;
	}
}

void ShaderManager::BuildRenderingProgram ( const std::string& i_VertexFileName, const std::string& i_GeometryFileName, const std::string& i_FragmentFileName, const GlobalConfig& i_Config )
{
	if (!i_Config.GLExtVars.IsGeometryShaderSupported)
	{
		ERR("Geometry shaders not supported!");
		return;
	}

	if (!CreateRenderingProgram(i_VertexFileName, i_GeometryFileName, i_FragmentFileName, i_Config))
	{
		ERR("Rendering program creation failed!");
		return;
	}

	if (!LinkProgram())
	{
		ERR("Rendering program linking failed!");
		return;
	}
}

void ShaderManager::BuildComputeProgram ( const std::string& i_ComputeFileName, const GlobalConfig& i_Config )
{
	if (!i_Config.GLExtVars.IsComputeShaderSupported)
	{
		ERR("Compute shaders not supported!");
		return;
	}

	if (!CreateComputeProgram(i_ComputeFileName, i_Config))
	{
		ERR("Compute program creation failed!");
		return;
	}

	if (!LinkProgram())
	{
		ERR("Compute program linking failed!");
		return;
	}

	return;
}

bool ShaderManager::CreateRenderingProgram ( const std::string& i_VertexFileName, const std::string& i_FragmentFileName, const GlobalConfig& i_Config )
{
	if (i_VertexFileName.empty() || i_FragmentFileName.empty())
	{
		ERR("Empty shader file name!");
		return false;
	}

	if (!CreateShader(i_VertexFileName, GL_VERTEX_SHADER, i_Config))
	{
		ERR("Vertex shader %s creation failed!", i_VertexFileName.c_str());
		return false;
	}

	if (!CreateShader(i_FragmentFileName, GL_FRAGMENT_SHADER, i_Config))
	{
		ERR("Fragment shader %s creation failed!", i_FragmentFileName.c_str());
		return false;
	}

	m_ShaderProgramID = glCreateProgram();
	glAttachShader(m_ShaderProgramID, m_VertexShaderID);
	glAttachShader(m_ShaderProgramID, m_FragmentShaderID);

	return true;
}

bool ShaderManager::CreateRenderingProgram ( const std::string& i_VertexFileName, const std::string& i_GeometryFileName, const std::string& i_FragmentFileName, const GlobalConfig& i_Config )
{
	if (i_VertexFileName.empty() || i_GeometryFileName.empty() || i_FragmentFileName.empty() )
	{
		ERR("Empty shader file name!");
		return false;
	}

	if (!CreateShader(i_VertexFileName, GL_VERTEX_SHADER, i_Config))
	{
		ERR("Vertex shader %s creation failed!", i_VertexFileName.c_str());
		return false;
	}

	if (!CreateShader(i_GeometryFileName, GL_GEOMETRY_SHADER, i_Config))
	{
		ERR("Geometry shader %s creation failed!", i_GeometryFileName.c_str());
		return false;
	}

	if (!CreateShader(i_FragmentFileName, GL_FRAGMENT_SHADER, i_Config))
	{
		ERR("Fragment shader %s creation failed!", i_FragmentFileName.c_str());
		return false;
	}

	m_ShaderProgramID = glCreateProgram();
	glAttachShader(m_ShaderProgramID, m_VertexShaderID);
	glAttachShader(m_ShaderProgramID, m_GeometryShaderID);
	glAttachShader(m_ShaderProgramID, m_FragmentShaderID);

	return true;
}

bool ShaderManager::CreateComputeProgram ( const std::string& i_ComputeFileName, const GlobalConfig& i_Config )
{
	if (i_ComputeFileName.empty())
	{
		ERR("Empty compute shader file name!");
		return false;
	}

	if (!CreateShader(i_ComputeFileName, GL_COMPUTE_SHADER, i_Config))
	{
		ERR("Compute shader %s creation failed!", i_ComputeFileName.c_str());
		return false;
	}

	m_ShaderProgramID = glCreateProgram();
	glAttachShader(m_ShaderProgramID, m_ComputeShaderID);

	return true;
}

bool ShaderManager::CreateShader ( const std::string& i_ShaderFileName, unsigned int i_ShaderType, const GlobalConfig& i_Config )
{
	FileUtils::TextFile shaderSource;

	if (!FileUtils::LoadTextFile(i_ShaderFileName, shaderSource))
	{
		ERR("Invalid shader source file!");
		return false;
	}

	const short kSize = 3;
	const char* shaderContent[kSize] = { nullptr };
	int shaderContentLengths[kSize] = { 0 };

	//// TODO improve this code
	std::string computeOptions;
	if (i_ShaderType == GL_COMPUTE_SHADER && i_Config.GLExtVars.IsComputeShaderSupported)
	{
		std::stringstream ss;
		ss << "#extension GL_ARB_compute_shader : require\n"
			<< "#extension GL_ARB_shader_image_load_store : require\n"
			<< "#extension GL_EXT_shader_image_load_store : require\n"
			<< "#extension GL_ARB_shading_language_420pack : require\n"
			<< "#extension GL_ARB_arrays_of_arrays : require\n"
			<< "#extension GL_ARB_enhanced_layouts : require\n";

		computeOptions = ss.str();
	}
	////
	std::string optionsString = computeOptions + i_Config.ShaderDefines.GetOptionsString();

	shaderContent[0] = i_Config.ShaderDefines.Header.c_str();
	shaderContent[1] = optionsString.c_str();
	shaderContent[2] = shaderSource.text.c_str();

	// NOTE! This shouldn't be necessary, because all kSize strings are null terminated
	//, but I use it anyway as a precaution!
	shaderContentLengths[0] = i_Config.ShaderDefines.Header.length();
	shaderContentLengths[1] = optionsString.length();
	shaderContentLengths[2] = shaderSource.text.length();

	unsigned int shaderID = glCreateShader(i_ShaderType);

	glShaderSource(shaderID, kSize, shaderContent, shaderContentLengths);

	int isCompiled = 0;
	glCompileShader(shaderID);
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
	if (!isCompiled)
	{
		int logLength = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);

		char* pLog = new char[logLength];
		assert(pLog != nullptr);
		if (!pLog)
		{
			glDeleteShader(shaderID);
			ERR("Invalid log data!");
			return false;
		}

		int charsWritten = 0;
		glGetShaderInfoLog(shaderID, logLength, &charsWritten, pLog);
		if (i_ShaderType == GL_VERTEX_SHADER)
		{
			ERR("Vertex shader compile error:");
		}
		else if (i_ShaderType == GL_GEOMETRY_SHADER)
		{
			ERR("Geometry shader compile error:");
		}
		else if (i_ShaderType == GL_FRAGMENT_SHADER)
		{
			ERR("Fragment shader compile error:");
		}
		else if (i_ShaderType == GL_COMPUTE_SHADER)
		{
			ERR("Compute shader compile error:");
		}
		ERR(i_ShaderFileName.c_str());
		ERR(pLog);

		SAFE_ARRAY_DELETE(pLog);
		glDeleteShader(shaderID);
		return false;
	}

	if (i_ShaderType == GL_VERTEX_SHADER) m_VertexShaderID = shaderID;
	if (i_ShaderType == GL_GEOMETRY_SHADER) m_GeometryShaderID = shaderID;
	if (i_ShaderType == GL_FRAGMENT_SHADER) m_FragmentShaderID = shaderID;
	if (i_ShaderType == GL_COMPUTE_SHADER) m_ComputeShaderID = shaderID;

	return true;
}

bool ShaderManager::LinkProgram ( void )
{
	if (!m_ShaderProgramID)
    {
    	ERR("Invalid program when linking!");
        return false;
    }

	int isLinked = 0;
	glLinkProgram(m_ShaderProgramID);
	glGetProgramiv(m_ShaderProgramID, GL_LINK_STATUS, &isLinked);

	if (!isLinked)
    {
		int logLength = 0;
		glGetProgramiv(m_ShaderProgramID, GL_INFO_LOG_LENGTH, &logLength);

        char* pLog = new char[logLength];
		assert(pLog != nullptr);
		if (!pLog)
        {
			glDeleteProgram(m_ShaderProgramID);
			m_ShaderProgramID = 0;
        	ERR("Invalid log data!");
            return false;
        }

		int charsWritten = 0;
		glGetProgramInfoLog(m_ShaderProgramID, logLength, &charsWritten, pLog);
		ERR("Shader program link error!");
		ERR(pLog);

		SAFE_ARRAY_DELETE(pLog);
		glDeleteProgram(m_ShaderProgramID);
		m_ShaderProgramID = 0;
 
        return false;
    }

	if (m_UseStrictVerification)
	{
		int isValid = 0;
		glValidateProgram(m_ShaderProgramID);
		glGetProgramiv(m_ShaderProgramID, GL_VALIDATE_STATUS, &isValid);

		if (!isValid)
		{
			int logLength = 0;
			glGetProgramiv(m_ShaderProgramID, GL_INFO_LOG_LENGTH, &logLength);

			char* pLog = new char[logLength];
			assert(pLog != nullptr);
			if (!pLog)
			{
				glDeleteProgram(m_ShaderProgramID);
				m_ShaderProgramID = 0;
				ERR("Invalid log data!");
				return false;
			}

			int charsWritten = 0;
			glGetProgramInfoLog(m_ShaderProgramID, logLength, &charsWritten, pLog);
			ERR("Shader program is NOT valid!");
			ERR(pLog);

			SAFE_ARRAY_DELETE(pLog);
			glDeleteProgram(m_ShaderProgramID);
			m_ShaderProgramID = 0;

			return false;
		}
	}

    return true;
}

void ShaderManager::UseProgram ( void ) const
{
	glUseProgram(m_ShaderProgramID);
}

GLvoid ShaderManager::UnUseProgram ( void ) const
{
	glUseProgram(0);
}

unsigned int ShaderManager::GetProgramID ( void ) const
{
	return m_ShaderProgramID;
}

int ShaderManager::GetAttributeLocation ( const char* i_pAttributeName ) const
{
	if (m_UseStrictVerification)
	{
		if (!m_ShaderProgramID)
		{
			ERR("Invalid program when getting attribute location!");
			return -1;
		}

		if (!i_pAttributeName)
		{
			ERR("Invalid attribute name!");
			return -1;
		}
	}

	return glGetAttribLocation(m_ShaderProgramID, i_pAttributeName);
}

int ShaderManager::GetUniformLocation ( const char* i_pUniformName ) const
{
	if (m_UseStrictVerification)
	{
		if (!m_ShaderProgramID)
		{
			ERR("Invalid program when getting uniform location!");
			return -1;
		}

		if (!i_pUniformName)
		{
			ERR("Invalid uniform name!");
			return -1;
		}
	}

	return glGetUniformLocation(m_ShaderProgramID, i_pUniformName);
}

void ShaderManager::SetUniform ( int i_UniformLocation, int i_UniformValue ) const
{
	if (m_UseStrictVerification)
	{
		if (!m_ShaderProgramID)
		{
			ERR("Invalid program when setting uniform value!");
			return;
		}

		if (i_UniformLocation == -1)
		{
			ERR("Uniform has a invalid location!");
			return;
		}
	}

	glUniform1i(i_UniformLocation, i_UniformValue);
}

void ShaderManager::SetUniform ( int i_UniformLocation, float i_UniformValue ) const
{
	if (m_UseStrictVerification)
	{
		if (!m_ShaderProgramID)
		{
			ERR("Invalid program when setting uniform value!");
			return;
		}

		if (i_UniformLocation == -1)
		{
			ERR("Uniform has a invalid location!");
			return;
		}
	}

	glUniform1f(i_UniformLocation, i_UniformValue);
}

void ShaderManager::SetUniform ( int i_UniformLocation, int i_ValueCount, const float* i_pUniformValue, ShaderManager::UNIFORM_TYPE i_UniformType ) const
{
	if (m_UseStrictVerification)
	{
		if (!m_ShaderProgramID)
		{
			ERR("Invalid program when setting uniform value!");
			return;
		}

		if (i_UniformLocation == -1)
		{
			ERR("Uniform has a invalid location!");
			return;
		}

		if (i_ValueCount <= 0.0f)
		{
			ERR("Invalid value count!");
			return;
		}

		if (!i_pUniformValue)
		{
			ERR("Invalid uniform value!");
			return;
		}
	}

	switch (i_UniformType)
	{
		case UNIFORM_TYPE::UT_FLOAT_VEC_2: glUniform2fv(i_UniformLocation, i_ValueCount, i_pUniformValue); break;
		case UNIFORM_TYPE::UT_FLOAT_VEC_3: glUniform3fv(i_UniformLocation, i_ValueCount, i_pUniformValue); break;
		case UNIFORM_TYPE::UT_FLOAT_VEC_4: glUniform4fv(i_UniformLocation, i_ValueCount, i_pUniformValue); break;
		case UNIFORM_TYPE::UT_FLOAT_MAT_2: glUniformMatrix2fv(i_UniformLocation, i_ValueCount, GL_FALSE, i_pUniformValue); break;
		case UNIFORM_TYPE::UT_FLOAT_MAT_3: glUniformMatrix3fv(i_UniformLocation, i_ValueCount, GL_FALSE, i_pUniformValue); break;
		case UNIFORM_TYPE::UT_FLOAT_MAT_4: glUniformMatrix4fv(i_UniformLocation, i_ValueCount, GL_FALSE, i_pUniformValue); break;
	}

}

void ShaderManager::SetupFragmentOutputStreams ( unsigned short i_LayerCount, unsigned short i_Stride ) const
{
	// setup output streams for fragment shader
	if (m_ShaderProgramID > 0)
	{
		for (unsigned short i = 0; i < i_LayerCount; ++i)
		{
			std::stringstream ss;
			ss << "fragColor" << i;
			std::string outputColor = ss.str();
			glBindFragDataLocation(m_ShaderProgramID, i_Stride + i, outputColor.c_str());
		}
	}
}