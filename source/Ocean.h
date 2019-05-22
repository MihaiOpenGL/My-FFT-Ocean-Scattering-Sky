/* Author: BAIRAC MIHAI */

#ifndef OCEAN_H
#define OCEAN_H

#include "CustomTypes.h"
#include "ShaderManager.h"
#include "MeshBufferManager.h"
#include "FrameBufferManager.h"
#include "TextureManager.h"
#include "Projector.h"
//#define GLM_SWIZZLE //offers the possibility to use: xx(), xy(), xyz(), ...
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include <string>
#include <vector>
#include <map>

class Camera;
class MotorBoat;
class FFTOceanPatchBase;
class GlobalConfig;

/*
 FFT Ocean implementation
 It manages all the FFT ocean patches, surface effects like: foam, reflections, refractions, boat effects
 there are also bottom effects like caustics and fog, and also under water god rays effect

 More info about FFT on ocean simulation:
 Jerry Tessendorf paper: http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.161.9102&rep=rep1&type=pdf
 nVidia DirectX 11 SDK Ocean demo: https://developer.nvidia.com/dx11-samples
 Keith Lantz ocean demo: http://www.keithlantz.net/2011/10/ocean-simulation-part-one-using-the-discrete-fourier-transform/
 Wikipedia: https://en.wikipedia.org/wiki/Discrete_Fourier_transform
 Paul Bourke DFT: http://paulbourke.net/miscellaneous/dft/

 More water info: http://www.digitalrune.com/Support/Blog/tabid/719/EntryId/210/Water-Rendering.aspx
*/

class Ocean
{
public:
	struct BoatPropellerWashData
	{
		float DistortFactor;
	};

	struct BoatKelvinWakeData
	{
		glm::vec3 BoatPosition;
		glm::vec3 WakePosition;
		float Amplitude;
		float FoamAmount;
	};

	Ocean(void);
	Ocean(const GlobalConfig& i_Config);
	~Ocean(void);

	void Initialize(const GlobalConfig& i_Config);

	void Update(const Camera& i_Camera, const glm::vec3& i_SunDirection, bool i_IsWireframeMode, bool i_IsFrustumVisible, float i_CrrTime);
	void UpdateGrid(unsigned short i_WindowWidth, unsigned short i_WindowHeight);
	void UpdateBoatEffects(const MotorBoat& i_MotorBoat);

	void Render(const Camera& i_CurrentViewingCamera);

	float ComputeWaterHeightAt(const glm::vec2& i_XZ);
	float ComputeAverageWaterHeightAt(const glm::vec2& i_XZ, const glm::ivec2& i_Zone);

	float GetWaveAmplitude(void) const;
	unsigned short GetPatchSize(void) const;
	float GetWindSpeed(void) const;
	float GetWindDirectionX(void) const;
	float GetWindDirectionZ(void) const;
	glm::vec3 GetWindDir(void) const;
	float GetOpposingWavesFactor(void) const;
	float GetVerySmallWavesFactor(void) const;
	float GetChoppyScale(void) const;
	float GetTileScale(void) const;

	unsigned short GetGodRaysNumberOfSamples(void) const;
	float GetGodRaysExposure(void) const;
	float GetGodRaysDecay(void) const;
	float GetGodRaysDensity(void) const;
	float GetGodRaysWeight(void) const;

	bool IsUnderWater(void) const;

	void SetWaveAmplitude(float i_WaveAmplitude);
	void SetPatchSize(unsigned short i_PatchSize);
	void SetWindSpeed(float i_WindSpeed);
	void SetWindDirectionX(float i_WindDirectionX);
	void SetWindDirectionZ(float i_WindDirectionZ);
	void SetOpposingWavesFactor(float i_OpposingWavesFactor);
	void SetVerySmallWavesFactor(float i_VerySmallWavesFactor);
	void SetChoppyScale(float i_ChoppyScale);
	void SetTileScale(float i_TileScale);

