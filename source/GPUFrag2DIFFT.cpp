/* Author: BAIRAC MIHAI */

#include "GPUFrag2DIFFT.h"
#include "CommonHeaders.h"
#include "GLConfig.h"
#include "glm/vec4.hpp"
#include "glm/exponential.hpp" //pow()
#include "glm/trigonometric.hpp" // sin(). cos()
#include "glm/gtc/constants.hpp" //pi()
#include "glm/vector_relational.hpp" //any(), notEqual()
#include "GlobalConfig.h"
#include <sstream> // std::stringstream


GPUFrag2DIFFT::GPUFrag2DIFFT ( void )
  : m_pFFTFBM(nullptr), m_IsPongTarget(false), m_Use2FBOs(false)
{
	LOG("GPUFrag2DIFFT successfully created!");
}

GPUFrag2DIFFT::GPUFrag2DIFFT ( const GlobalConfig& i_Config )
  : m_pFFTFBM(nullptr), m_IsPongTarget(false), m_Use2FBOs(false)
{
	Initialize(i_Config);
}

GPUFrag2DIFFT::~GPUFrag2DIFFT ( void )
{
	Destroy();
}

void GPUFrag2DIFFT::Destroy ( void )
{
	// should free resources

	if (m_Use2FBOs)
	{
		SAFE_ARRAY_DELETE(m_pFFTFBM);
	}
	else
	{
		SAFE_DELETE(m_pFFTFBM);
	}

	LOG("GPUFrag2DIFFT successfully destroyed!");
}

