/* Author: BAIRAC MIHAI */

#include "CPUFFTW2DIFFT.h"

#include "ErrorHandler.h"

#include "GL/glew.h"

#include <assert.h>

#include "GlobalConfig.h"


CPUFFTW2DIFFT::CPUFFTW2DIFFT ( void )
	: m_pDY(nullptr), m_pDX(nullptr), m_pDZ(nullptr), m_pSX(nullptr), m_pSZ(nullptr),
	  m_FFTDataTexId(0)
{}

CPUFFTW2DIFFT::CPUFFTW2DIFFT ( const GlobalConfig& i_Config )
	: m_pDY(nullptr), m_pDX(nullptr), m_pDZ(nullptr), m_pSX(nullptr), m_pSZ(nullptr),
	  m_FFTDataTexId(0)
{
	Initialize(i_Config);
}


CPUFFTW2DIFFT::~CPUFFTW2DIFFT ( void )
{
	Destroy();
}

void CPUFFTW2DIFFT::Initialize ( const GlobalConfig& i_Config )
{
	Base2DIFFT::Initialize(i_Config);

	// NOTE! for FFT slopes we need 2 layers, otherwise only 1 is needed!
	m_FFTLayerCount = (m_UseFFTSlopes ? 2 : 1);

	// Allocate memory for data structures used to compute 2D IFFT
	m_pDY = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * m_FFTSize * m_FFTSize);
	assert(m_pDY != nullptr);
	m_pDX = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * m_FFTSize * m_FFTSize);
	assert(m_pDX != nullptr);
	m_pDZ = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * m_FFTSize * m_FFTSize);
	assert(m_pDZ != nullptr);

	if (m_UseFFTSlopes)
	{
		m_pSX = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * m_FFTSize * m_FFTSize);
		assert(m_pSX != nullptr);
		m_pSZ = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * m_FFTSize * m_FFTSize);
		assert(m_pSZ != nullptr);
	}

	/*
	 We compute 2D IFFT - meaning an Inverse FFT on 2 dimensions: horizontally and vertically.
	 This approach is the right one, because we have a grid of points that will later be displaced by the IFFT data
	 To displace 2D geometry we need 2D FFT.FFT goes from time domain to frquency one, we need the other way around
	 meaning Inverse FFT - from frquency domain to time.Time domain being the proper domain to make a simulation in real - time
	 We work with complex numbers hence the 4d vectors : which hold 2 complex numbers : [r1, i1], [r2, i2]

	 fftw_plan_dft_2d() - allows us to create a 2d discrete FFT, FFTW_BACKWARD - denotes inverse FFT

	 In reality we must compute 3 IFFT: 1 - displacement along OY axis, 2 - displacement along OZ, 3- displacement along OZ axis
	 In case we use FFT slopes, we compute 2 IFFT more: 4 - slope along OX axis, 5 - slope along OZ axis
	*/

	// NOTE! The best flags are: FFTW_MEASURE, FFTW_PATIENT, FFTW_EXHAUSTIVE (increased time, but more optimal)
	// the fastest flag is FFTW_MEASURE, the most optim is FFTW_PATIENT (for this PC)
	m_PDY = fftw_plan_dft_2d(m_FFTSize, m_FFTSize, m_pDY, m_pDY, FFTW_BACKWARD, FFTW_MEASURE);
	m_PDX = fftw_plan_dft_2d(m_FFTSize, m_FFTSize, m_pDX, m_pDX, FFTW_BACKWARD, FFTW_MEASURE);
	m_PDZ = fftw_plan_dft_2d(m_FFTSize, m_FFTSize, m_pDZ, m_pDZ, FFTW_BACKWARD, FFTW_MEASURE);

	if (m_UseFFTSlopes)
	{
		m_PSX = fftw_plan_dft_2d(m_FFTSize, m_FFTSize, m_pSX, m_pSX, FFTW_BACKWARD, FFTW_MEASURE);
		m_PSZ = fftw_plan_dft_2d(m_FFTSize, m_FFTSize, m_pSZ, m_pSZ, FFTW_BACKWARD, FFTW_MEASURE);
	}

	////////////
	m_TM.Initialize("CPUFFTW2DIFFT");
	// NOTE! no need for more than 3 levels of mipmaps
	m_FFTDataTexId = m_TM.Create2DArrayTexture(m_FFTLayerCount, GL_RGBA16F, GL_RGBA, GL_FLOAT, m_FFTSize, m_FFTSize, GL_REPEAT, GL_LINEAR, nullptr, i_Config.TexUnit.Ocean.CPU2DIFFT.FFTMap, m_kMipmapCount);

	if (m_UseFFTSlopes)
	{
		m_FFTProcessedData.resize(m_FFTSize * m_FFTSize * 2);
	}
	else
	{
		m_FFTProcessedData.resize(m_FFTSize * m_FFTSize);
	}

	LOG("CPUFFTW2DIFFT has been created successfully!");
}

