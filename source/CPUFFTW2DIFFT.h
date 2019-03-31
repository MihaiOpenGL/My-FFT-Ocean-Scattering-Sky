/* Author: BAIRAC MIHAI

 Implementation based on FFTW: http://fftw.org/
 License: http://www.fftw.org/doc/License-and-Copyright.html

*/

#ifndef CPU_FFTW_2D_IFFT_H
#define CPU_FFTW_2D_IFFT_H

#include "Base2DIFFT.h"
#include "FFTW/fftw3.h"
#include "glm/vec4.hpp"
#include "MeshBufferManager.h"
#include <complex> //to use std::complex numbers

//OPTIMIZATION: use single precision(float) fftw, by default the double-precision(double) is used!
#define fftw_complex         fftwf_complex
#define fftw_plan            fftwf_plan
#define fftw_destroy_plan    fftwf_destroy_plan
#define fftw_plan_dft_2d     fftwf_plan_dft_2d
#define fftw_execute         fftwf_execute
#define fftw_malloc          fftwf_malloc
#define fftw_free            fftwf_free

class GlobalConfig;

/*
 CPU implementation of the 2D IFFT using the FFTW - a free 3rd party library
 More info about FFTW: http://fftw.org/
*/

class CPUFFTW2DIFFT: public Base2DIFFT
{
public:
	CPUFFTW2DIFFT(void);
	CPUFFTW2DIFFT(const GlobalConfig& i_Config);
	~CPUFFTW2DIFFT(void);

	void Initialize(const GlobalConfig& i_Config) override;

	void Pre2DFFTSetup(const std::complex<float>& i_DX, const std::complex<float>& i_DY, const std::complex<float>& i_DZ, const std::complex<float>& i_SX, const std::complex<float>& i_SZ, unsigned int i_Index);
	void Perform2DIFFT(void) override;
	void Post2DFFTSetup(short i_Sign, unsigned int i_Index);

	void UpdateTextureData(void);

	void BindDestinationTexture(void) const override;

	unsigned int GetDestinationTexId(void) const override;
	unsigned short GetDestinationTexUnitId(void) const override;

private:
	//// Methods ////
	void Destroy(void);

	//// Variables ////
	// Pointers are needed here, because we don't know the exact FFT size
	// for fast fourier transform
	fftw_complex *m_pDY, *m_pDX, *m_pDZ; // fft displacement on OX, OY and OZ
	fftw_plan m_PDY, m_PDX, m_PDZ; // fftw plans

	fftw_complex *m_pSX, *m_pSZ; // slopes on OX and OZ
	fftw_plan m_PSX, m_PSZ; // fftw plans

	unsigned int m_FFTDataTexId;

	std::vector<glm::vec4> m_FFTProcessedData;
};

#endif /* CPU_FFTW_2D_IFFT_H */