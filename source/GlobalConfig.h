/* Author: BAIRAC MIHAI */

#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#include "CustomTypes.h"
#include "XMLParser.h"

#include <string>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

/*
 This class implements the Global Config system
 It basically consists of a multi-layer struct which holds all the neededdata and parameters
 Its inputs is XMLParser output - the config file is a xml file
*/

class GlobalConfig
{
public:
	// Project

	struct Window
	{
		bool UseWindowHints;
		bool IsWindowMode;
		bool IsWindowResizable;
	} Window;

	struct OpenGLContext
	{
		bool IsCoreProfile;

		struct OpenGLVersion
		{
			short major;
			short minor;
		} OpenGLVersion;

	} OpenGLContext;

	struct Input
	{
		float KeySpeed;
		float MouseSpeed;
	} Input;

	struct Camera
	{
		bool UseConstraints;
		glm::vec3 InitialPosition;
		float InitialPitch;
		float InitialYaw;
		float InitialFieldOfView;
		float InitialZNear;
		float InitialZFar;
	} Camera;

	struct Simulation
	{
		bool ShowGUI;
		float TimeScale;
	} Simulation;

	struct Rendering
	{
		struct HDR
		{
			bool Enabled;
			float Exposure;
		} HDR;
	} Rendering;

	struct Shaders
	{
		bool UseStrictVerification;
	} Shaders;

	struct TexUnit
	{
		struct Global
		{
			unsigned short ReflectionMap;
			unsigned short RefractionMap;
			unsigned short PostProcessingMap;
		} Global;

		struct Sky
		{
			struct CubeMapSkyModel
			{
				unsigned short CubeMap;
			} CubeMapSkyModel;

			struct ScatteringSkyModel
			{
			} ScatteringSkyModel;

			struct PrecomputedScatteringSkyModel
			{
				unsigned short IrradianceMap;
				unsigned short InscatterMap;
				unsigned short TransmittanceMap;
				unsigned short NoiseMap;
			} PrecomputedScatteringSkyModel;
		} Sky;

		struct Ocean
		{
			struct CPU2DIFFT
			{
				unsigned short FFTMap;
			} CPU2DIFFT;

			struct GPU2DIFFT
			{
				unsigned short ButterflyMap;
				unsigned short PingArrayMap;
			} GPU2DIFFT;

			struct GPU2DIFFTComp
			{
				unsigned short IndicesMap;
				unsigned short WeightsMap;
				unsigned short PingArrayMap;
			} GPU2DIFFTComp;

			struct FFTNormalGradientFoldingBase
			{
				unsigned short NormalGradientFoldingMap;
			} FFTNormalGradientFoldingBase;

			struct FFTNormalGradientFoldingGPUFrag
			{
			} FFTNormalGradientFoldingGPUFrag;

			struct FFTNormalGradientFoldingGPUComp
			{
			} FFTNormalGradientFoldingGPUComp;

			struct FFTOceanPatchBase
			{
			} FFTOceanPatchBase;

			struct FFTOceanPatchCPUFFTW
			{
			} FFTOceanPatchCPUFFTW;

			struct FFTOceanPatchGPUFrag
			{
				unsigned short FFTInitDataMap;
			} FFTOceanPatchGPUFrag;

			struct FFTOceanPatchGPUComp
			{
				unsigned short FFTInitDataMap;
			} FFTOceanPatchGPUComp;

			struct Surface
			{
				unsigned short PerlinDisplacementMap;
				unsigned short WavesFoamMap;
				unsigned short BoatFoamMap;
				unsigned short PropellerWashMap;
				unsigned short KelvinWakeDispNormMap;
				unsigned short KelvinWakeFoamMap;
			} Surface;

			struct UnderWater
			{
				unsigned short GodRaysMap;
			} UnderWater;

			struct Bottom
			{
				unsigned short SandDiffuseMap;
				unsigned short CausticsMap;
			} Bottom;
		} Ocean;

		struct MotorBoat
		{
			unsigned short BoatDiffMap;
			unsigned short BoatNormalMap;
		} MotorBoat;
	} TexUnit;

	struct VisualEffects
	{
		bool ShowReflections;
		bool ShowRefractions;

		struct PostProcessing
		{
			bool Enabled;
			CustomTypes::PostProcessing::EffectType EffectType;
		} PostProcessing;
	} VisualEffects;

