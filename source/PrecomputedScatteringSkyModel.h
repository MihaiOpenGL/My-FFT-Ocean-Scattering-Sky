/* Author: BAIRAC MIHAI

 Implementation based on Eric Bruneton'w work
 Precomputed Atmosphere Scattering paper
 paper: http://www-ljk.imag.fr/Publications/Basilic/com.lmc.publi.PUBLI_Article@11e7cdda2f7_f64b69/article.pdf
 License: check the package

*/

#ifndef PRECOMPUTED_SCATTERING_SKY_MODEL_H
#define PRECOMPUTED_SCATTERING_SKY_MODEL_H

#include "BaseSkyModel.h"
#include "glm/mat4x4.hpp"
#include "Camera.h"
#include "TextureManager.h"
#include <string>
#include <vector>
#include <map>

class GlobalConfig;
class Camera;

/* 
 Precomputed Scattering Sky Model
 This Sky Model is implemented based on Eric Bruneton old implementation of the 
 2008 precomputed atmosphere scattering paper
 More info here: http://www-ljk.imag.fr/Publications/Basilic/com.lmc.publi.PUBLI_Article@11e7cdda2f7_f64b69/article.pdf
 New implementation here: https://ebruneton.github.io/precomputed_atmospheric_scattering/
 Ocean Lighting: https://hal.inria.fr/inria-00443630/file/article-1.pdf
 Eric's Evasion work: http://www-evasion.imag.fr/people/Eric.Bruneton/
*/

class PrecomputedScatteringSkyModel : public BaseSkyModel
{
public:
	PrecomputedScatteringSkyModel(void);
	PrecomputedScatteringSkyModel(const GlobalConfig& i_Config);
	~PrecomputedScatteringSkyModel(void);

	void Initialize(const GlobalConfig& i_Config) override;

	void Update(const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime) override;
	void UpdateReflected(const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime) override;
	void UpdateRefracted(const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime) override;

	void UpdateSunPosition(float i_CrrTime) override;

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
	void SetCloudsNorm(float i_Norm) override;
	float GetCloudsNorm(void) const override;

private:
	//// Methods ////
	void SetupSkyShaders(const GlobalConfig& i_Config, std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& o_Attributes);
	void SetupCloudsShaders(const GlobalConfig& i_Config, std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& o_Attributes);

	void SetupSkyGeometry(const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_Attributes);
	void SetupCloudsGeometry(const GlobalConfig& i_Config, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_Attributes);

	void SetupTextures(const GlobalConfig& i_Config);

	void UpdateInternal(float i_RotateAngle, bool i_ApplyCloudsCorrection, bool i_ApplySunDirCorrection, const Camera& i_Camera, bool i_IsReflMode, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime);
	void RenderInternal(bool i_RevertWinding = false);

	void Destroy(void);

	// Variables

	// self init
	// name, location
	ShaderManager m_CloudsSM;
	MeshBufferManager m_CloudsMBM;
	TextureManager m_TM;

	std::map<std::string, int> m_CloudsUniforms;

	struct CloudsData
	{
		unsigned short Octaves;
		float Lacunarity;
		float Gain;
		float Norm;
	} m_CloudsData;

	bool m_AreCloudsEnabled;
};

#endif /* PRECOMPUTED_SCATTERING_SKY_MODEL_H */