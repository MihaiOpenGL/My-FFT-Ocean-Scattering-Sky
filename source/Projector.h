/* Author: BAIRAC MIHAI

 Implemntation based on Claes Johanson work
 paper: http://fileadmin.cs.lth.se/graphics/theses/projects/projgrid/
 License: check the demo package

*/

#ifndef PROJECTOR_H
#define PROJECTOR_H

#include <vector>
#define GLM_SWIZZLE //offers the possibility to use: .xx(), xy(), xyz(), ...
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "Camera.h"

class GlobalConfig;

/*
 Projector class - provides a way to have an infinite ocean

 Based on Claes Johanson paper:
 http://fileadmin.cs.lth.se/graphics/theses/projects/projgrid/
*/

class Projector
{
public:
	enum class PROJ_TYPE
	{
		PT_SURFACE = 0,
		PT_UNDERWATER,
		PT_COUNT
	};

	Projector(void);
	Projector(const GlobalConfig& i_Config, Projector::PROJ_TYPE i_Type);
	~Projector(void);

	void Initialize(const GlobalConfig& i_Config, Projector::PROJ_TYPE i_Type);

	void Update(const Camera& i_RenderingCamera);

	glm::mat4 ComputeGridCorners(void);

	void SetDisplacementWaveAmplitude(void);

	const glm::mat4& GetProjectingMatrix(void) const;
	const Camera& GetProjectingCamera(void)  const;

	const glm::vec4& GetPlane(void) const;
	float GetPlaneDistance(void) const;

	bool IsPlaneWithinFrustum(void) const;
	bool IsUnderMainPlane(void) const;

private:
	//// Methods ////
	void Destroy(void);

	bool ComputeGridCornerPosition(const glm::vec2& i_UV, glm::vec4& o_GridCornerPos);

	//// Variables ////
	glm::vec3 m_Position, m_Normal;
	glm::vec4 m_Plane, m_UpperPlane, m_LowerPlane; //m_Plane - base plane
	glm::mat4 m_ProjectingMatrix;
	// Pointers to Camera class are not needed, because the class is small
	Camera m_ProjectingCamera;
	bool m_IsPlaneWithinFrustum;
	bool m_IsFrustumVisible;

	float m_MaxWaveAmplitude;
	float m_StrengthElevation;
	float m_AimPointCorrection;

	Projector::PROJ_TYPE m_Type;
	bool m_IsUnderMainPlane;
};

#endif /* PROJECTOR_H */