/* Author: BAIRAC MIHAI */

#ifndef MOTOR_BOAT_H
#define MOTOR_BOAT_H

#define GLM_SWIZZLE //offers the possibility to use: .xx(), xy(), xyz(), ...
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "MeshBufferManager.h"
#include "FrameBufferManager.h"
#include "Model.h"
#include "Ocean.h"
#include <string>
#include <vector>
#include <map>

class GlobalConfig;

/*
 Simple implementation of a motor boat
 Involves boat movement
 Buoyancy upon the boat is handled externally
*/

class MotorBoat
{
public:
	MotorBoat(void);
	MotorBoat(const GlobalConfig& i_Config);
	~MotorBoat(void);

	void Initialize(const GlobalConfig& i_Config);

	void Update(const Camera& i_Camera, const glm::vec3& i_SunDirection, bool i_IsWireframeMode, float i_CrrTime);
	void UpdateReflected(const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, const glm::vec3& i_SunDirection, bool i_IsWireframeMode, float i_CrrTime);
	void UpdateRefracted(const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, const glm::vec3& i_SunDirection, bool i_IsWireframeMode, float i_CrrTime);

	void Render(void);
	void RenderReflected(void);
	void RenderRefracted(void);
	void RenderFlattened(void);

	void Accelerate(float i_DeltaTime);
	void Decelerate(float i_DeltaTime);
	void TurnRight(float i_DeltaTime);
	void TurnLeft(float i_DeltaTime);

	void BindPropellerWashTexture() const;

	const Ocean::BoatPropellerWashData& GetPropellerWashData(void) const;
	const Ocean::BoatKelvinWakeData& GetKelvinWakeData(void) const;

	const Model::Dimensions& GetBoatDimensions(void) const;
	float GetBoatWidth(void) const;
	float GetBoatHeight(void) const;
	float GetBoatDepth(void) const;
	float GetBoatDensity(void) const;
	float GetBoatArea(void) const;
	float GetBoatVolume(void) const;
	float GetBoatMass(void) const;
	float GetBoatDragCoefficient(void) const;
	float GetBoatYAccelerationFactor(void) const;

	const glm::vec3& GetBoatPosition(void) const;
	float GetBoatYPos(void) const;
	float GetBoatVelocity(void) const;

	void SetBoatYPos(float i_BoatYPos);

private:
	//// Methods ////
	void UpdateInternal(const glm::mat4& i_ScaleMatrix, bool i_ApplyBoatPositionCorrection, bool i_ApplyHDR, const Camera& i_Camera, const glm::vec3& i_SunDirection, bool i_IsWireframeMode, float i_CrrTime);

	void RenderInternal(void);
	void RenderTrail(void);

	void Destroy(void);

	//// Constants /////
	static const short m_kBoatTrailVertexCount;

	//// Variables ////
	ShaderManager m_SM, m_TrailSM;
	TextureManager m_TM;
	Model m_M;
	FrameBufferManager m_TrailFBM;
	MeshBufferManager m_TrailMBM;

	// self init
	// name, location
	std::map<std::string, int> m_Uniforms;
	std::map<std::string, int> m_TrailUniforms;

	glm::vec3 m_BoatCurrentPosition;
	float m_BoatVelocity;
	float m_BoatTurnAngle;
	Ocean::BoatPropellerWashData m_BoatProperllerWashData;
	Ocean::BoatKelvinWakeData m_BoatKelvinWakeData;


	///////////
	std::vector<MeshBufferManager::VertexData> m_TrailVertexData;
	unsigned short m_CrrPropellerPosIdx;
	glm::vec3 m_PrevPropellerPos;

	glm::vec3 m_BoatAxis;
	float m_KelvinWakeOffset;
	float m_PropellerWashOffset;
	float m_PropellerWashWidth;

	float m_AccelerationFactor;
	float m_TurnAngleFactor;

	float m_KelvinWakeDisplacementFactor;
	float m_FoamAmountFactor;

	bool m_EnableBoatFoam;
	bool m_EnableBoatKelvinWake;
	bool m_EnableBoatPropellerWash;
	bool m_IsWireframeMode;

	float m_BoatArea;
	float m_BoatVolume;
	float m_BoatMass;
	float m_BoatDensity;
	float m_BoatDragCoefficient;
	float m_BoatYAccelerationFactor;
};

#endif /* MOTOR_BOAT_H */