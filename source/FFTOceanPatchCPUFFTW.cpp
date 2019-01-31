/* Author: BAIRAC MIHAI */

#include "FFTOceanPatchCPUFFTW.h"
#include <sstream> // std::stringstream
#include <time.h>
#include "GL/glew.h"
#include "glm/gtc/constants.hpp" //pi()
#include "glm/gtc/type_ptr.hpp" //value_ptr()
#include "Common.h"
#include "ErrorHandler.h"
#include "FileUtils.h"
#include "GlobalConfig.h"
#include "FFTNormalGradientFoldingBase.h"


FFTOceanPatchCPUFFTW::FFTOceanPatchCPUFFTW ( void )
	: m_pFFTDisplaymentData(nullptr)
{
}

FFTOceanPatchCPUFFTW::FFTOceanPatchCPUFFTW ( const GlobalConfig& i_Config )
	: m_pFFTDisplaymentData(nullptr)
{
	Initialize(i_Config);
}

FFTOceanPatchCPUFFTW::~FFTOceanPatchCPUFFTW ( void )
{
	Destroy();
}

void FFTOceanPatchCPUFFTW::Destroy ( void )
{
	// should free resources
	SAFE_ARRAY_DELETE(m_pFFTDisplaymentData);

	LOG("FFTOceanPatchCPUFFTW has been destroyed successfully!");
}

void FFTOceanPatchCPUFFTW::Initialize ( const GlobalConfig& i_Config )
{
	FFTOceanPatchBase::Initialize(i_Config);

	///////////////
	m_2DIFFT.Initialize(i_Config);

	////////// Initialize FFT Data /////////
	m_FFTInitData.resize(m_FFTSize * m_FFTSize);

	InitFFTData();

	m_pFFTDisplaymentData = new float[m_FFTSize * m_FFTSize * 4 * sizeof(float)];
	assert(m_pFFTDisplaymentData != nullptr);

	/////////// NORMAL, FOLDING SETUP ///////////
	if (i_Config.Scene.Ocean.Surface.OceanPatch.NormalGradientFolding.Type == CustomTypes::Ocean::NormalGradientFoldingType::NGF_GPU_FRAG)
	{
		if (m_pNormalGradientFolding)
		{
			m_pNormalGradientFolding->LinkSourceTex(m_2DIFFT.GetDestinationTexUnitId());
		}
	}
	else if (i_Config.Scene.Ocean.Surface.OceanPatch.NormalGradientFolding.Type == CustomTypes::Ocean::NormalGradientFoldingType::NGF_GPU_COMP)
	{
		if (m_pNormalGradientFolding)
		{
			m_pNormalGradientFolding->LinkSourceTex(m_2DIFFT.GetDestinationTexId());
		}
	}

	LOG("FFTOceanPatchCPUFFTW has been created successfully!");
}

void FFTOceanPatchCPUFFTW::SetFFTData ( void )
{
	InitFFTData();
}

void FFTOceanPatchCPUFFTW::InitFFTData ( void )
{
	srand(0);

	glm::vec2 waveVector(0.0f);
	float fPatchSize = static_cast<float>(m_PatchSize);
	float min = glm::pi<float>() / m_PatchSize;
	for (unsigned short i = 0; i < m_FFTSize; ++ i)
	{
		waveVector.y = glm::pi<float>() * (2.0f * i - m_FFTSize) / fPatchSize;

		for (unsigned short j = 0; j < m_FFTSize; ++ j)
		{
			waveVector.x = glm::pi<float>() * (2.0f * j - m_FFTSize) / fPatchSize;

			unsigned int index = i * m_FFTSize + j;

			if (glm::abs(waveVector.x) < min && glm::abs(waveVector.y) < min)
			{
				// values already initialzied to zero
			}
			else
			{
				m_FFTInitData[index].hTilde0 = HTilde0(waveVector);
				m_FFTInitData[index].hTilde0Conj = std::conj(HTilde0(- waveVector));
				m_FFTInitData[index].dispersionFrequency = DispersionFrequency(waveVector);
			}
		}
	}
}

void FFTOceanPatchCPUFFTW::EvaluateWaves ( float i_CrrTime )
{
	/////// UPDATE HEIGHTMAP

	unsigned int index = 0;

	////////// Init Data for FFT / Pre FFT calc

	float waveVectorLength; glm::vec2 waveVector(0.0f);
	float fPatchSize = static_cast<float>(m_PatchSize);
	std::complex<float> DX, DY, DZ, SX, SZ;
	for (unsigned short i = 0; i < m_FFTSize; ++i)
	{
		waveVector.y = glm::pi<float>() * (2.0f * i - m_FFTSize) / m_PatchSize;
		for (unsigned short j = 0; j < m_FFTSize; ++j)
		{
			waveVector.x = glm::pi<float>() * (2.0f * j - m_FFTSize) / m_PatchSize;
			waveVectorLength = glm::length(waveVector);

			index = i * m_FFTSize + j;

			DY = HTilde(index, i_CrrTime);

			DX = (waveVectorLength < 0.000001f ? 0.0f : (DY * std::complex<float>(0.0f, -waveVector.x / waveVectorLength)));
			DZ = (waveVectorLength < 0.000001f ? 0.0f : (DY * std::complex<float>(0.0f, -waveVector.y / waveVectorLength)));

			if (m_2DIFFT.GetUseFFTSlopes())
			{
				SX = DY * std::complex<float>(0.0f, waveVector.x);
				SZ = DY * std::complex<float>(0.0f, waveVector.y);
			}

			m_2DIFFT.Pre2DFFTSetup(DX, DY, DZ, SX, SZ, index);
		}
	}

	//// PERFORM 2D Inverse FFT
	m_2DIFFT.Perform2DIFFT();

	/////////// Correct FFT Data / Post FFT calc
	short sign = 0;

	const short k_signs[] = { 1, -1 };
	for (unsigned short i = 0; i < m_FFTSize; ++i)
	{
		for (unsigned short j = 0; j < m_FFTSize; ++j)
		{
			index = i * m_FFTSize + j;
			sign = k_signs[(i + j) & 1];

			//sign correction
			m_2DIFFT.Post2DFFTSetup(sign, index);
		}
	}

	////////// Update the fft data texture
	m_2DIFFT.UpdateTextureData();

	FFTOceanPatchBase::EvaluateWaves(i_CrrTime);
}

