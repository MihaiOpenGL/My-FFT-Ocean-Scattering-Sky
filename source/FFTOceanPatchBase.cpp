/* Author: BAIRAC MIHAI */

/*
Historical data

Wave spectrum models:
Phillips - 1958, 1985
JONSWAP - 1973
Pierson & Moskowitz - 1964
Donel & Persion - 1987
Unified - based on work by: Phillips, Donel, Pierson, Moskowitz, etc. - 1997
*/

#include "FFTOceanPatchBase.h"
#include "CommonHeaders.h"
// glm::vec2, glm::vec3 come from the header
#include "glm/common.hpp" //floor()
#include "glm/gtc/constants.hpp" //two_pi()
#include "glm/exponential.hpp" //exp(), pow(), log(), sqrt()
#include "glm/trigonometric.hpp" //cos(), atan(), tanh()
#include "glm/geometric.hpp" // dot(), normalize(), length()
#include "PhysicsConstants.h"
#include "GlobalConfig.h"
#include "FFTNormalGradientFoldingGPUFrag.h"
#include "FFTNormalGradientFoldingGPUComp.h"


FFTOceanPatchBase::FFTOceanPatchBase ( void )
	: m_WaveAmplitudeScale(0), m_FFTSize(0), m_PatchSize(0),
	  m_WaveAmplitude(0.0f), m_WindSpeed(0.0f), m_DispersionFrequencyTimePeriod(0.0f),
	  m_ChoppyScale(0.0f), m_TileScale(0.0f),
	  m_OpposingWavesFactor(0.0f), m_VerySmallWavesFactor(0.0f),
	  m_SeaState(0.0f), m_MinimumPhaseSpeed(0.0f), m_SecondaryGravityCapillaryPeak(0.0f),
	  m_SpectrumType(CustomTypes::Ocean::SpectrumType::ST_COUNT),
	  m_pNormalGradientFolding(nullptr)
{
	LOG("FFTOceanPatchBase successfully created!");
}

FFTOceanPatchBase::FFTOceanPatchBase ( const GlobalConfig& i_Config )
	: m_WaveAmplitudeScale(0), m_FFTSize(0), m_PatchSize(0),
	  m_WaveAmplitude(0.0f), m_WindSpeed(0.0f), m_DispersionFrequencyTimePeriod(0.0f),
	  m_ChoppyScale(0.0f), m_TileScale(0.0f),
	  m_OpposingWavesFactor(0.0f), m_VerySmallWavesFactor(0.0f),
	  m_SeaState(0.0f), m_MinimumPhaseSpeed(0.0f), m_SecondaryGravityCapillaryPeak(0.0f),
	  m_SpectrumType(CustomTypes::Ocean::SpectrumType::ST_COUNT),
	  m_pNormalGradientFolding(nullptr)
{
	Initialize(i_Config);
}

FFTOceanPatchBase::~FFTOceanPatchBase ( void )
{
	Destroy();
}

void FFTOceanPatchBase::Destroy ( void )
{
	// should free resources
	SAFE_DELETE(m_pNormalGradientFolding);

	LOG("FFTOceanPatchBase successfully destroyed!");
}

