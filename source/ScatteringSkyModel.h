/* Author: BAIRAC MIHAI

 Implementation based on this Light Scattering paper
 paper: https://www.gamedev.net/forums/topic/461747-atmospheric-scattering-sean-oneill---gpu-gems2/
 Lcense: GameDev.net Open License

*/

#ifndef SCATTERING_SKY_MODEL_H
#define SCATTERING_SKY_MODEL_H

#include "BaseSkyModel.h"
#include "glm/mat4x4.hpp"
#include "Camera.h"
#include "FrameBufferManager.h"
#include <string>
#include <vector>
#include <map>

class GlobalConfig;
class Camera;

/*
 Scattering Sky Model
 This Sky Model is implemented based on Sean O'Neil 2004 atmosphere scattering paper
 More info here: https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter16.html
 and here: https://www.gamedev.net/forums/topic/461747-atmospheric-scattering-sean-oneill---gpu-gems2/
*/

class ScatteringSkyModel : public BaseSkyModel
{
public:
	ScatteringSkyModel(void);
	ScatteringSkyModel(const GlobalConfig& i_Config);
	~ScatteringSkyModel(void);

	void Initialize(const GlobalConfig& i_Config) override;

	void Update(const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime) override;
	void UpdateReflected(const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime) override;
	void UpdateRefracted(const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime) override;

	void Render(void) override;
	void RenderReflected(void) override;
	void RenderRefracted(void) override;

	void SetSunDirection(float i_Phi, float i_Theta) override;

	void SetEnabledClouds(bool i_Value) override;
	bool GetEnabledClouds(void) const override;

	void SetCloudsOctaves(unsigned short i_Octaves) override;
	unsigned short GetCloudsOctaves(void) const override;
	void SetCloudsLacunarity(float i_Lacunarity) override;
	float GetCloudsLacunarity(void) const override;
	void SetCloudsGain(float i_Gain) override;
	float GetCloudsGain(void) const override;
	void SetCloudsScaleFactor(float i_ScaleFactor) override;
	float GetCloudsScaleFactor(void) const override;

private:
	//// Methods ////
	void SetupSkyShaders(const GlobalConfig& i_Config, std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& o_Attributes);
	void SetupCloudsShaders(const GlobalConfig& i_Config, std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& o_Attributes);

	void SetupSkyGeometry(const GlobalConfig& i_Config, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_Attributes);
	void SetupCloudsGeometry(const GlobalConfig& i_Config, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_Attributes);

	void UpdateInternal(const glm::mat4& i_ModelMatrix, bool i_ApplyHDR, const Camera& i_Camera, bool i_IsReflMode, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime);
	void RenderInternal(void);

	void Destroy(void);

	//// Variables ////
	ShaderManager m_CloudsSM;
	MeshBufferManager m_CloudsMBM;

	// self init
	// name, location
	std::map<std::string, int> m_CloudsUniforms;

	float m_AtmosphereInnerRadius;

	struct CloudsData
	{
		unsigned short Octaves;
		float Lacunarity;
		float Gain;
		float ScaleFactor;
	} m_CloudsData;

	bool m_AreCloudsEnabled;
};

#endif /* SCATTERING_SKY_MODEL_H */