void GPUFrag2DIFFT::Initialize ( const GlobalConfig& i_Config )
{
	Base2DIFFT::Initialize(i_Config);

	// For FFT Slopes we need 3 layers, otherwise only 2 are needed
	m_FFTLayerCount = (m_UseFFTSlopes ? 3 : 2);

	m_Use2FBOs = i_Config.Scene.Ocean.Surface.OceanPatch.ComputeFFT.Use2FBOs;

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

	float* pButterFlyData = new float[m_FFTSize * m_NumButterflies * 4];
	assert(pButterFlyData != nullptr);
	ComputeButterflyLookupTexture(pButterFlyData);

	m_TM.Initialize("GPUFrag2DIFFT - Butterfly Lookup Texture", i_Config);
	m_TM.Create2DTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT, m_FFTSize, m_NumButterflies, GL_CLAMP_TO_EDGE, GL_NEAREST, pButterFlyData, i_Config.TexUnit.Ocean.GPU2DIFFT.ButterflyMap);

	SAFE_ARRAY_DELETE(pButterFlyData);
	//////////////

	//// FFT FrameBuffer Setup
	if (m_Use2FBOs)
	{
		// 0 - ping, 1 - pong
		m_pFFTFBM = new FrameBufferManager[m_kPingPongLayerCount];
		assert(m_pFFTFBM != nullptr);

		for (short i = 0; i < m_kPingPongLayerCount; ++i)
		{
			if (&m_pFFTFBM[i])
			{
				m_pFFTFBM[i].Initialize("GPUFrag2DIFFT - FrameBuffer", i_Config);
				// NOTE! the fft details are localized only pretty near the camera position, so we don't need more than 3 levels of mipmaps .. ftt data doesn't get to far away from the camera!
				m_pFFTFBM[i].CreateLayered(1, m_FFTLayerCount, GL_RGBA16F, GL_RGBA, GL_FLOAT, m_FFTSize, m_FFTSize, GL_REPEAT, GL_LINEAR, i_Config.TexUnit.Ocean.GPU2DIFFT.PingArrayMap + i, m_kMipmapCount);
				m_pFFTFBM[i].Bind();
				m_pFFTFBM[i].SetupDrawBuffers(m_FFTLayerCount, 0);
				m_pFFTFBM[i].UnBind();
			}
		}
	}
	else
	{
		// use only one FBO for both textures
		m_pFFTFBM = new FrameBufferManager("GPUFrag2DIFFT - FrameBuffer", i_Config);
		assert(m_pFFTFBM != nullptr);

		// NOTE! the fft details are localized only pretty near the camera position, so we don't need more than 3 levels of mipmaps .. ftt data doesn't get to far away from the camera!
		if (m_pFFTFBM)
		{
			m_pFFTFBM->CreateLayered(m_kPingPongLayerCount, m_FFTLayerCount, GL_RGBA16F, GL_RGBA, GL_FLOAT, m_FFTSize, m_FFTSize, GL_REPEAT, GL_LINEAR, i_Config.TexUnit.Ocean.GPU2DIFFT.PingArrayMap, m_kMipmapCount);
		}
	}

	/////////// HORIZONTAL ///////////
	m_HorizontalSM.Initialize("GPUFrag2DIFFT - Horizontal FFT");

	if (m_UseFFTSlopes)
	{
		m_HorizontalSM.BuildRenderingProgram("resources/shaders/Quad.vert.glsl", "resources/shaders/FFTHorizontal.frag.glsl", i_Config);
	}
	else
	{
		m_HorizontalSM.BuildRenderingProgram("resources/shaders/Quad.vert.glsl", "resources/shaders/FFTHorizontal_NoFFTSlopes.frag.glsl", i_Config);
	}

	m_HorizontalSM.UseProgram();

	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> horizontalAttributes;
	horizontalAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_HorizontalSM.GetAttributeLocation("a_position");
	horizontalAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV] = m_HorizontalSM.GetAttributeLocation("a_uv");

	m_HorizontalUniforms["u_ButterflyMap"] = m_HorizontalSM.GetUniformLocation("u_ButterflyMap");
	m_HorizontalSM.SetUniform(m_HorizontalUniforms.find("u_ButterflyMap")->second, i_Config.TexUnit.Ocean.GPU2DIFFT.ButterflyMap);

	m_HorizontalUniforms["u_PingPongMap"] = m_HorizontalSM.GetUniformLocation("u_PingPongMap");

	if (m_Use2FBOs)
	{
		m_HorizontalSM.SetupFragmentOutputStreams(m_FFTLayerCount, 0);
	}
	m_HorizontalUniforms["u_Step"] = m_HorizontalSM.GetUniformLocation("u_Step");

	m_HorizontalSM.UnUseProgram();
	//////////
	m_HorizontalMBM.Initialize("GPUFrag2DIFFT - Horizontal FFT");

	if (m_Use2FBOs)
	{
		if (&m_pFFTFBM[0])
		{
			m_HorizontalMBM.CreateModelContext(horizontalAttributes, m_pFFTFBM[0].GetQuadVBOID(), m_pFFTFBM[0].GetQuadAccessType());
		}
	}
	else
	{
		if (m_pFFTFBM)
		{
			m_HorizontalMBM.CreateModelContext(horizontalAttributes, m_pFFTFBM->GetQuadVBOID(), m_pFFTFBM->GetQuadAccessType());
		}
	}

	/////////// VERTICAL ////////////
	m_VerticalSM.Initialize("GPUFrag2DIFFT - Vertical FFT");

	if (m_UseFFTSlopes)
	{
		m_VerticalSM.BuildRenderingProgram("resources/shaders/Quad.vert.glsl", "resources/shaders/FFTVertical.frag.glsl", i_Config);
	}
	else
	{
		m_VerticalSM.BuildRenderingProgram("resources/shaders/Quad.vert.glsl", "resources/shaders/FFTVertical_NoFFTSlopes.frag.glsl", i_Config);
	}

	m_VerticalSM.UseProgram();

	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> verticalAttributes;
	verticalAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_VerticalSM.GetAttributeLocation("a_position");
	verticalAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV] = m_VerticalSM.GetAttributeLocation("a_uv");

	m_VerticalUniforms["u_ButterflyMap"] = m_VerticalSM.GetUniformLocation("u_ButterflyMap");
	m_VerticalSM.SetUniform(m_VerticalUniforms.find("u_ButterflyMap")->second, i_Config.TexUnit.Ocean.GPU2DIFFT.ButterflyMap);
	m_VerticalUniforms["u_PingPongMap"] = m_VerticalSM.GetUniformLocation("u_PingPongMap");

	if (m_Use2FBOs)
	{
		m_VerticalSM.SetupFragmentOutputStreams(m_FFTLayerCount, 0);
	}

	m_VerticalUniforms["u_FFTSize"] = m_VerticalSM.GetUniformLocation("u_FFTSize");
	m_VerticalSM.SetUniform(m_VerticalUniforms.find("u_FFTSize")->second, static_cast<float>(m_FFTSize));
	m_VerticalUniforms["u_Step"] = m_VerticalSM.GetUniformLocation("u_Step");
	m_VerticalUniforms["u_IsLastStep"] = m_VerticalSM.GetUniformLocation("u_IsLastStep");

	m_VerticalSM.UnUseProgram();

	//////////
	m_VerticalMBM.Initialize("GPUFrag2DIFFT - Vertical FFT");

	if (m_Use2FBOs)
	{
		if (&m_pFFTFBM[1])
		{
			m_VerticalMBM.CreateModelContext(verticalAttributes, m_pFFTFBM[1].GetQuadVBOID(), m_pFFTFBM[1].GetQuadAccessType());
		}
	}
	else
	{
		if (m_pFFTFBM)
		{
			m_VerticalMBM.CreateModelContext(verticalAttributes, m_pFFTFBM->GetQuadVBOID(), m_pFFTFBM->GetQuadAccessType());
		}
	}

	LOG("GPUFrag2DIFFT successfully created!");
}