	struct Scene
	{
		struct Sky
		{
			struct Model
			{
				CustomTypes::Sky::ModelType Type;

				struct Cubemap
				{
					struct Sun
					{
						bool AllowChangeDirWithMouse;
						bool IsDynamic;
						float MoveFactor;
						float InitialPhi;
						float InitialTheta;
						float Shininess;
						float Strength;
						float SunFactor;
					} Sun;
				} Cubemap;

				struct Scattering
				{
					struct Sun
					{
						bool AllowChangeDirWithMouse;
						bool IsDynamic;
						float MoveFactor;
						float InitialPhi;
						float InitialTheta;
						float Shininess;
						float Strength;
					} Sun;

					struct Atmosphere
					{
						short SampleCount;
						float RayleighScatteringConstant;
						float MieScatteringConstant;
						float SunBrightnessConstant;
						glm::vec3 WaveLength;
						float InnerRadius;
						float OuterRadius;
						unsigned short GeometrySliceCount;
						float AltitudeOffset;
					} Atmosphere;

					struct Clouds
					{
						bool Enabled;
						short Octaves;
						float Lacunarity;
						float Gain;
						float ScaleFactor;
						float Offset;
						float Altitude;
					} Clouds;
				} Scattering;

				struct PrecomputedScattering
				{
					struct Sun
					{
						bool AllowChangeDirWithMouse;
						bool IsDynamic;
						float MoveFactor;
						float InitialPhi;
						float InitialTheta;
					} Sun;

					struct Atmosphere
					{
						float SunIntensity;
						float MieScattering;
						glm::vec3 Rgtl;
						glm::vec3 BetaR;
					} Atmosphere;

					struct Clouds
					{
						bool Enabled;
						short Octaves;
						float Lacunarity;
						float Gain;
						float Norm;
						float Clamp1;
						float Clamp2;
						glm::vec3 Color;
						float Offset;
						float Altitude;
						float AltitudeOffset;
					} Clouds;
				} PrecomputedScattering;
			} Model;

			glm::vec3 UnderWaterColor;
		} Sky;

		struct Ocean
		{
			struct Grid
			{
				CustomTypes::Ocean::GridType Type;

				struct WorldSpace
				{
					unsigned short Width;
					unsigned short Height;
				} WorldSpace;

				struct ScreenSpace
				{
					float GridResolution;
				} ScreenSpace;
			} Grid;

			struct Surface
			{
				float MaxFadeAltitude;

				float ReflectionDistortFactor;
				float RefractionDistortFactor;

				glm::vec3 WaterColor;
				glm::vec3 WaterRefrColor;

				struct Projector
				{
					glm::vec3 Position;
					glm::vec3 Normal;
					float MaxWaveAmplitude;
					float StrengthElevation;
					float AimPointCorrection;
					bool UseGridCorners;
				} Projector;

				struct OceanPatch
				{
					short FFTSize;
					short PatchSize;
					float WaveAmpltitude;
					float WindSpeed;
					float WindSpeedMixLimit;
					glm::vec2 WindDirection;
					float DispersionFrequencyTimePeriod;
					float ChoppyScale;
					float TileScale;

					struct ComputeFFT
					{
						CustomTypes::Ocean::ComputeFFTType Type;
						bool UseFFTSlopes;
						bool Use2FBOs;
					} ComputeFFT;

					struct Spectrum
					{	
						CustomTypes::Ocean::SpectrumType Type;
						struct Phillips
						{
							float OpposingWavesFactor;
							float VerySmallWavesFactor;
						} Phillips;

						struct Unified
						{
							float SeaState;
							float MinimumPhaseSpeed;
							float SecondaryGravityCapillaryPeak;
						} Unified;
					} Spectrum;

					struct NormalGradientFolding
					{	
						CustomTypes::Ocean::NormalGradientFoldingType Type;
					} NormalGradientFolding;
				} OceanPatch;

				struct PerlinNoise
				{
					glm::vec3 Amplitudes;
					glm::vec3 Octaves;
					glm::vec3 Gradients;
					float Speed;
				} PerlinNoise;

				struct WaveBlending
				{
					float Begin;
					float End;
				} WaveBlending;

				struct Foam
				{
					bool Enabled;
					float ScaleFactor;
					float CoverageFactor;
				} Foam;

