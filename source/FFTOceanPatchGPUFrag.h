/* Author: BAIRAC MIHAI */

#ifndef FFT_OCEAN_PATCH_GPU_FRAG_H
#define FFT_OCEAN_PATCH_GPU_FRAG_H

#include "FFTOceanPatchBase.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "FrameBufferManager.h"
//#define GLM_SWIZZLE //offers the possibility to use: .xx(), xy(), xyz(), ...
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "GPUFrag2DIFFT.h"
#include <string>
#include <vector>
#include <map>

class GlobalConfig;

/*
 GPU implementation of the FFT ocean patch using fragment shaders
 Check GPUFrag2DIFFT class for more details
*/

class FFTOceanPatchGPUFrag : public FFTOceanPatchBase
{
public:
	FFTOceanPatchGPUFrag(void);
	FFTOceanPatchGPUFrag(const GlobalConfig& i_Config);
	~FFTOceanPatchGPUFrag(void);

	void Initialize(const GlobalConfig& i_Config) override;

	void EvaluateWaves(float i_CrrTime) override;

	float ComputeWaterHeightAt(const glm::vec2& i_XZ) const override;

	void BindFFTWaveDataTexture(void) const override;
	unsigned short GetFFTWaveDataTexUnitId(void) const override;

private:
	//// Methods ////
	void Destroy(void);

	void SetFFTData(void) override;
	void InitFFTData(void) override;

	//// Variables ////
	GPUFrag2DIFFT m_2DIFFT;

	// init fft data
	unsigned int m_FFTInitDataTexId;

	std::vector<glm::vec3> m_FFTInitData;

	float* m_pFFTDisplaymentData;

	//////// FFT Ht //////
	ShaderManager m_FFTHtSM;
	FrameBufferManager m_FFTHtFBM;
	TextureManager m_FFTTM;

	// self init
	// name, location
	std::map<std::string, int> m_FFTHtUniforms;
};

#endif /* FFT_OCEAN_PATCH_GPU_FRAG_H */