/* Author: BAIRAC MIHAI */

#ifndef FFT_OCEAN_PATCH_CPU_FFTW_H
#define FFT_OCEAN_PATCH_CPU_FFTW_H

#include <vector>
#include "FFTOceanPatchBase.h"
//#define GLM_SWIZZLE //offers the possibility to use: .xx(), xy(), xyz(), ...
#include "glm/vec2.hpp" //
#include "CPUFFTW2DIFFT.h"
#include <complex>

class GlobalConfig;

/*
 CPU implementation of the FFT ocean patch uisng the FFTW thrid-party lib
 Check CPUFFTW2IFFT class for more details
 Havily based on complex numbers
*/

class FFTOceanPatchCPUFFTW : public FFTOceanPatchBase
{
public:
	FFTOceanPatchCPUFFTW(void);
	FFTOceanPatchCPUFFTW(const GlobalConfig& i_Config);
	~FFTOceanPatchCPUFFTW(void);

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

	std::complex<float> HTilde0(const glm::vec2& i_WaveVector);
	std::complex<float> HTilde(unsigned int i_Index, float i_CrrTime);

	//// Variables ////
	CPUFFTW2DIFFT m_2DIFFT;

	// init fft data
	struct FFTInitData
	{
		std::complex<float> hTilde0; // htilde0 /// ecquation (26) from Jerry Tessendorf's article
		std::complex<float> hTilde0Conj; // htilde0 conjugate /// ec. (26) from Jerry Tessendorf's article
		float dispersionFrequency; //ec. (17) from Jerry Tessendorf's article
	};

	std::vector<FFTInitData> m_FFTInitData;

	float* m_pFFTDisplaymentData;
};

#endif /* FFT_OCEAN_PATCH_CPU_FFTW_H */