	void SetGodRaysNumberOfSamples(unsigned short i_NumberOfSamples);
	void SetGodRaysExposure(float i_Exposure);
	void SetGodRaysDecay(float i_Decay);
	void SetGodRaysDensity(float i_Density);
	void SetGodRaysWeight(float i_Weight);

private:
	//// Methods ////
	void SetupGrid(const GlobalConfig& i_Config);
	void SetupOceanSurface(const GlobalConfig& i_Config);
	void SetupScreenSpaceGrid(unsigned short i_WindowWidth, unsigned short i_WindowHeight);

	void SetupOceanBottom(const GlobalConfig& i_Config);
	void SetupOceanBottomCaustics(const GlobalConfig& i_Config);
	void SetupOceanBottomGodRays(const GlobalConfig& i_Config);
	void SetupDebugFrustum(const GlobalConfig& i_Config);
	void SetupTextures(const GlobalConfig& i_Config);

	void UpdateOceanSurface(const Camera& i_Camera, const glm::vec3& i_SunDirection, float i_CrrTime);
	void UpdateOceanBottom(const Camera& i_Camera, const glm::vec3& i_SunDirection, glm::mat4& o_BottomGridCorners);
	void UpdateOceanBottomCaustics(const glm::mat4& i_BottomGridCorners, const glm::vec3& i_SunDirection);
	void UpdateOceanBottomGodRays(const Camera& i_Camera, const glm::vec3& i_SunDirection);
	void UpdateDebugFrustum(const Camera& i_Camera);

	void RenderOceanSurface(const Camera& i_CurrentViewingCamera);
	void RenderOceanBottom(const Camera& i_CurrentViewingCamera);
	void RenderOceanBottomCaustics(const Camera& i_CurrentViewingCamera);
	void RenderOceanBottomGodRays(void);
	void RenderDebugFrustum(const Camera& i_CurrentViewingCamera);

	void Destroy(void);

	//// Projector
	Projector m_WaveProjector, m_BottomProjector;

	//// FFT Ocen Patch
	FFTOceanPatchBase* m_pFFTOceanPatch;

	//// Variables ////
	ShaderManager m_OceanSurfaceSM, m_OceanBottomSM, m_OceanCausticsSM, m_OceanOccluderSM, m_OceanGodRaysSM;
	MeshBufferManager m_GridMBM, m_OceanSurfaceMBM, m_OceanBottomMBM, m_OceanCausticsMBM , m_OceanOccluderMBM;
	FrameBufferManager m_OceanCausticsFBM, m_OceanGodRaysFBM;
	TextureManager m_OceanTM;

	unsigned int m_GridVertexCount, m_GridIndexCount;
	unsigned int m_OccluderIndexCount;
	float m_ScreenSpaceGridResolution;

	unsigned short m_GodRaysMapWidth, m_GodRaysMapHeight;
	unsigned short m_CausticsMapSize;

	float m_PerlinNoiseSpeed;

	// self init
	// name, location
	std::map<std::string, int> m_OceanSurfaceUniforms;
	std::map<std::string, int> m_OceanBottomUniforms;
	std::map<std::string, int> m_OceanCausticsUniforms;
	std::map<std::string, int> m_OceanOccluderUniforms;
	std::map<std::string, int> m_OceanGodRaysUniforms;

	bool m_IsWireframeMode;

	// View & Projector Frustums
	ShaderManager m_FrustumSM;
	MeshBufferManager m_FrustumMBM;
	std::map<std::string, int> m_FrustumUniforms;

	bool m_IsFrustumVisible;

	CustomTypes::Ocean::GridType m_GridType;
	CustomTypes::Sky::ModelType m_SkyModelType;

	Camera* m_pCurrentCamera;

	struct UnderWaterGodRaysData
	{
		unsigned short NumberOfSamples;
		float Exposure;
		float Decay;
		float Density;
		float Weight;
	} m_UnderWaterGodRaysData;

	float m_SunDirY;

	bool m_SurfaceUseGridCorners;
	bool m_BottomUseGridCorners;
	bool m_EnableUnderWaterGodRays;
	bool m_EnableBottomCaustics;
	bool m_EnableBoatFoam;
	bool m_EnableBoatKelvinWake;
	bool m_EnableBoatPropellerWash;
};

#endif /* OCEAN_H */