				struct SubSurfaceScattering
				{
					bool Enabled;
					glm::vec3 Color;
					float Scale;
					float Power;
					float WaveHeightScale;
					float MaxAllowedValue;
				} SubSurfaceScattering;

				struct UnderWater
				{
					glm::vec3 Color;

					struct Fog
					{
						bool Enabled;
						glm::vec3 Color;
						float Density;
					} Fog;
				} UnderWater;

				struct BoatEffects
				{
					struct Foam
					{
						bool Enabled;
						float Scale;
					} Foam;

					struct KelvinWake
					{
						bool Enabled;
						float Scale;
					} KelvinWake;

					struct PropellerWash
					{
						bool Enabled;
						float DistortFactor;
					} PropellerWash;

					struct Buoyancy
					{
						bool Enabled;
					} Buoyancy;
					
					bool HideInsideWater;
				} BoatEffects;

			} Surface;

			struct UnderWater
			{
				glm::vec3 Color;

				struct Fog
				{
					bool Enabled;
					glm::vec3 Color;
					float Density;
				} Fog;

				struct GodRays
				{
					bool Enabled;
					unsigned short NumberOfSamples;
					glm::vec3 Color;
					float Exposure;
					float Decay;
					float Density;
					float Weight;

					struct Occluder
					{
						unsigned short Size;
						unsigned short Step;
					} Occluder;
				} GodRays;
			} UnderWater;

			struct Bottom
			{
				float PatchSize;

				struct Fog
				{
					bool Enabled;
					glm::vec3 Color;
					float Density;
				} Fog;

				struct Projector
				{
					glm::vec3 Position;
					glm::vec3 Normal;
					float MaxWaveAmplitude;
					float StrengthElevation;
					float AimPointCorrection;
					bool UseGridCorners;
				} Projector;

				struct Sand
				{
					float Scale;
				} Sand;

				struct PerlinNoise
				{
					float Scale;
					float Amplitude;
				} PerlinNoise;

				struct Caustics
				{
					bool Enabled;
					unsigned short MapSize;
					glm::vec3 Color;
					float Intensity;
					float PlaneDistanceOffset;
					float Scale;
				} Caustics;
			} Bottom;
		} Ocean;

		struct Boat
		{
			glm::vec3 Position;
			float KelvinWakeOffset;
			float PropellerWashOffset;
			float PropellerWashWidth;
			float AccelerationFactor;
			float TurnAngleFactor;
			float KelvinWakeDisplacementFactor;
			float FoamAmountFactor;
			bool UseFlattenedModel;

			float Density;
			float DragCoefficient;
			float YAccelerationFactor;
		} Boat;
	} Scene;

	struct ShaderDefines
	{
		std::string Header;

		std::string HDR;

		struct Ocean
		{
			struct Surface
			{
				std::string GridCorners;
				std::string Foam;
				std::string SSS;
				std::string UnderWaterFog;

				std::string FFTSize;

				struct BoatEffects
				{
					std::string Foam;
					std::string KelvinWake;
					std::string PropellerWash;
				} BoatEffects;

			} Surface;

			struct UnderWater
			{
				std::string Fog;
				std::string GodRays;
			} UnderWater;

			struct Bottom
			{
				std::string GridCorners;
				std::string UnderWaterFog;
				std::string Caustics;
			} Bottom;
		} Ocean;

		std::string GetOptionsString (void) const
		{
			std::string options = HDR + Ocean.Surface.FFTSize + Ocean.Surface.GridCorners + Ocean.Surface.Foam + Ocean.Surface.SSS +
				Ocean.Surface.BoatEffects.Foam + Ocean.Surface.BoatEffects.KelvinWake + Ocean.Surface.BoatEffects.PropellerWash +
				Ocean.Surface.UnderWaterFog + Ocean.UnderWater.Fog + Ocean.UnderWater.GodRays + Ocean.Bottom.GridCorners +
				Ocean.Bottom.UnderWaterFog + Ocean.Bottom.Caustics;

			return options;
		}

	} ShaderDefines;

private:
	XMLParser* m_pConfigParser;

	// Methods
	void Setup ( void );
	void Destroy ( void );

public:
	GlobalConfig ( void );
	GlobalConfig ( const std::string& i_FileName );
	~GlobalConfig ( void );

	void Initialize ( const std::string& i_FileName );
};

#endif /* GLOBAL_CONFIG_H */