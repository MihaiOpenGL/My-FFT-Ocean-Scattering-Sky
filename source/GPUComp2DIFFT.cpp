/* Author: BAIRAC MIHAI */

#include "GPUComp2DIFFT.h"
#include <sstream> // std::stringstream
#include "CommonHeaders.h"
#include "GLConfig.h"
#include "glm/exponential.hpp" //log2(), pow()
#include "glm/trigonometric.hpp" //cos(), sin() 
#include "glm/gtc/constants.hpp" //two_pi()
#include "GlobalConfig.h"


GPUComp2DIFFT::GPUComp2DIFFT ( void )
  : m_IndicesTexId(0), m_WeightsTexId(0), m_NumButterflies(0),
	m_IsComputeShaderSupported(false)
{
	LOG("GPUComp2DFFT successfully created!");
}

GPUComp2DIFFT::GPUComp2DIFFT ( const GlobalConfig& i_Config )
  : m_IndicesTexId(0), m_WeightsTexId(0), m_NumButterflies(0),
	m_IsComputeShaderSupported(false)
{
	Initialize(i_Config);
}

GPUComp2DIFFT::~GPUComp2DIFFT ( void )
{
	Destroy();
}

void GPUComp2DIFFT::Destroy ( void )
{
	// should free resources

	LOG("GPUComp2DFFT successfully destroyed!");
}

void GPUComp2DIFFT::Initialize ( const GlobalConfig& i_Config )
{
	Base2DIFFT::Initialize(i_Config);

	m_IsComputeShaderSupported = i_Config.GLExtVars.IsComputeShaderSupported;

	// For FFT Slopes we need 3 layers, otherwise only 2 are needed
	m_FFTLayerCount = (m_UseFFTSlopes ? 3 : 2);


	// the FFT algorithm need log(n) base 2 steps
	m_NumButterflies = static_cast<unsigned short>(glm::log2(static_cast<float>(m_FFTSize)));

	//////////////
	/*
	Now, the init data for FFT is being created.
	Based on the Cooley–Tukey algorithm: https://en.wikipedia.org/wiki/Cooley%E2%80%93Tukey_FFT_algorithm
	We compute 2D IFFT - meaning an Inverse FFT on 2 dimensions: horizontally and vertically.
	This approach is the right one, because we have a grid of points that will later be displaced by the IFFT data
	To displace 2D geometry we need 2D FFT. FFT goes from time domain to frquency one, we need the other way around
	meaning Inverse FFT - from frquency domain to time. Time domain being the proper domain to make a simulation in real-time
	We work with complex numbers hence the 4d vectors: which hold 2 complex numbers: [r1, i1], [r2, i2]
	*/

	m_TM.Initialize("GPUComp2DFFT", i_Config);

	///////// 
	// Create the pingpong textures first, so that they ids don't change in case we need to create other textures !!!
	for (unsigned short i = 0; i < m_kPingPongLayerCount; ++i)
	{
		m_PingPongTexIds[i] = m_TM.Create2DArrayTexture(m_FFTLayerCount, GL_RGBA16F, GL_RGBA, GL_FLOAT, m_FFTSize, m_FFTSize, GL_REPEAT, GL_LINEAR, nullptr, i_Config.TexUnit.Ocean.GPU2DIFFTComp.PingArrayMap + i, m_kMipmapCount);
	}

	float* pIndicesData = new float[m_FFTSize];
	assert(pIndicesData != nullptr);
	ComputeIndicesLookupTexture(pIndicesData);
	m_IndicesTexId = m_TM.Create1DTexture(GL_R16F, GL_RED, GL_FLOAT, m_FFTSize, GL_CLAMP_TO_EDGE, GL_NEAREST, pIndicesData, i_Config.TexUnit.Ocean.GPU2DIFFTComp.IndicesMap);
	SAFE_ARRAY_DELETE(pIndicesData);

	float* pWeightsData = new float[m_NumButterflies * m_FFTSize * 2];
	assert(pWeightsData != nullptr);
	ComputeWeightsLookupTexture(pWeightsData);
	m_WeightsTexId = m_TM.Create2DTexture(GL_RG16F, GL_RG, GL_FLOAT, m_FFTSize, m_NumButterflies, GL_CLAMP_TO_EDGE, GL_NEAREST, pWeightsData, i_Config.TexUnit.Ocean.GPU2DIFFTComp.WeightsMap);
	SAFE_ARRAY_DELETE(pWeightsData);
	//////////////


	/////////// HORIZONTAL ///////////
	m_HorizontalSM.Initialize("GPUComp2DFFT - Horizontal FFT");

	if (m_UseFFTSlopes)
	{
		m_HorizontalSM.BuildComputeProgram("../resources/shaders/FFTHorizontal.comp.glsl", i_Config);
	}
	else
	{
		m_HorizontalSM.BuildComputeProgram("../resources/shaders/FFTHorizontal_NoFFTSlopes.comp.glsl", i_Config);
	}

	m_HorizontalSM.UseProgram();

	m_HorizontalUniforms["u_Steps"] = m_HorizontalSM.GetUniformLocation("u_Steps");
	m_HorizontalSM.SetUniform(m_HorizontalUniforms.find("u_Steps")->second, m_NumButterflies);

	m_HorizontalSM.UnUseProgram();

	/////////// VERTICAL ///////////
	m_VerticalSM.Initialize("GPUComp2DFFT - Vertical FFT");

	if (m_UseFFTSlopes)
	{
		m_VerticalSM.BuildComputeProgram("../resources/shaders/FFTVertical.comp.glsl", i_Config);
	}
	else
	{
		m_VerticalSM.BuildComputeProgram("../resources/shaders/FFTVertical_NoFFTSlopes.comp.glsl", i_Config);
	}

	m_VerticalSM.UseProgram();

	m_VerticalUniforms["u_Steps"] = m_VerticalSM.GetUniformLocation("u_Steps");
	m_VerticalSM.SetUniform(m_VerticalUniforms.find("u_Steps")->second, m_NumButterflies);

	m_VerticalSM.UnUseProgram();

	LOG("GPUComp2DFFT successfully created!");
}