///////////////////////////////////
// source from Eric Bruneton's fft ocean demo:  http://www-evasion.imag.fr/people/Eric.Bruneton/
void GPUFrag2DIFFT::ComputeButterflyLookupTexture ( float* i_pData )
{
	assert(i_pData != nullptr);

	for (unsigned short i = 0; i < m_NumButterflies; ++i)
	{
		unsigned short nBlocks = static_cast<unsigned short>(glm::pow(2.0f, static_cast<float>(m_NumButterflies - 1 - i)));
		unsigned short nHInputs = static_cast<unsigned short>(glm::pow(2.0f, static_cast<float>(i)));
		for (unsigned short j = 0; j < nBlocks; ++j)
		{
			for (unsigned short k = 0; k < nHInputs; ++k)
			{
				unsigned short i1 = 0, i2 = 0, j1 = 0, j2 = 0;
				if (i == 0)
				{
					i1 = j * nHInputs * 2 + k;
					i2 = j * nHInputs * 2 + nHInputs + k;
					j1 = BitReverse(i1);
					j2 = BitReverse(i2);
				}
				else
				{
					i1 = j * nHInputs * 2 + k;
					i2 = j * nHInputs * 2 + nHInputs + k;
					j1 = i1;
					j2 = i2;
				}

				float wr = 0, wi = 0;
				ComputeWeight(k * nBlocks, wr, wi);

				float fFFTSize = static_cast<float>(m_FFTSize);
				unsigned short offset1 = 4 * (i1 + i * m_FFTSize);
				i_pData[offset1 + 0] = (j1 + 0.5f) / fFFTSize;
				i_pData[offset1 + 1] = (j2 + 0.5f) / fFFTSize;
				i_pData[offset1 + 2] = wr;
				i_pData[offset1 + 3] = wi;

				unsigned short offset2 = 4 * (i2 + i * m_FFTSize);
				i_pData[offset2 + 0] = (j1 + 0.5f) / fFFTSize;
				i_pData[offset2 + 1] = (j2 + 0.5f) / fFFTSize;
				i_pData[offset2 + 2] = -wr;
				i_pData[offset2 + 3] = -wi;
			}
		}
	}
}

unsigned short GPUFrag2DIFFT::BitReverse ( unsigned short i_I )
{
	unsigned short Sum = 0;
	unsigned short W = 1;
	unsigned short j = i_I;
	unsigned short N = m_FFTSize / 2;
	while (N != 0)
	{
		j = (i_I & N) > N - 1;
		Sum += j * W;
		W *= 2;
		N = N / 2;
	}
	return Sum;
}