void CPUFFTW2DIFFT::Destroy ( void )
{
	if (m_PDY) fftw_destroy_plan(m_PDY);
	if (m_PDX) fftw_destroy_plan(m_PDX);
	if (m_PDZ) fftw_destroy_plan(m_PDZ);

	if (m_UseFFTSlopes)
	{
		if (m_PSX) fftw_destroy_plan(m_PSX);
		if (m_PSZ) fftw_destroy_plan(m_PSZ);
	}

	if (m_pDY) fftw_free(m_pDY); m_pDY = nullptr;
	if (m_pDX) fftw_free(m_pDX); m_pDX = nullptr;
	if (m_pDZ) fftw_free(m_pDZ); m_pDZ = nullptr;

	if (m_UseFFTSlopes)
	{
		fftw_free(m_pSX); m_pSX = nullptr;
		fftw_free(m_pSZ); m_pSZ = nullptr;
	}

	LOG("CPUFFTW2DIFFT has been destroyed successfully!");
}

void CPUFFTW2DIFFT::Pre2DFFTSetup ( const std::complex<float>& i_DX, const std::complex<float>& i_DY, const std::complex<float>& i_DZ, const std::complex<float>& i_SX, const std::complex<float>& i_SZ, unsigned int i_Index )
{
	assert(i_Index < (unsigned int)(m_FFTSize * m_FFTSize));

	if (m_pDY && m_pDX && m_pDZ)
	{
		m_pDY[i_Index][0] = i_DY.real();
		m_pDY[i_Index][1] = i_DY.imag();

		m_pDX[i_Index][0] = i_DX.real();
		m_pDX[i_Index][1] = i_DX.imag();

		m_pDZ[i_Index][0] = i_DZ.real();
		m_pDZ[i_Index][1] = i_DZ.imag();
	}

	if (m_UseFFTSlopes)
	{
		if (m_pSX && m_pSZ)
		{
			m_pSX[i_Index][0] = i_SX.real();
			m_pSX[i_Index][1] = i_SX.imag();

			m_pSZ[i_Index][0] = i_SZ.real();
			m_pSZ[i_Index][1] = i_SZ.imag();
		}
	}
}

void CPUFFTW2DIFFT::Perform2DIFFT ( void )
{
	// Compute 2D IFFT
	if (m_PDY) fftw_execute(m_PDY);
	if (m_PDX) fftw_execute(m_PDX);
	if (m_PDZ) fftw_execute(m_PDZ);

	if (m_UseFFTSlopes)
	{
		if (m_PSX) fftw_execute(m_PSX);
		if (m_PSZ) fftw_execute(m_PSZ);
	}
}

void CPUFFTW2DIFFT::Post2DFFTSetup ( short i_Sign, unsigned int i_Index )
{
	assert(i_Index < (unsigned int)(m_FFTSize * m_FFTSize));

	const short k_lambda = -1;
	short sign_correction = i_Sign * k_lambda;

	if (m_pDX && m_pDY && m_pDZ)
	{
		// 1st texture layer - displacement
		m_FFTProcessedData[i_Index].x = m_pDX[i_Index][0] * sign_correction;
		m_FFTProcessedData[i_Index].y = m_pDY[i_Index][0] * i_Sign;
		m_FFTProcessedData[i_Index].z = m_pDZ[i_Index][0] * sign_correction;
	}

	if (m_UseFFTSlopes)
	{
		if (m_pSX && m_pSZ)
		{
			// 2nd texture layer - slopes
			unsigned int offset = m_FFTSize * m_FFTSize;
			m_FFTProcessedData[i_Index + offset].x = m_pSX[i_Index][0] * sign_correction;
			m_FFTProcessedData[i_Index + offset].y = m_pSZ[i_Index][0] * sign_correction;
		}
	}
}

void CPUFFTW2DIFFT::BindDestinationTexture ( void ) const
{
	m_TM.BindTexture(m_FFTDataTexId, true);
}

void CPUFFTW2DIFFT::UpdateTextureData ( void )
{
	m_TM.Update2DArrayTextureData(m_FFTDataTexId, &m_FFTProcessedData[0]);
}

unsigned int CPUFFTW2DIFFT::GetDestinationTexId ( void ) const
{
	return m_TM.GetTextureId(0);
}

unsigned short CPUFFTW2DIFFT::GetDestinationTexUnitId ( void ) const
{
	return m_TM.GetTextureUnitId(0);
}