///////////////////////////////////
void GPUComp2DIFFT::ComputeIndicesLookupTexture ( float* i_pIndices )
{
	assert(i_pIndices != nullptr);

	for (unsigned short i = 0; i < m_FFTSize; ++i)
	{
		i_pIndices[i] = i;
	}

	ShuffleIndices(i_pIndices, m_FFTSize, 0);
}

void GPUComp2DIFFT::ShuffleIndices ( float* o_pBuffer, unsigned short i_N, unsigned short i_Offset )
{
	if (i_N > 1)
	{
		unsigned short i, k;

		unsigned short m = i_N / 2;

		float temp;
		for (i = 1; i < m; ++i)
		{
			for (k = 0; k < m - i; k++)
			{
				temp = o_pBuffer[i_Offset + 2 * k + i];
				o_pBuffer[i_Offset + 2 * k + i] = o_pBuffer[i_Offset + 2 * k + 1 + i];
				o_pBuffer[i_Offset + 2 * k + 1 + i] = temp;
			}
		}

		ShuffleIndices(o_pBuffer, m, i_Offset);
		ShuffleIndices(o_pBuffer, m, i_Offset + m);
	}
	else
	{
		// c0 = v0, so do nothing.
	}
}

void GPUComp2DIFFT::ComputeWeightsLookupTexture ( float* i_pWeights )
{
	assert(i_pWeights != nullptr);

	for (unsigned short i = 0; i < m_NumButterflies; ++i)
	{
		unsigned short nBlocks = static_cast<unsigned short>(glm::pow(2.0f, static_cast<float>(m_NumButterflies - 1 - i)));
		unsigned short nHInputs = static_cast<unsigned short>(glm::pow(2.0f, static_cast<float>(i)));
		for (unsigned short j = 0; j < nBlocks; ++j)
		{
			for (unsigned short k = 0; k < nHInputs; ++k)
			{
				unsigned short i1 = j * nHInputs * 2 + k;

				float wr = 0, wi = 0;
				ComputeWeight(k * nBlocks, wr, wi);

				unsigned short offset = 2 * (i * m_FFTSize + i1);
				i_pWeights[offset + 0] = wr;
				i_pWeights[offset + 1] = wi;
			}
		}
	}
}