std::complex<float> FFTOceanPatchCPUFFTW::HTilde0 ( const glm::vec2& i_WaveVector )
{
	// Ec. (25) from Jerry Tessendorf's article

	float specFactor = 1.0f;

	//complex<float> r = GaussianRandomVariable(); // r = Xr + i * Xi
	if (m_SpectrumType == CustomTypes::Ocean::SpectrumType::ST_PHILLIPS)
	{
		specFactor = glm::sqrt(PhillipsSpectrum(i_WaveVector) / 2.0f);
	}
	else if (m_SpectrumType == CustomTypes::Ocean::SpectrumType::ST_UNIFIED)
	{
		specFactor = glm::sqrt(UnifiedSpectrum(i_WaveVector) / 2.0f) * glm::two_pi<float>() / m_PatchSize;
	}

	glm::vec2 res = GaussianRandomVariable() * specFactor;

	return std::complex<float>(res.x, res.y);
}

std::complex<float> FFTOceanPatchCPUFFTW::HTilde ( unsigned int i_Index, float i_CrrTime )
{

	// w(k) * t
	float omegat = m_FFTInitData[i_Index].dispersionFrequency * i_CrrTime;

	// Euler formula: http://en.wikipedia.org/wiki/Euler%27s_formula
	// exp(i * x) = cos(x) + i * sin(x)
	
	float cos_ = glm::cos(omegat);
	float sin_ = glm::sin(omegat);

	// eq (26) from Jerry Tessendorf's paper
	return m_FFTInitData[i_Index].hTilde0 * std::complex<float>(cos_, sin_) + m_FFTInitData[i_Index].hTilde0Conj * std::complex<float>(cos_, -sin_);
}

float FFTOceanPatchCPUFFTW::ComputeWaterHeightAt ( const glm::vec2& i_XZ ) const
{
	float waterHeight = 0.0f;

	if (m_pFFTDisplaymentData)
	{
		//// obtain the height data from the FFT texture (displacement is available at layer 0)
		m_2DIFFT.BindDestinationTexture();
		glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, GL_FLOAT, m_pFFTDisplaymentData);

		//// compute the water height by interploating the nearest 4 neighbors from the FFT displayament texture at the given (x, z) data space position
		// Convert coords from world-space to data-space
		int x = static_cast<int>(i_XZ.x) % m_FFTSize,
			z = static_cast<int>(i_XZ.y) % m_FFTSize;

		// If data-space coords are negative, transform it to positive
		if (i_XZ.x < 0.0f) x += (m_FFTSize - 1);
		if (i_XZ.y < 0.0f) z += (m_FFTSize - 1);

		// Adjust the index if coords are out of range
		int xx = (x == m_FFTSize - 1) ? -1 : x,
			zz = (z == m_FFTSize - 1) ? -1 : z;

		// the data we get from FFT data texture is an array of floats groups as xyzw values (4 floats)
		const int k_stride = 4;
		const int k_yOffset = 1; // y - has offset 1, because x offset = 0, z offset = 2, w offset = 3

								 // Determine x and y diff for linear interpolation
		int xINT = (i_XZ.x > 0.0f) ? static_cast<int>(i_XZ.x) : static_cast<int>(i_XZ.x - 1.0f),
			zINT = (i_XZ.y > 0.0f) ? static_cast<int>(i_XZ.y) : static_cast<int>(i_XZ.y - 1.0f);

		// Calculate interpolation coefficients
		float xDIFF = i_XZ.x - xINT,
			zDIFF = i_XZ.y - zINT,
			_xDIFF = 1.0f - xDIFF,
			_zDIFF = 1.0f - zDIFF;

		//   A      B
		//     
		//
		//   C      D
		//interpolate among 4 adjacent neighbors
		unsigned int indexA = k_stride * (z * m_FFTSize + x) + k_yOffset,
			indexB = k_stride * (z * m_FFTSize + (xx + 1)) + k_yOffset,
			indexC = k_stride * ((zz + 1) * m_FFTSize + x) + k_yOffset,
			indexD = k_stride * ((zz + 1) * m_FFTSize + (xx + 1)) + k_yOffset;

		waterHeight = m_pFFTDisplaymentData[indexA] * _xDIFF *_zDIFF + m_pFFTDisplaymentData[indexB] * xDIFF *_zDIFF + m_pFFTDisplaymentData[indexC] * _xDIFF * zDIFF + m_pFFTDisplaymentData[indexD] * xDIFF * zDIFF;
	}

	return waterHeight;
}

void FFTOceanPatchCPUFFTW::BindFFTWaveDataTexture ( void ) const
{
	m_2DIFFT.BindDestinationTexture();
}

unsigned short FFTOceanPatchCPUFFTW::GetFFTWaveDataTexUnitId ( void ) const
{
	return m_2DIFFT.GetDestinationTexUnitId();
}