/* Author: BAIRAC MIHAI */

#include "Base2DIFFT.h"
#include <sstream> // std::stringstream
#include "Common.h"
#include "ErrorHandler.h"
#include "GL/glew.h"
#include "glm/exponential.hpp" //log2()
#include "GlobalConfig.h"


Base2DIFFT::Base2DIFFT( void )
	: m_NumButterflies(0), m_FFTSize(0), m_FFTLayerCount(0), m_UseFFTSlopes(false)
{}

Base2DIFFT::Base2DIFFT(const GlobalConfig& i_Config )
  : m_NumButterflies(0), m_FFTSize(0), m_FFTLayerCount(0), m_UseFFTSlopes(false)
{
	Initialize(i_Config);
}

Base2DIFFT::~Base2DIFFT( void )
{
	Destroy();
}

void Base2DIFFT::Destroy ( void )
{
	// should free resources

	LOG("Base2DIFFT has been destroyed successfully!");
}

void Base2DIFFT::Initialize(const GlobalConfig& i_Config)
{
	m_FFTSize = i_Config.Scene.Ocean.Surface.OceanPatch.FFTSize;
	m_UseFFTSlopes = i_Config.Scene.Ocean.Surface.OceanPatch.ComputeFFT.UseFFTSlopes;

	// the FFT algorithm need log(n) base 2 steps
	m_NumButterflies = static_cast<unsigned short>(glm::log2(static_cast<float>(m_FFTSize)));

	LOG("Base2DIFFT has been created successfully!");
}

void Base2DIFFT::Perform2DIFFT ( void )
{
	//stub
}

void Base2DIFFT::BindDestinationTexture ( void ) const
{
	//stub
}

unsigned short Base2DIFFT::GetFFTLayerCount (void) const
{
	return m_FFTLayerCount;
}

bool Base2DIFFT::GetUseFFTSlopes (void) const
{
	return m_UseFFTSlopes;
}

unsigned int Base2DIFFT::GetSourceTexId (void) const
{
	// stub
	return 0;
}

unsigned short Base2DIFFT::GetSourceTexUnitId (void) const
{
	// stub
	return 0;
}

unsigned int Base2DIFFT::GetDestinationTexId ( void ) const
{
	// stub
	return 0;
}

unsigned short Base2DIFFT::GetDestinationTexUnitId (void) const
{
	// stub
	return 0;
}