void FFTOceanPatchBase::Initialize ( const GlobalConfig& i_Config )
{
	m_FFTSize = i_Config.Scene.Ocean.Surface.OceanPatch.FFTSize;
	m_PatchSize = i_Config.Scene.Ocean.Surface.OceanPatch.PatchSize;
	m_WaveAmplitude = i_Config.Scene.Ocean.Surface.OceanPatch.WaveAmpltitude;
	m_WindSpeed = i_Config.Scene.Ocean.Surface.OceanPatch.WindSpeed;
	m_WindDirection = i_Config.Scene.Ocean.Surface.OceanPatch.WindDirection;
	m_DispersionFrequencyTimePeriod = i_Config.Scene.Ocean.Surface.OceanPatch.DispersionFrequencyTimePeriod;
	m_ChoppyScale = i_Config.Scene.Ocean.Surface.OceanPatch.ChoppyScale;
	m_TileScale = i_Config.Scene.Ocean.Surface.OceanPatch.TileScale;

	m_OpposingWavesFactor = i_Config.Scene.Ocean.Surface.OceanPatch.Spectrum.Phillips.OpposingWavesFactor;
	m_VerySmallWavesFactor = i_Config.Scene.Ocean.Surface.OceanPatch.Spectrum.Phillips.VerySmallWavesFactor;

	m_SeaState = i_Config.Scene.Ocean.Surface.OceanPatch.Spectrum.Unified.SeaState;
	m_MinimumPhaseSpeed = i_Config.Scene.Ocean.Surface.OceanPatch.Spectrum.Unified.MinimumPhaseSpeed;
	m_SecondaryGravityCapillaryPeak = i_Config.Scene.Ocean.Surface.OceanPatch.Spectrum.Unified.SecondaryGravityCapillaryPeak;

	m_SpectrumType = i_Config.Scene.Ocean.Surface.OceanPatch.Spectrum.Type;

	if (m_FFTSize == 1024)
	{
		m_WaveAmplitudeScale = 1e-7f;
	}
	else if (m_FFTSize == 512)
	{
		m_WaveAmplitudeScale = 1e-6f;
	}
	else if (m_FFTSize == 256)
	{
		m_WaveAmplitudeScale = 1e-6f;
	}
	else if (m_FFTSize == 128)
	{
		m_WaveAmplitudeScale = 1e-5f;
	}

	/////////// NORMAL, FOLDING SETUP ///////////
	if (i_Config.Scene.Ocean.Surface.OceanPatch.NormalGradientFolding.Type == CustomTypes::Ocean::NormalGradientFoldingType::NGF_GPU_FRAG)
	{
		m_pNormalGradientFolding = new FFTNormalGradientFoldingGPUFrag(i_Config);
	}
	else if (i_Config.Scene.Ocean.Surface.OceanPatch.NormalGradientFolding.Type == CustomTypes::Ocean::NormalGradientFoldingType::NGF_GPU_COMP)
	{
		m_pNormalGradientFolding = new FFTNormalGradientFoldingGPUComp(i_Config);
	}
	assert(m_pNormalGradientFolding != nullptr);
	
	LOG("FFTOceanPatchBase successfully created!");
}

void FFTOceanPatchBase::SetFFTData ( void )
{
	InitFFTData();
}

void FFTOceanPatchBase::InitFFTData ( void )
{
	srand(0);
}

float FFTOceanPatchBase::PhillipsSpectrum ( const glm::vec2& i_WaveVector )
{
	// Jerry Tessendorf - Simulating Ocean Water - 2001 paper
	// paper: https://people.cs.clemson.edu/~jtessen/papers_files/coursenotes2004.pdf

	// Phillips spectrum implementation accroding to Jerry Tesendorf paper (23)

	float waveVectorSqr = i_WaveVector.x * i_WaveVector.x + i_WaveVector.y * i_WaveVector.y;

	// NOTE!  m_WindDirection is already normalized
	float waveDotWind = glm::dot(glm::normalize(i_WaveVector), m_WindDirection);

	// L - largest possible waves arising, L = W^2 / g, W - wind velocity, g - gravitational constant, g = 9.81 m^2/s
	float L = m_WindSpeed * m_WindSpeed / PhysicsConstants::kG;

	// Ec. (23)
	// A - amplitude, influences the wave height

	float phillips = m_WaveAmplitude * m_WaveAmplitudeScale * glm::exp(-1.0f / (waveVectorSqr * L * L)) * (waveDotWind * waveDotWind) / (waveVectorSqr * waveVectorSqr);

	//Avoid division by zero
	if (L == 0.0f || waveVectorSqr == 0.0f)
	{
		return 0.0f;
	}

	// removing the waves that go against the wind
	// details can be found between Ec. (23) si (24)
	if (waveDotWind < 0.0f)
	{
		phillips *= m_OpposingWavesFactor;
	}


	// eliminating the capillary waves
	float l = L * m_VerySmallWavesFactor;
	// Ec. (24)
	float damp = glm::exp(-waveVectorSqr * l * l);

	return phillips * damp;
}

