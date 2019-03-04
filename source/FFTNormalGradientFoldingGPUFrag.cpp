
/* Author: BAIRAC MIHAI */

#include "FFTNormalGradientFoldingGPUFrag.h"
#include "GL/glew.h"
#include "glm/vec4.hpp"
#include "glm/vector_relational.hpp" //any(), notEqual()
//#define ENABLE_ERROR_CHECK
#include "ErrorHandler.h"
#include "GlobalConfig.h"


FFTNormalGradientFoldingGPUFrag::FFTNormalGradientFoldingGPUFrag ( void )
{
	
}

FFTNormalGradientFoldingGPUFrag::FFTNormalGradientFoldingGPUFrag ( const GlobalConfig& i_Config )
{
	Initialize(i_Config);
}

FFTNormalGradientFoldingGPUFrag::~FFTNormalGradientFoldingGPUFrag ( void )
{
	Destroy();
}

void FFTNormalGradientFoldingGPUFrag::Destroy ( void )
{
	// should free resources

	LOG("FFTNormalGradientFoldingGPUFrag has been destroyed successfully!");
}

void FFTNormalGradientFoldingGPUFrag::Initialize ( const GlobalConfig& i_Config )
{
	FFTNormalGradientFoldingBase::Initialize(i_Config);

	m_FFTSize = i_Config.Scene.Ocean.Surface.OceanPatch.FFTSize;

	m_SM.Initialize("FFTNormalGradientFoldingGPUFrag");
	m_SM.BuildRenderingProgram("../resources/shaders/Quad.vert.glsl", "../resources/shaders/FFTNormalGradientFolding.frag.glsl", i_Config);

	m_SM.UseProgram();

	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> attributes;
	attributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_SM.GetAttributeLocation("a_position");
	attributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV] = m_SM.GetAttributeLocation("a_uv");

	m_Uniforms["u_FFTWaveDataMap"] = m_SM.GetUniformLocation("u_FFTWaveDataMap");

	m_Uniforms["u_FFTSize"] = m_SM.GetUniformLocation("u_FFTSize");
	m_SM.SetUniform(m_Uniforms.find("u_FFTSize")->second, static_cast<float>(m_FFTSize));

	m_Uniforms["u_PatchSize"] = m_SM.GetUniformLocation("u_PatchSize");
	m_SM.SetUniform(m_Uniforms.find("u_PatchSize")->second, i_Config.Scene.Ocean.Surface.OceanPatch.PatchSize);

	m_Uniforms["u_ChoppyScale"] = m_SM.GetUniformLocation("u_ChoppyScale");
	m_SM.SetUniform(m_Uniforms.find("u_ChoppyScale")->second, i_Config.Scene.Ocean.Surface.OceanPatch.ChoppyScale);

	m_Uniforms["u_CoverageFactor"] = m_SM.GetUniformLocation("u_CoverageFactor");
	m_SM.SetUniform(m_Uniforms.find("u_CoverageFactor")->second, i_Config.Scene.Ocean.Surface.Foam.CoverageFactor);

	m_SM.UnUseProgram();

	//////////
	m_FBM.Initialize("FFTNormalGradientFoldingGPUFrag");
	// the normal map also contains the folding factor, so for it to be spread also farther away from the camera position we need mipmaps!
	// 3 levels of mipmaps will suffice, because foam is needed more where fft waves are localized, and those are only near camera!
	m_FBM.CreateSimple(attributes, 1, GL_RGBA16F, GL_RGBA, GL_FLOAT, m_FFTSize, m_FFTSize, GL_REPEAT, GL_LINEAR, i_Config.TexUnit.Ocean.FFTNormalGradientFoldingBase.NormalGradientFoldingMap, 3);

	LOG("FFTNormalGradientFoldingGPUFrag has been created successfully!");
}

void FFTNormalGradientFoldingGPUFrag::ComputeNormalGradientFolding ( void )
{
	// save current viewport
	glm::ivec4 oldViewport;
	glm::ivec4 newViewport(0, 0, m_FFTSize, m_FFTSize);

	glGetIntegerv(GL_VIEWPORT, &oldViewport[0]);


	m_FBM.Bind();

	// set new viewport
	if (glm::any(glm::notEqual(newViewport, oldViewport)))
	{
		glViewport(newViewport.x, newViewport.y, newViewport.z, newViewport.w);
	}

	//// UPDATE Normal Gradient and Folding 
	m_SM.UseProgram();

	m_FBM.RenderToQuad();

	m_FBM.UnBind();

	// restore viewport
	if (glm::any(glm::notEqual(newViewport, oldViewport)))
	{
		glViewport(oldViewport.x, oldViewport.y, oldViewport.z, oldViewport.w);
	}
	//////////////////////////////////////////////
}

void FFTNormalGradientFoldingGPUFrag::BindTexture ( void ) const
{
	// as explained in the init function we need mipmaps for the folding factor that is used in simulating foam!
	m_FBM.BindColorAttachmentByIndex(0, true);
}

unsigned short FFTNormalGradientFoldingGPUFrag::GetTexUnitId ( void ) const
{
	return m_FBM.GetColorAttachmentTexUnitId(0);
}

void FFTNormalGradientFoldingGPUFrag::LinkSourceTex ( unsigned int i_SourceTex )
{
	// NOTE! In this case i_SourceTex = tex unit id
	m_SM.UseProgram();
	m_SM.SetUniform(m_Uniforms.find("u_FFTWaveDataMap")->second, static_cast<unsigned short>(i_SourceTex));
	m_SM.UnUseProgram();
}