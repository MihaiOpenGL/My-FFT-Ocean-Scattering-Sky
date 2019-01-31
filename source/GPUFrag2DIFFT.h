/* Author: BAIRAC MIHAI

Implementation based on work:

Bruneton's FFT ocean implementation:
http://www-evasion.imag.fr/people/Eric.Bruneton/
License: check the demo package

*/

#ifndef GPU_FRAG_2D_IFFT_H
#define GPU_FRAG_2D_IFFT_H

#include <string>
#include <vector>
#include <map>
#include "Base2DIFFT.h"
#include "ShaderManager.h"
#include "MeshBufferManager.h"
#include "FrameBufferManager.h"

class GlobalConfig;

/*
 GPU implementation of the 2D IFFT using fragment shaders
 based on: Eric Bruneton's FFT ocean implementation:
 http://www-evasion.imag.fr/people/Eric.Bruneton/

 More details about FFT computation on GPU:
 nVidia DirectX 11 SDK Ocean demo: https://developer.nvidia.com/dx11-samples

 Multiple output streams from fragment shader
 http://stackoverflow.com/questions/25835374/multiple-output-from-fragment-shader-using-a-fbo
 16 bit floating point texture
 http://gamedev.stackexchange.com/questions/67420/how-do-i-efficiently-use-16-bit-texture-coordinates
*/

class GPUFrag2DIFFT: public Base2DIFFT
{
public:
	GPUFrag2DIFFT(void);
	GPUFrag2DIFFT(const GlobalConfig& i_Config);
	~GPUFrag2DIFFT(void);

	void Initialize(const GlobalConfig& i_Config) override;
	void Perform2DIFFT(void) override;
	void BindDestinationTexture(void) const override;

	unsigned int GetSourceTexId(void) const override;
	unsigned int GetDestinationTexId(void) const override;
	unsigned short GetDestinationTexUnitId(void) const override;

private:
	//// Methods ////
	void Destroy(void);

	float* ComputeButterflyLookupTexture(void);
	unsigned short BitReverse(unsigned short i_I);
	void ComputeWeight(unsigned short i_K, float& i_Wr, float& i_Wi);

	//// Variables ////
	static const unsigned short m_kPingPongLayerCount = 2;

	//////// Horizontal and Vertical passes 
	ShaderManager m_HorizontalSM, m_VerticalSM;
	MeshBufferManager m_HorizontalMBM, m_VerticalMBM;
	FrameBufferManager* m_pFFTFBM;

	// self init
	// name, location
	std::map<std::string, int> m_HorizontalUniforms;
	std::map<std::string, int> m_VerticalUniforms;

	bool m_IsPongTarget;
	bool m_Use2FBOs;
};

#endif /* GPU_FRAG_2D_IFFT_H */