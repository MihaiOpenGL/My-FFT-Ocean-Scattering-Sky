/* Author: BAIRAC MIHAI */

#ifndef FFT_OCEAN_PATCH_GPU_COMP_H
#define FFT_OCEAN_PATCH_GPU_COMP_H

#include "FFTOceanPatchBase.h"
#include "ShaderManager.h"
#include "TextureManager.h"
//#define GLM_SWIZZLE //offers the possibility to use: .xx(), xy(), xyz(), ...
#include "glm/vec2.hpp" //
#include "glm/vec4.hpp" //
#include "GPUComp2DIFFT.h"
#include <string>
#include <vector>
#include <map>

class GlobalConfig;

/*
 GPU implementation of the FFT ocean patch using compute shaders
 Check GPUComp2DIFFT class for more details
*/

class FFTOceanPatchGPUComp : public FFTOceanPatchBase
{
public:
	FFTOceanPatchGPUComp(void);
	FFTOceanPatchGPUComp(const GlobalConfig& i_Config);
	~FFTOceanPatchGPUComp(void);

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
	GPUComp2DIFFT m_2DIFFT;

	// init fft data
	unsigned int m_FFTInitDataTexId;

	std::vector<glm::vec4> m_FFTInitData;

	float* m_pFFTDisplaymentData;

	//////// FFT Ht //////
	ShaderManager m_FFTHtSM;
	TextureManager m_FFTTM;

	// self init
	// name, location
	std::map<std::string, int> m_FFTHtUniforms;

	bool m_IsComputeShaderSupported;
};

#endif /* FFT_OCEAN_PATCH_GPU_COMP_H */