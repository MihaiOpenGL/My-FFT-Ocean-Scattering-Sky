
/* Author: BAIRAC MIHAI */

#include "FFTNormalGradientFoldingBase.h"
//#define ENABLE_ERROR_CHECK
#include "ErrorHandler.h"
#include "GlobalConfig.h"


FFTNormalGradientFoldingBase::FFTNormalGradientFoldingBase ( void )
	: m_FFTSize(0)
{
}

FFTNormalGradientFoldingBase::FFTNormalGradientFoldingBase ( const GlobalConfig& i_Config )
	: m_FFTSize(0)
{
	Initialize(i_Config);
}

FFTNormalGradientFoldingBase::~FFTNormalGradientFoldingBase ( void )
{
	Destroy();
}

void FFTNormalGradientFoldingBase::Destroy ( void )
{
	// should free resources

	LOG("FFTNormalGradientFoldingBase has been destroyed successfully!");
}

void FFTNormalGradientFoldingBase::Initialize ( const GlobalConfig& i_Config )
{
	LOG("FFTNormalGradientFoldingBase has been created successfully!");
}

void FFTNormalGradientFoldingBase::ComputeNormalGradientFolding ( void )
{
	// stub
}

void FFTNormalGradientFoldingBase::BindTexture ( void ) const
{
	// stub
}

unsigned short FFTNormalGradientFoldingBase::GetTexUnitId ( void ) const
{
	// stub
	return 0;
}

void FFTNormalGradientFoldingBase::LinkSourceTex ( unsigned int i_SourceTex )
{
	// stub
}

void FFTNormalGradientFoldingBase::SetPatchSize ( unsigned short i_PatchSize )
{
	m_SM.UseProgram();
	m_SM.SetUniform(m_Uniforms.find("u_PatchSize")->second, i_PatchSize);
	m_SM.UnUseProgram();
}

void FFTNormalGradientFoldingBase::SetChoppyScale ( float i_ChoppyScale )
{
	m_SM.UseProgram();
	m_SM.SetUniform(m_Uniforms.find("u_ChoppyScale")->second, i_ChoppyScale);
	m_SM.UnUseProgram();
}