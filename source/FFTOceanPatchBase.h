/* Author: BAIRAC MIHAI

 Current implementation is based on Jerry Tessendorf's paper:
 Simulating Ocean Water - 2001

 It implements wind dirven waves using Phillips and Unified spectra

*/

#ifndef FFT_OCEAN_PATCH_BASE_H
#define FFT_OCEAN_PATCH_BASE_H

#include <string>
#include <map>

#include "CustomTypes.h"
#include "GlobalConfig.h"

#include "ShaderManager.h"

//#define GLM_SWIZZLE //offers the possibility to use: .xx(), xy(), xyz(), ...
#include "glm/vec2.hpp" //

/*
 Base class for FFT ocean patch
 Computes: the waves spectrum, frequency distribution

 Check FFTNormalGradientFoldingBase see how the gradients for normals and the folding are computed!
*/

class FFTNormalGradientFoldingBase;

class FFTOceanPatchBase
{
protected:
	///// statics
	static const unsigned short m_kMipmapCount = 3;

	//// Variables ////
	unsigned short m_FFTSize;
	unsigned short m_PatchSize;

	// General FFT Wave spectrum
	float m_WaveAmplitude;
	float m_WaveAmplitudeScale;
	float m_WindSpeed;
	glm::vec2 m_WindDirection;
	float m_DispersionFrequencyTimePeriod;
	float m_ChoppyScale;
	float m_TileScale;

	// Phillips spectrum
	float m_OpposingWavesFactor;
	float m_VerySmallWavesFactor;

	// Unified spectrum
	float m_SeaState;
	float m_MinimumPhaseSpeed;
	float m_SecondaryGravityCapillaryPeak;

	CustomTypes::Ocean::SpectrumType m_SpectrumType;

	FFTNormalGradientFoldingBase* m_pNormalGradientFolding;

	//// Methods ////
	virtual void SetFFTData ( void );
	virtual void InitFFTData ( void );

	virtual float PhillipsSpectrum ( const glm::vec2& i_WaveVector );
	virtual glm::vec2 GaussianRandomVariable ( void );

	virtual float UnifiedSpectrum ( const glm::vec2& i_WaveVector );
	virtual float Omega ( float i_K, float i_KM );
	virtual float Sqr ( float i_X );

	virtual float DispersionFrequency ( const glm::vec2& i_WaveVector );

private:
	void Destroy ( void );

	float UniformRandomVariable ( void );

public:
	FFTOceanPatchBase ( void );
	FFTOceanPatchBase ( const GlobalConfig& i_Config );
	virtual ~FFTOceanPatchBase ( void );

	virtual void Initialize ( const GlobalConfig& i_Config );

	virtual void EvaluateWaves ( float i_CrrTime );

	virtual float ComputeWaterHeightAt ( const glm::vec2& i_XZ ) const;

	virtual void BindFFTWaveDataTexture ( void ) const;
	virtual void BindNormalFoldingTexture ( void ) const;

	virtual unsigned short GetFFTWaveDataTexUnitId ( void ) const;
	virtual unsigned short GetNormalGradientFoldingTexUnitId ( void ) const;

	virtual float GetWaveAmplitude ( void ) const;
	virtual unsigned short GetPatchSize ( void ) const;
	virtual float GetWindSpeed ( void ) const;
	virtual float GetWindDirectionX ( void ) const;
	virtual float GetWindDirectionZ ( void ) const;
	virtual glm::vec3 GetWindDir ( void ) const;
	virtual float GetOpposingWavesFactor ( void ) const;
	virtual float GetVerySmallWavesFactor ( void ) const;
	virtual float GetChoppyScale ( void ) const;
	virtual float GetTileScale ( void ) const;

	virtual void SetWaveAmplitude ( float i_WaveAmplitude );
	virtual void SetPatchSize ( unsigned short i_PatchSize );
	virtual void SetWindSpeed ( float i_WindSpeed );
	virtual void SetWindDirectionX ( float i_WindDirectionX );
	virtual void SetWindDirectionZ ( float i_WindDirectionZ );
	virtual void SetOpposingWavesFactor ( float i_OpposingWavesFactor );
	virtual void SetVerySmallWavesFactor ( float i_VerySmallWavesFactor );
	virtual void SetChoppyScale ( float i_ChoppyScale );
	virtual void SetTileScale ( float i_TileScale );
};

#endif /* FFT_OCEAN_PATCH_BASE_H */