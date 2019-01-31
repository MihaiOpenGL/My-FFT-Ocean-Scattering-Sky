/* Author: BAIRAC MIHAI */

#ifndef SKY_H
#define SKY_H

#include <string>
#include <map>
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "CustomTypes.h"

class GlobalConfig;
class BaseSkyModel;
class Camera;

/*
 General class for Sky 
 It manages all the Sky Models:
 Cubemap
 Scattering
 PrecomputedScattering
*/

class Sky
{
public:
	Sky(void);
	Sky(const GlobalConfig& i_Config);
	~Sky(void);

	void Initialize(const GlobalConfig& i_Config);

	void Update(const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime);
	void UpdateReflected(const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime);
	void UpdateRefracted(const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime);

	void Render(void);
	void RenderReflected(void);
	void RenderRefracted(void);

	void SetSunDirection(float i_Phi, float i_Theta);
	glm::vec3 GetSunDirection(void) const;

	bool GetAllowChangeDirWithMouse(void) const;

	void SetEnabledClouds(bool i_Value);
	bool GetEnabledClouds(void) const;

	void SetCloudsOctaves(unsigned short i_Octaves);
	unsigned short GetCloudsOctaves(void) const;
	void SetCloudsLacunarity(float i_Lacunarity);
	float GetCloudsLacunarity(void) const;
	void SetCloudsGain(float i_Gain);
	float GetCloudsGain(void) const;
	void SetCloudsScaleFactor(float i_ScaleFactor);
	float GetCloudsScaleFactor(void) const;
	void SetCloudsNorm(float i_Norm);
	float GetCloudsNorm(void) const;

	CustomTypes::Sky::ModelType GetModelType(void) const;

private:
	//// Methods ////
	void Destroy(void);

	//// variables ////
	BaseSkyModel* m_pSkyModel;

	CustomTypes::Sky::ModelType m_ModelType;
};

#endif /* SKY_H */