// 1/kx and 1/ky in meters
float FFTOceanPatchBase::UnifiedSpectrum ( const glm::vec2& i_WaveVector )
{
	// WAVES SPECTRUM
	// using "A unified directional spectrum for long and short wind-driven waves"
	// T. Elfouhaily, B. Chapron, K. Katsaros, D. Vandemark
	// Journal of Geophysical Research vol 102, p781-796, 1997
	// 

	// paper: http://archimer.ifremer.fr/doc/00091/20226/17877.pdf

	// sea state (inverse wave age)
	// 0.84 - fully developed
	// 1.0 - mature
	// >2.0 - young
	float w = m_SeaState; // omega - [0.84, 5.0]

	// minimum phase speed at the wavenumber km
	float cm = m_MinimumPhaseSpeed; // Eq 59

	// km - secondary gravity - capillary peak
	float km = m_SecondaryGravityCapillaryPeak; // Eq 59

	float U10 = m_WindSpeed; // wind - 10 meters above water

	// phase speed
	float k = glm::length(i_WaveVector);

	//// added wind direction dependency
	float waveDotWind = glm::dot(glm::normalize(i_WaveVector), m_WindDirection);
	
	// c(k) - wave phase speed
	float c = Omega(k, km) / k;

	// kp - spectral peak
	float kp = PhysicsConstants::kG * Sqr(w / U10); // after Eq 3

	// cp - phase speed at the spectral peak
	float cp = Omega(kp, km) / kp;

	// friction velocity
	float z0 = 3.7e-5f * Sqr(U10) / PhysicsConstants::kG * glm::pow(U10 / cp, 0.9f); // Eq 66
	float u_star = 0.41f * U10 / glm::log(10.0f / z0); // Eq 60

	float Lpm = glm::exp(-5.0f / 4.0f * Sqr(kp / k)); // after Eq 3
	float gamma = w < 1.0f ? 1.7f : 1.7f + 6.0f * glm::log(w); // after Eq 3 // log10 or log??
	float sigma = 0.08f * (1.0f + 4.0f / glm::pow(w, 3.0f)); // after Eq 3
	float Gamma = glm::exp(-1.0f / (2.0f * Sqr(sigma)) * Sqr(glm::sqrt(k / kp) - 1.0f));

	// Jp - JONSWAP spectrum
	float Jp = glm::pow(gamma, Gamma); // Eq 3
	// Fm - long-wave side effect function
	float Fp = Lpm * Jp * glm::exp(- w / glm::sqrt(10.0f) * (glm::sqrt(k / kp) - 1.0f)); // Eq 32
	float alphap = 0.006f * glm::sqrt(w); // Eq 34

	// Bl - long-wave curvature spectrum
	float Bl = 0.5f * alphap * cp / c * Fp; // Eq 31

	float alpham = 0.01f * (u_star < cm ? 1.0f + glm::log(u_star / cm) : 1.0f + 3.0f * glm::log(u_star / cm)); // Eq 44
	// Fm - short-wave side effect function
	float Fm = glm::exp(-0.25f * Sqr(k / km - 1.0f)); // Eq 41
													  
	// Bh - short-wave curvature spectrum
	float Bh = 0.5f * alpham * cm / c * Fm * Lpm; // Eq 40 (fixed)

	float a0 = glm::log(2.0f) / 4.0f; float ap = 4.0f; float am = 0.13f * u_star / cm; // Eq 59
	float Delta = glm::tanh(a0 + ap * glm::pow(c / cp, 2.5f) + am * glm::pow(cm / c, 2.5f)); // Eq 57

	// phi - wave spreading function
	float phi = glm::atan(i_WaveVector.x, i_WaveVector.y);

	float unified = 0.0f;

	if (waveDotWind < 0.0f)
	{
		unified = 0.0f;
	}
	else
	{
		Bl *= 2.0f;
		Bh *= 2.0f;

		// added wind direction dependency - Eq 67
		unified = m_WaveAmplitude * (Bl + Bh) * (1.0f + Delta * glm::cos(2.0f * phi)) * waveDotWind / (glm::two_pi<float>() * Sqr(Sqr(k))); // Eq 67
	}


	return unified;
}

float FFTOceanPatchBase::Omega ( float i_K, float i_KM )
{
	return glm::sqrt(PhysicsConstants::kG * i_K * (1.0f + Sqr(i_K / i_KM))); // Eq 24
}

float FFTOceanPatchBase::Sqr ( float i_X )
{
	return i_X * i_X;
}

float FFTOceanPatchBase::UniformRandomVariable ( void )
{
	// generates a random number between 0.0f and 1.0f
	return static_cast<float>(std::rand()) / RAND_MAX;
}

// Gaussian random number generator with mean 0 and standard deviation 1
glm::vec2 FFTOceanPatchBase::GaussianRandomVariable ( void )
{
	/* Keith Lantz implementation
	check the source code:
	https://www.keithlantz.net/2011/11/ocean-simulation-part-two-using-the-fast-fourier-transform/
	*/
	float x1 = 0.0f, x2 = 0.0f, w = 0.0f;
	do {
		x1 = 2.0f * UniformRandomVariable() - 1.0f;
		x2 = 2.0f * UniformRandomVariable() - 1.0f;
		w = x1 * x1 + x2 * x2;
	} while (w >= 1.0f);

	w = glm::sqrt((-2.0f * glm::log(w)) / w);

	return glm::vec2(x1 * w, x2 * w);
}

