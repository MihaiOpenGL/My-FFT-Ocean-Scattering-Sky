
/* Author: BAIRAC MIHAI */


#include "FFTNormalGradientFoldingGPUComp.h"
#include "CommonHeaders.h"
#include "GLConfig.h"
#include "GlobalConfig.h"


FFTNormalGradientFoldingGPUComp::FFTNormalGradientFoldingGPUComp ( void )
	: m_TexId(0), m_SourceTexId(0)
{
	LOG("FFTNormalGradientFoldingGPUComp successfully created!");
}

FFTNormalGradientFoldingGPUComp::FFTNormalGradientFoldingGPUComp ( const GlobalConfig& i_Config )
	: m_TexId(0), m_SourceTexId(0)
{
	Initialize(i_Config);
}

FFTNormalGradientFoldingGPUComp::~FFTNormalGradientFoldingGPUComp ( void )
{
	Destroy();
}

void FFTNormalGradientFoldingGPUComp::Destroy ( void )
{
	// should free resources

	LOG("FFTNormalGradientFoldingGPUComp successfully destroyed!");
}

void FFTNormalGradientFoldingGPUComp::Initialize ( const GlobalConfig& i_Config )
{
	FFTNormalGradientFoldingBase::Initialize(i_Config);

	m_FFTSize = i_Config.Scene.Ocean.Surface.OceanPatch.FFTSize;

	m_SM.Initialize("FFTNormalGradientFoldingGPUComp");
	m_SM.BuildComputeProgram("resources/shaders/FFTNormalGradientFolding.comp.glsl", i_Config);

	m_SM.UseProgram();
	m_Uniforms["u_FFTSize"] = m_SM.GetUniformLocation("u_FFTSize");
	m_SM.SetUniform(m_Uniforms.find("u_FFTSize")->second, m_FFTSize);

	m_Uniforms["u_PatchSize"] = m_SM.GetUniformLocation("u_PatchSize");
	m_SM.SetUniform(m_Uniforms.find("u_PatchSize")->second, i_Config.Scene.Ocean.Surface.OceanPatch.PatchSize);

	m_Uniforms["u_ChoppyScale"] = m_SM.GetUniformLocation("u_ChoppyScale");
	m_SM.SetUniform(m_Uniforms.find("u_ChoppyScale")->second, i_Config.Scene.Ocean.Surface.OceanPatch.ChoppyScale);

	m_Uniforms["u_CoverageFactor"] = m_SM.GetUniformLocation("u_CoverageFactor");
	m_SM.SetUniform(m_Uniforms.find("u_CoverageFactor")->second, i_Config.Scene.Ocean.Surface.Foam.CoverageFactor);

	m_SM.UnUseProgram();

	//////////
	// the normal map also contains the folding factor, so for it to be spread also farther away from the camera position we need mipmaps!
	// 3 levels of mipmaps will suffice, because foam is needed more where fft waves are localized, and those are only near camera!
	m_TM.Initialize("FFTNormalGradientFoldingGPUComp", i_Config);
	m_TexId = m_TM.Create2DTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT, m_FFTSize, m_FFTSize, GL_REPEAT, GL_LINEAR, nullptr, i_Config.TexUnit.Ocean.FFTNormalGradientFoldingBase.NormalGradientFoldingMap, 3);

	LOG("FFTNormalGradientFoldingGPUComp successfully created!");
}

void FFTNormalGradientFoldingGPUComp::ComputeNormalGradientFolding (void)
{
	m_SM.UseProgram();

	// input
	glBindImageTexture(0, m_SourceTexId, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16F);

	// output
	glBindImageTexture(1, m_TexId, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

	// Process all vertices. No synchronization needed, so start NxN threads with local size of 1x1.
	glDispatchCompute(m_FFTSize, m_FFTSize, 1);

	// Make sure, all values are written.
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}


void FFTNormalGradientFoldingGPUComp::BindTexture ( void ) const
{
	// as explained in the init function we need mipmaps for the folding factor that is used in simulating foam!
	m_TM.BindTexture(m_TexId, true);
}

unsigned short FFTNormalGradientFoldingGPUComp::GetTexUnitId ( void ) const
{
	return m_TM.GetTextureUnitId(0);
}

void FFTNormalGradientFoldingGPUComp::LinkSourceTex ( unsigned int i_SourceTex )
{
	// NOTE! In this case i_SourceTex = tex id
	m_SourceTexId = i_SourceTex;
}