void GPUComp2DIFFT::ComputeWeight ( unsigned short i_K, float& i_Wr, float& i_Wi )
{
	i_Wr = glm::cos(glm::two_pi<float>() * i_K / static_cast<float>(m_FFTSize));
	i_Wi = glm::sin(glm::two_pi<float>() * i_K / static_cast<float>(m_FFTSize));
}

void GPUComp2DIFFT::Perform2DIFFT ( void )
{
	///////// Compute shader setup /////////

	/////// Horizontal pass
	m_HorizontalSM.UseProgram();

	if (m_IsComputeShaderSupported)
	{
		glBindImageTexture(2, m_IndicesTexId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16F);
		glBindImageTexture(3, m_WeightsTexId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16F);

		// input - Ht
		glBindImageTexture(0, m_PingPongTexIds[0], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16F);
		glBindImageTexture(0, m_PingPongTexIds[0], 0, GL_TRUE, 1, GL_READ_ONLY, GL_RGBA16F);
		if (m_UseFFTSlopes)
		{
			glBindImageTexture(0, m_PingPongTexIds[0], 0, GL_TRUE, 2, GL_READ_ONLY, GL_RGBA16F);
		}

		// output - displacement
		glBindImageTexture(1, m_PingPongTexIds[1], 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glBindImageTexture(1, m_PingPongTexIds[1], 0, GL_TRUE, 1, GL_WRITE_ONLY, GL_RGBA16F);
		if (m_UseFFTSlopes)
		{
			glBindImageTexture(1, m_PingPongTexIds[1], 0, GL_TRUE, 2, GL_WRITE_ONLY, GL_RGBA16F);
		}
		// compute
		glDispatchCompute(1, m_FFTSize, 1);

		// Make sure, all values are written.
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	/////// Vertical pass
	m_VerticalSM.UseProgram();

	if (m_IsComputeShaderSupported)
	{
		glBindImageTexture(2, m_IndicesTexId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16F);
		glBindImageTexture(3, m_WeightsTexId, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16F);

		// input - displacement
		glBindImageTexture(0, m_PingPongTexIds[1], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA16F);
		glBindImageTexture(0, m_PingPongTexIds[1], 0, GL_TRUE, 1, GL_READ_ONLY, GL_RGBA16F);
		if (m_UseFFTSlopes)
		{
			glBindImageTexture(0, m_PingPongTexIds[1], 0, GL_TRUE, 2, GL_READ_ONLY, GL_RGBA16F);
		}

		// output - final displacement
		glBindImageTexture(1, m_PingPongTexIds[0], 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glBindImageTexture(1, m_PingPongTexIds[0], 0, GL_TRUE, 1, GL_WRITE_ONLY, GL_RGBA16F);
		if (m_UseFFTSlopes)
		{
			glBindImageTexture(1, m_PingPongTexIds[0], 0, GL_TRUE, 2, GL_WRITE_ONLY, GL_RGBA16F);
		}
		// compute
		glDispatchCompute(1, m_FFTSize, 1);

		// Make sure, all values are written.
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}

void GPUComp2DIFFT::BindDestinationTexture ( void ) const
{
	// only the final texture data needs mipmaps!
	m_TM.BindTexture(m_PingPongTexIds[0], true);
}

unsigned int GPUComp2DIFFT::GetSourceTexId ( void ) const
{
	return m_PingPongTexIds[0];
}

unsigned short GPUComp2DIFFT::GetSourceTexUnitId ( void ) const
{
	return m_TM.GetTextureUnitId(0);
}

unsigned int GPUComp2DIFFT::GetDestinationTexId ( void ) const
{
	return m_PingPongTexIds[0];
}

unsigned short GPUComp2DIFFT::GetDestinationTexUnitId ( void ) const
{
	return m_TM.GetTextureUnitId(0);
}