void GPUFrag2DIFFT::ComputeWeight ( unsigned short i_K, float& i_Wr, float& i_Wi )
{
	i_Wr = glm::cos(glm::two_pi<float>() * i_K / static_cast<float>(m_FFTSize));
	i_Wi = glm::sin(glm::two_pi<float>() * i_K / static_cast<float>(m_FFTSize));
}
////////////////////////////////

void GPUFrag2DIFFT::Perform2DIFFT ( void )
{
	glm::ivec4 oldViewport;
	glm::ivec4 newViewport(0, 0, m_FFTSize, m_FFTSize);

	// save current viewport
	glGetIntegerv(GL_VIEWPORT, &oldViewport[0]);

	// set new viewport
	if (glm::any(glm::notEqual(newViewport, oldViewport)))
	{
		glViewport(newViewport.x, newViewport.y, newViewport.z, newViewport.w);
	}

	if (! m_Use2FBOs)
	{
		if (m_pFFTFBM) m_pFFTFBM->Bind();
	}

	///// Horizontal pass
	m_HorizontalSM.UseProgram();
	m_HorizontalMBM.BindModelContext();

	m_IsPongTarget = true;

	for (unsigned short i = 0; i < m_NumButterflies; ++ i)
	{
		m_HorizontalSM.SetUniform(m_HorizontalUniforms.find("u_Step")->second, i / static_cast<float>(m_NumButterflies - 1));

		if (m_Use2FBOs)
		{
			if (&m_pFFTFBM[0] && &m_pFFTFBM[1])
			{
				m_pFFTFBM[m_IsPongTarget].Bind();

				m_pFFTFBM[!m_IsPongTarget].BindColorAttachmentByIndex(0);
				m_HorizontalSM.SetUniform(m_HorizontalUniforms.find("u_PingPongMap")->second, m_pFFTFBM[!m_IsPongTarget].GetColorAttachmentTexUnitId(0));

				m_IsPongTarget = !m_IsPongTarget;

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}
		else
		{
			if (m_pFFTFBM)
			{
				if (m_IsPongTarget)
				{
					// read from Ping (first time Ping texture contains Ht data)
					m_pFFTFBM->BindColorAttachmentByIndex(0);
					unsigned short kPingMapTexUnitId = m_pFFTFBM->GetColorAttachmentTexUnitId(0);
					m_HorizontalSM.SetUniform(m_HorizontalUniforms.find("u_PingPongMap")->second, kPingMapTexUnitId);
					m_HorizontalSM.SetupFragmentOutputStreams(m_FFTLayerCount, m_FFTLayerCount);

					// write to Pong
					m_pFFTFBM->SetupDrawBuffers(m_FFTLayerCount, m_FFTLayerCount);
				}
				else
				{
					// read from Pong
					m_pFFTFBM->BindColorAttachmentByIndex(1);
					unsigned short kPongMapTexUnitId = m_pFFTFBM->GetColorAttachmentTexUnitId(1);
					m_HorizontalSM.SetUniform(m_HorizontalUniforms.find("u_PingPongMap")->second, kPongMapTexUnitId);
					m_HorizontalSM.SetupFragmentOutputStreams(m_FFTLayerCount, 0);

					// write to Ping
					m_pFFTFBM->SetupDrawBuffers(m_FFTLayerCount, 0);
				}
				m_IsPongTarget = !m_IsPongTarget;

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}
	}
	m_HorizontalMBM.UnBindModelContext();
	
	/////// Vertical pass
	m_VerticalSM.UseProgram();
	m_VerticalMBM.BindModelContext();

	for (unsigned short i = 0; i < m_NumButterflies; ++ i)
	{
		m_VerticalSM.SetUniform(m_VerticalUniforms.find("u_Step")->second, i / static_cast<float>(m_NumButterflies - 1));
		if (i == 0) m_VerticalSM.SetUniform(m_VerticalUniforms.find("u_IsLastStep")->second, false);
		if (i == m_NumButterflies - 1) m_VerticalSM.SetUniform(m_VerticalUniforms.find("u_IsLastStep")->second, true);

		if (m_Use2FBOs)
		{
			if (&m_pFFTFBM[0] && &m_pFFTFBM[1])
			{
				m_pFFTFBM[m_IsPongTarget].Bind();

				m_pFFTFBM[!m_IsPongTarget].BindColorAttachmentByIndex(0);
				m_VerticalSM.SetUniform(m_VerticalUniforms.find("u_PingPongMap")->second, m_pFFTFBM[!m_IsPongTarget].GetColorAttachmentTexUnitId(0));

				m_IsPongTarget = !m_IsPongTarget;

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}
		else
		{
			if (m_pFFTFBM)
			{
				if (m_IsPongTarget)
				{
					// read from Ping
					m_pFFTFBM->BindColorAttachmentByIndex(0);
					const unsigned short& kPingMapTexUnitId = m_pFFTFBM->GetColorAttachmentTexUnitId(0);
					m_VerticalSM.SetUniform(m_VerticalUniforms.find("u_PingPongMap")->second, kPingMapTexUnitId);
					m_VerticalSM.SetupFragmentOutputStreams(m_FFTLayerCount, m_FFTLayerCount);

					// write to Pong
					m_pFFTFBM->SetupDrawBuffers(m_FFTLayerCount, m_FFTLayerCount);
				}
				else
				{
					// read from Pong
					m_pFFTFBM->BindColorAttachmentByIndex(1);
					const unsigned short& kPongMapTexUnitId = m_pFFTFBM->GetColorAttachmentTexUnitId(1);
					m_VerticalSM.SetUniform(m_VerticalUniforms.find("u_PingPongMap")->second, kPongMapTexUnitId);
					m_VerticalSM.SetupFragmentOutputStreams(m_FFTLayerCount, 0);

					// write to Ping
					m_pFFTFBM->SetupDrawBuffers(m_FFTLayerCount, 0);
				}
				m_IsPongTarget = !m_IsPongTarget;

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}
	}
	m_VerticalMBM.UnBindModelContext();
	
	// go back to default framebuffer
	if (m_pFFTFBM) m_pFFTFBM->UnBind();

	// reset the viewport
	if (glm::any(glm::notEqual(newViewport, oldViewport)))
	{
		glViewport(oldViewport.x, oldViewport.y, oldViewport.z, oldViewport.w);
	}
}

void GPUFrag2DIFFT::BindDestinationTexture ( void ) const
{
	// only the final texture data needs mipmaps!
	if (m_Use2FBOs)
	{
		if (&m_pFFTFBM[0])
		{
			m_pFFTFBM[0].BindColorAttachmentByIndex(0, true);
		}
	}
	else
	{
		if (m_pFFTFBM)
		{
			m_pFFTFBM->BindColorAttachmentByIndex(0, true);
		}
	}
}

unsigned int GPUFrag2DIFFT::GetSourceTexId ( void ) const
{
	unsigned int val = 0;

	if (m_Use2FBOs)
	{
		if (&m_pFFTFBM[0])
		{
			val = m_pFFTFBM[0].GetColorAttachmentTexId(0);
		}
	}
	else
	{
		if (m_pFFTFBM)
		{
			val = m_pFFTFBM->GetColorAttachmentTexId(0);
		}
	}

	return val;
}

unsigned int GPUFrag2DIFFT::GetDestinationTexId ( void ) const
{
	unsigned short val = 0;

	if (m_Use2FBOs)
	{
		if (&m_pFFTFBM[0])
		{
			val = m_pFFTFBM[0].GetColorAttachmentTexId(0);
		}
	}
	else
	{
		if (m_pFFTFBM)
		{
			val = m_pFFTFBM->GetColorAttachmentTexId(0);
		}
	}

	return val;
}

unsigned short GPUFrag2DIFFT::GetDestinationTexUnitId ( void ) const
{
	unsigned short val = 0;

	if (m_Use2FBOs)
	{
		if (&m_pFFTFBM[0])
		{
			val = m_pFFTFBM[0].GetColorAttachmentTexUnitId(0);
		}
	}
	else
	{
		if (m_pFFTFBM)
		{
			val = m_pFFTFBM->GetColorAttachmentTexUnitId(0);
		}
	}

	return val;
}