float FFTOceanPatchBase::DispersionFrequency ( const glm::vec2& i_WaveVector )
{
	float w0 = glm::two_pi<float>() / m_DispersionFrequencyTimePeriod;

	return glm::floor(glm::sqrt(PhysicsConstants::kG * glm::length(i_WaveVector)) / w0) * w0;
}

void FFTOceanPatchBase::EvaluateWaves ( float i_CrrTime )
{
	/////// Compute Height Map

	// The Hightmap will be evaluated in the derived class!!!

	//////// Compute Normal gradients and folding factor
	if (m_pNormalGradientFolding)
	{
		m_pNormalGradientFolding->ComputeNormalGradientFolding();
	}
}

float FFTOceanPatchBase::ComputeWaterHeightAt ( const glm::vec2& i_XZ ) const
{
	//stub
	return 0.0f;
}

void FFTOceanPatchBase::BindFFTWaveDataTexture ( void ) const
{
	//stub
}

void FFTOceanPatchBase::BindNormalFoldingTexture ( void ) const
{
	if (m_pNormalGradientFolding)
	{
		m_pNormalGradientFolding->BindTexture();
	}
}

unsigned short FFTOceanPatchBase::GetFFTWaveDataTexUnitId ( void ) const
{
	//stub
	return 0;
}

unsigned short FFTOceanPatchBase::GetNormalGradientFoldingTexUnitId ( void ) const
{
	unsigned short val = 0;

	if (m_pNormalGradientFolding)
	{
		val = m_pNormalGradientFolding->GetTexUnitId();
	}

	return val;
}

float FFTOceanPatchBase::GetWaveAmplitude ( void ) const
{
	return m_WaveAmplitude;
}

unsigned short FFTOceanPatchBase::GetPatchSize ( void ) const
{
	return m_PatchSize;
}

float FFTOceanPatchBase::GetWindSpeed ( void ) const
{
	return m_WindSpeed;
}

float FFTOceanPatchBase::GetWindDirectionX ( void ) const
{
	return m_WindDirection.x;
}

float FFTOceanPatchBase::GetWindDirectionZ ( void ) const
{
	return m_WindDirection.y;
}

glm::vec3 FFTOceanPatchBase::GetWindDir ( void ) const
{
	return glm::vec3(m_WindDirection.x, 0.0f, m_WindDirection.y);
}

float FFTOceanPatchBase::GetOpposingWavesFactor ( void ) const
{
	return m_OpposingWavesFactor;
}

float FFTOceanPatchBase::GetVerySmallWavesFactor ( void ) const
{
	return m_VerySmallWavesFactor;
}

float FFTOceanPatchBase::GetChoppyScale ( void ) const
{
	return m_ChoppyScale;
}

float FFTOceanPatchBase::GetTileScale ( void ) const
{
	return m_TileScale;
}


void FFTOceanPatchBase::SetWaveAmplitude ( float i_WaveAmplitude )
{
	m_WaveAmplitude = i_WaveAmplitude;
	SetFFTData();
}

void FFTOceanPatchBase::SetPatchSize ( unsigned short i_PatchSize )
{
	m_PatchSize = i_PatchSize;
	SetFFTData();

	if (m_pNormalGradientFolding)
	{
		m_pNormalGradientFolding->SetPatchSize(i_PatchSize);
	}
}

void FFTOceanPatchBase::SetWindSpeed ( float i_WindSpeed )
{
	m_WindSpeed = i_WindSpeed;
	SetFFTData();
}

void FFTOceanPatchBase::SetWindDirectionX ( float i_WindDirectionX )
{
	m_WindDirection.x = i_WindDirectionX;
	SetFFTData();
}

void FFTOceanPatchBase::SetWindDirectionZ ( float i_WindDirectionZ )
{
	m_WindDirection.y = i_WindDirectionZ;
	SetFFTData();
}

void FFTOceanPatchBase::SetOpposingWavesFactor ( float i_OpposingWavesFactor )
{
	m_OpposingWavesFactor = i_OpposingWavesFactor;
	SetFFTData();
}

void FFTOceanPatchBase::SetVerySmallWavesFactor ( float i_VerySmallWavesFactor )
{
	m_VerySmallWavesFactor = i_VerySmallWavesFactor;
	SetFFTData();
}

void FFTOceanPatchBase::SetChoppyScale ( float i_ChoppyScale )
{
	m_ChoppyScale = i_ChoppyScale;

	if (m_pNormalGradientFolding)
	{
		m_pNormalGradientFolding->SetChoppyScale(i_ChoppyScale);
	}
}

void FFTOceanPatchBase::SetTileScale ( float i_TileScale )
{
	m_TileScale = i_TileScale;
}