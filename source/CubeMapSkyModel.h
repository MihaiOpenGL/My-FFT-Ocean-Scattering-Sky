/* Author: BAIRAC MIHAI */

#ifndef CUBE_MAP_SKY_MODEL_H
#define CUBE_MAP_SKY_MODEL_H

#include "BaseSkyModel.h"
#include <string>
#include <vector>
#include <map>
#include "glm/mat4x4.hpp"
#include "Camera.h"
#include "TextureManager.h"
#include "FrameBufferManager.h"

class GlobalConfig;

/* 
   Cube Map Sky Model
   This Sky Model is implemented using the simple CubeMap concept
   More info here: https://learnopengl.com/#!Advanced-OpenGL/Cubemaps
*/

class CubeMapSkyModel : public BaseSkyModel
{
public:
	CubeMapSkyModel(void);
	CubeMapSkyModel(const GlobalConfig& i_Config);
	~CubeMapSkyModel(void);

	void Initialize(const GlobalConfig& i_Config) override;

	void Update(const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime) override;
	void UpdateReflected(const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime) override;
	void UpdateRefracted(const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime) override;

	void Render(void) override;
	void RenderReflected(void) override;
	void RenderRefracted(void) override;

	void SetSunDirection(float i_Phi, float i_Theta) override;

private:
	//// Methods ////
	void SetupShaders(const GlobalConfig& i_Config, std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& o_Attributes);
	void SetupGeometry(const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_Attributes);
	void SetupTextures(const GlobalConfig& i_Config);

	void UpdateInternal(const glm::mat4& i_ModelMatrix, bool i_ApplyHDR, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime);
	void RenderInternal(void);

	void Destroy(void);

	// Variables
	TextureManager m_TM;
};

#endif /* CUBE_MAP_SKY_MODEL_H */