/* Author: BAIRAC MIHAI */

#ifndef CAMERA_H
#define CAMERA_H

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

#include <string>

////////// Free Flying Camera with optional constrains /////////

class GlobalConfig;

/*
 FPS Free Camera implementation
 Used on several purposes, not only as a viewing camera,
 but also for observation, debugging and projection of the ocean grid

 based on : http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
 and on http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/ 

 9.005 Are OpenGL matrices column-major or row-major?
 For programming purposes, OpenGL matrices are 16-value arrays with base vectors laid out contiguously in memory.
 The translation components occupy the 13th, 14th, and 15th elements of the 16-element matrix, where indices are
 numbered from 1 to 16 as described in section 2.11.2 of the OpenGL 2.1 Specification.
 Column-major versus row-major is purely a notational convention. Note that post-multiplying with column-major
 matrices produces the same result as pre-multiplying with row-major matrices. The OpenGL Specification and the OpenGL
 Reference Manual both use column-major notation. You can use any notation, as long as it's clearly stated.

 Sadly, the use of column-major format in the spec and blue book has resulted in endless confusion in the OpenGL
 programming community. Column-major notation suggests that matrices are not laid out in memory as a programmer would expect.

 More info here:  http://stackoverflow.com/questions/17717600/confusion-between-c-and-opengl-matrix-order-row-major-vs-column-major

  !!! ORDERING CONVENTION
  GLM uses column major ordering by default, and all vectors are represented as column vectors.
  !!!
*/

class Camera
{
private:
	//// Variables ////
	std::string m_Name;

	// POSITION, TARGET
	// glm types self init !!!
	glm::vec3 m_Position; // position in world space
	glm::vec3 m_Forward;
	glm::vec3 m_Right;
	glm::vec3 m_Up;

	// ORIENTATION
	float m_Pitch;
	float m_Yaw;

	// MATRICES
	// glm types self init !!!
	glm::mat4 m_View; // world -> camera
	glm::mat4 m_Projection; // camera -> clip
	glm::mat4 m_ProjectionView; // world -> clip
	glm::mat4 m_InverseView; // camera -> world
	glm::mat4 m_InverseProjection; // clip -> camera
	glm::mat4 m_InverseProjectionView; // clip -> world

	// Projection
	float m_FOVy, m_InitialFOVy;
	float m_ZNear;
	float m_ZFar;

	bool m_UseConstraints;

	float ComputePerspectiveProjectionCorrectionFactor ( void ) const;

public:
	enum CAMERA_DIRECTIONS 
	{
		CD_FORWARD = 0,
		CD_BACKWARD,
		CD_RIGHT, 
		CD_LEFT, 
		CD_UP,
		CD_DOWN,
		CD_COUNT
	};

	//// Methods ////
	Camera ( void );
	Camera ( const std::string& i_Name );
	Camera ( const std::string& i_Name, const GlobalConfig& i_Config );
	Camera ( Camera* i_pCamera );
	Camera ( const Camera& i_Camera );
	Camera& operator= ( const Camera& i_Camera );
	~Camera ( void );

	void Copy ( Camera* i_pCamera );
	void Copy ( const Camera& i_Camera );

	void UpdateViewMatrix ( void );
	void UpdatePerspectiveProjectionMatrix ( int i_WindowWidth, int i_WindowHeight );
	void UpdatePerspectiveProjectionMatrix ( float i_Fovy, float i_Aspect, float i_ZNear, float i_ZFar );
	void UpdateOrthographicProjectionMatrix ( float i_Left, float i_Right, float i_Bottom, float i_Top, float i_ZNear, float i_ZFar );
	void UpdateOrientationWithMouse ( float i_Dx, float i_Dy );
	void UpdatePositionWithKeyboard ( float i_Value, Camera::CAMERA_DIRECTIONS i_Dir );

	//// Getters ////
	const glm::vec3& GetPosition ( void ) const;
	float GetAltitude ( void ) const;
	const glm::vec3& GetForward ( void ) const;
	const glm::vec3& GetRight ( void ) const;
	const glm::vec3& GetUp ( void ) const;

	float GetPitch ( void ) const;
	float GetYaw ( void ) const;
	float GetFOV ( void ) const;

	const glm::mat4& GetViewMatrix ( void ) const;
	const glm::mat4& GetProjectionMatrix ( void ) const;
	const glm::mat4& GetProjectionViewMatrix ( void ) const;
	const glm::mat4& GetInverseViewMatrix ( void ) const;
	const glm::mat4& GetInverseProjectionMatrix ( void ) const;
	const glm::mat4& GetInverseProjectionViewMatrix ( void ) const;

	//// Setters ////
	void SetPosition ( const glm::vec3& i_Position );
	void SetAltitude ( float i_Altitude);
	void SetForward ( const glm::vec3& i_Forward );
	void SetRight ( const glm::vec3& i_Right );
	void SetUp ( const glm::vec3& i_Up );

	void SetFOV ( float i_FOV );
	void ResetFOV ( void );

	void SetProjectionMatrix ( const glm::mat4& i_ProjectionMatrix );
};


#endif /* CAMERA_H */
