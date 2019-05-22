/* Author: BAIRAC MIHAI */

#include "FFTOceanPatchGPUComp.h"
#include "CommonHeaders.h"
#include "GLConfig.h"
#include "GLExtensionVars.h"
// glm::vec2, glm::vec4 come from the header
#include "glm/common.hpp" //abs()
#include "glm/gtc/constants.hpp" //pi(), wo_pi()
#include "glm/exponential.hpp" //sqrt()
#include "FileUtils.h"
#include "GlobalConfig.h"
#include "FFTNormalGradientFoldingBase.h"
#include <time.h>


FFTOceanPatchGPUComp::FFTOceanPatchGPUComp ( void )
	: m_FFTInitDataTexId(0), m_pFFTDisplaymentData(nullptr),
	  m_IsComputeShaderSupported(false)
{
	LOG("FFTOceanPatchGPUComp successfully created!");
}

FFTOceanPatchGPUComp::FFTOceanPatchGPUComp ( const GlobalConfig& i_Config )
	: m_FFTInitDataTexId(0), m_pFFTDisplaymentData(nullptr),
	  m_IsComputeShaderSupported(false)
{
	Initialize(i_Config);
}

FFTOceanPatchGPUComp::~FFTOceanPatchGPUComp ( void )
{
	Destroy();
}

void FFTOceanPatchGPUComp::Destroy ( void )
{
	// should free resources
	SAFE_ARRAY_DELETE(m_pFFTDisplaymentData);

	LOG("FFTOceanPatchGPUComp successfully destroyed!");
}

void FFTOceanPatchGPUComp::Initialize ( const GlobalConfig& i_Config )
{
	m_IsComputeShaderSupported = i_Config.GLExtVars.IsComputeShaderSupported;

	FFTOceanPatchBase::Initialize(i_Config);
	///////////////
	m_2DIFFT.Initialize(i_Config);


	////////// Initialize FFT Data /////////
	m_FFTInitData.resize(m_FFTSize * m_FFTSize);

	InitFFTData();

	//// Create H0Omega texture
	m_FFTTM.Initialize("FFTOceanPatchGPUComp", i_Config);
	m_FFTInitDataTexId = m_FFTTM.Create2DTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT, m_FFTSize, m_FFTSize, GL_REPEAT, GL_NEAREST, &m_FFTInitData[0], i_Config.TexUnit.Ocean.FFTOceanPatchGPUComp.FFTInitDataMap);

	m_pFFTDisplaymentData = new float[m_FFTSize * m_FFTSize * 4 * sizeof(float)];
	assert(m_pFFTDisplaymentData != nullptr);

	///////////// FFT Ht SETUP ///////////
	m_FFTHtSM.Initialize("FFTOceanPatchGPUComp");

	if (m_2DIFFT.GetUseFFTSlopes())
	{
		m_FFTHtSM.BuildComputeProgram("resources/shaders/FFTHt.comp.glsl", i_Config);
	}
	else
	{
		m_FFTHtSM.BuildComputeProgram("resources/shaders/FFTHt_NoFFTSlopes.comp.glsl", i_Config);
	}

	m_FFTHtSM.UseProgram();

	m_FFTHtUniforms["u_FFTSize"] = m_FFTHtSM.GetUniformLocation("u_FFTSize");
	m_FFTHtSM.SetUniform(m_FFTHtUniforms.find("u_FFTSize")->second, static_cast<float>(m_FFTSize));

	m_FFTHtUniforms["u_PatchSize"] = m_FFTHtSM.GetUniformLocation("u_PatchSize");
	m_FFTHtSM.SetUniform(m_FFTHtUniforms.find("u_PatchSize")->second, static_cast<float>(m_PatchSize));

	m_FFTHtUniforms["u_Time"] = m_FFTHtSM.GetUniformLocation("u_Time");

	m_FFTHtSM.UnUseProgram();

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

	LOG("FFTOceanPatchGPUComp successfully created!");
}

void FFTOceanPatchGPUComp::SetFFTData ( void )
{
	InitFFTData();

	m_FFTTM.Update2DTextureData(m_FFTInitDataTexId, &m_FFTInitData[0]);
}

void FFTOceanPatchGPUComp::InitFFTData ( void )
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
				m_FFTInitData[index].x = 0.0f;
				m_FFTInitData[index].y = 0.0f;
				m_FFTInitData[index].z = 0.0f;
			}
			else
			{
				float specFactor = 1.0f;

				if (m_SpectrumType == CustomTypes::Ocean::SpectrumType::ST_PHILLIPS)
				{
					specFactor = glm::sqrt(PhillipsSpectrum(waveVector) / 2.0f);
				}
				else if (m_SpectrumType == CustomTypes::Ocean::SpectrumType::ST_UNIFIED)
				{
					specFactor = glm::sqrt(UnifiedSpectrum(waveVector) / 2.0f) * glm::two_pi<float>() / m_PatchSize;
				}

				glm::vec2 hTilde0 = GaussianRandomVariable() * specFactor;
				m_FFTInitData[index].x = hTilde0.x;
				m_FFTInitData[index].y = hTilde0.y;
				m_FFTInitData[index].z = DispersionFrequency(waveVector);
			}
		}
	}
}

void FFTOceanPatchGPUComp::EvaluateWaves ( float i_CrrTime )
{
	//// UPDATE FFT Ht
	m_FFTHtSM.UseProgram();

	m_FFTHtSM.SetUniform(m_FFTHtUniforms.find("u_Time")->second, i_CrrTime);

	///////////
	if (m_IsComputeShaderSupported)
	{
		// input - H0
		glBindImageTexture(0, m_FFTInitDataTexId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);

		// output - Ht
		glBindImageTexture(1, m_2DIFFT.GetSourceTexId(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glBindImageTexture(1, m_2DIFFT.GetSourceTexId(), 0, GL_TRUE, 1, GL_WRITE_ONLY, GL_RGBA16F);
		if (m_2DIFFT.GetUseFFTSlopes())
		{
			glBindImageTexture(1, m_2DIFFT.GetSourceTexId(), 0, GL_TRUE, 2, GL_WRITE_ONLY, GL_RGBA16F);
		}

		// Process all vertices. No synchronization needed, so start NxN threads with local size of 1x1.
		glDispatchCompute(m_FFTSize, m_FFTSize, 1);

		// Make sure, all values are written.
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		/////////
	}
	//// PERFORM 2D Inverse FFT
	m_2DIFFT.Perform2DIFFT();

	/////
	FFTOceanPatchBase::EvaluateWaves(i_CrrTime);
}

float FFTOceanPatchGPUComp::ComputeWaterHeightAt ( const glm::vec2& i_XZ ) const
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

void FFTOceanPatchGPUComp::BindFFTWaveDataTexture ( void ) const
{
	m_2DIFFT.BindDestinationTexture();
}

unsigned short FFTOceanPatchGPUComp::GetFFTWaveDataTexUnitId ( void ) const
{
	return m_2DIFFT.GetDestinationTexUnitId();
}