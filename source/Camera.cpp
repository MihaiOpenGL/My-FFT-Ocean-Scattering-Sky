/* Author: BAIRAC MIHAI */

#include "Camera.h"
#include "ErrorHandler.h"
#include "GL/glew.h"
// glm::vec3, glm::mat4 come from the header
#include "glm/gtc/matrix_transform.hpp" //perspective(), ortho(), lookAt(), transpose(), inerse()
#include "glm/geometric.hpp" //dot(), normalize(), cross()
#include "glm/trigonometric.hpp" //sin(), cos(), radians()
#include "glm/gtc/constants.hpp" //hlf_pi()
#include "HelperFunctions.h"
#include "GlobalConfig.h"


Camera::Camera ( void )
	: m_Name("Camera"), m_Position(glm::vec3(0.0f)), m_Forward(glm::vec3(0.0f, 0.0f, -1.0f)),
	  m_Right(glm::vec3(+1.0f, 0.0f, 0.0f)), m_Up(glm::vec3(0.0f, +1.0f, 0.0f)),
	  m_Pitch(0.0f), m_Yaw(0.0f), m_FOVy(0.0f), m_InitialFOVy(0.0f), m_ZNear(0.0f), m_ZFar(0.0f),
	  m_UseConstraints(false)
{}

Camera::Camera ( const std::string& i_Name )
	: m_Name(i_Name), m_Position(glm::vec3(0.0f)), m_Forward(glm::vec3(0.0f, 0.0f, -1.0f)),
	  m_Right(glm::vec3(+1.0f, 0.0f, 0.0f)), m_Up(glm::vec3(0.0f, +1.0f, 0.0f)),
	  m_Pitch(0.0f), m_Yaw(0.0f), m_FOVy(0.0f), m_InitialFOVy(0.0f), m_ZNear(0.0f), m_ZFar(0.0f),
	  m_UseConstraints(false)
{}

Camera::Camera ( const std::string& i_Name, const GlobalConfig& i_Config )
	: m_Name(i_Name), m_Forward(glm::vec3(0.0f, 0.0f, -1.0f)),
	  m_Right(glm::vec3(+1.0f, 0.0f, 0.0f)), m_Up(glm::vec3(0.0f, +1.0f, 0.0f))
	{
		m_Position = i_Config.Camera.InitialPosition;
		m_Pitch = i_Config.Camera.InitialPitch;
		m_Yaw = i_Config.Camera.InitialYaw;
		m_InitialFOVy = m_FOVy = i_Config.Camera.InitialFieldOfView;
		m_ZNear = i_Config.Camera.InitialZNear;
		m_ZFar = i_Config.Camera.InitialZFar;
		m_UseConstraints = i_Config.Camera.UseConstraints;
	}

Camera::Camera ( Camera* i_pCamera )
{
	assert(i_pCamera != nullptr);

	SetPosition(i_pCamera->GetPosition());
	SetForward(i_pCamera->GetForward());
	SetRight(i_pCamera->GetRight());
	SetUp(i_pCamera->GetUp());
	SetProjectionMatrix(i_pCamera->GetProjectionMatrix());

	UpdateViewMatrix();
}

Camera::Camera ( const Camera& i_Camera )
{
	SetPosition(i_Camera.GetPosition());
	SetForward(i_Camera.GetForward());
	SetRight(i_Camera.GetRight());
	SetUp(i_Camera.GetUp());
	SetProjectionMatrix(i_Camera.GetProjectionMatrix());

	UpdateViewMatrix();
}

Camera& Camera::operator= ( const Camera& i_Camera )
{
	SetPosition(i_Camera.GetPosition());
	SetForward(i_Camera.GetForward());
	SetRight(i_Camera.GetRight());
	SetUp(i_Camera.GetUp());
	SetProjectionMatrix(i_Camera.GetProjectionMatrix());

	UpdateViewMatrix();

	return *this;
}

Camera::~Camera ( void )
{
//	LOG("[" + m_Name + "] Camera has been destroyed successfully!");
}

void Camera::Copy ( Camera* i_pCamera )
{
	assert(i_pCamera != nullptr);

	SetPosition(i_pCamera->GetPosition());
	SetForward(i_pCamera->GetForward());
	SetRight(i_pCamera->GetRight());
	SetUp(i_pCamera->GetUp());
	SetProjectionMatrix(i_pCamera->GetProjectionMatrix());

	UpdateViewMatrix();
}


void Camera::Copy ( const Camera& i_Camera )
{
	SetPosition(i_Camera.GetPosition());
	SetForward(i_Camera.GetForward());
	SetRight(i_Camera.GetRight());
	SetUp(i_Camera.GetUp());
	SetProjectionMatrix(i_Camera.GetProjectionMatrix());

	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix ( void )
{
	// m_Position + m_Forward - creates the target camera is looking at
	m_View = glm::lookAt(m_Position, m_Position + m_Forward, m_Up);

	// update the other matrices too
	// the view matrix is orthogonal so it's inverse is actually it's transpose
	// the transpose operation is much cheaper than inverse one
	// more info about view matrix: http://www.3dgep.com/understanding-the-view-matrix/
//	m_InverseView = glm::inverse(m_View);
	m_InverseView = glm::transpose(m_View);
	m_ProjectionView = m_Projection * m_View;

	// projection * view matrix is NOT orthogonal
	m_InverseProjectionView = glm::inverse(m_ProjectionView);
}

/* NOTE! This function fixes two major problems with the ocean grid projection!
   a) If camera goes really, really high above the ocean then the grid was cut by the far plane
   The issue was getting worse as the altitude got bigger and bigger!
   Also, this would break realism, because of the visible gap between the ocean grid an the sky horizon
   Below sky horizon it's usually dark/black, so the black gap is really visible!
   b) If camera goes further and further away from the center of the world (0.0, 0.0, 0.0)
   it didn't matter in which direction (x, z) or y
   then the projected matrix/grid far points would vary a lot
   A very disturbinf visual artifact, as if the grid is shaking a lot !!!
*/
float Camera::ComputePerspectiveProjectionCorrectionFactor ( void ) const
{
	float cameraAltitude = m_Position.y;
	if (HelperFunctions::EqualAbs(cameraAltitude, 0.0f))
	{
		cameraAltitude = 0.000001f;
	}
	else if (cameraAltitude < 0.0f)
	{
		cameraAltitude *= -1.0f;
	}

	return cameraAltitude;
}

void Camera::UpdatePerspectiveProjectionMatrix ( int i_WindowWidth, int i_WindowHeight )
{
	assert(i_WindowWidth > 0);
	assert(i_WindowHeight > 0);

	float correctionFactor = ComputePerspectiveProjectionCorrectionFactor();

	m_Projection = glm::perspective(glm::radians(m_FOVy), i_WindowWidth /static_cast<float>(i_WindowHeight), m_ZNear * correctionFactor, m_ZFar * correctionFactor);

	// update the other matrices too
	// more info about projection matrix: http://www.songho.ca/opengl/gl_projectionmatrix.html
	// projection matrix is NOT orthogonal, so we can't replace inverse operation with transpose one !!!
	m_InverseProjection = glm::inverse(m_Projection);
	m_ProjectionView = m_Projection * m_View;

	// projection * view matrix is NOT orthogonal
	m_InverseProjectionView = glm::inverse(m_ProjectionView);
}

void Camera::UpdatePerspectiveProjectionMatrix ( float i_Fovy, float i_Aspect, float i_ZNear, float i_ZFar )
{
	assert(i_Fovy > 0);
	assert(i_Aspect > 0);
	assert(i_ZNear > 0);
	assert(i_ZFar > 0);

	float correctionFactor = ComputePerspectiveProjectionCorrectionFactor();

	m_Projection = glm::perspective(glm::radians(i_Fovy), i_Aspect, i_ZNear * correctionFactor, i_ZFar * correctionFactor);

	// update the other matrices too
	// more info about projection matrix: http://www.songho.ca/opengl/gl_projectionmatrix.html
	// projection matrix is NOT orthogonal, so we can't replace inverse operation with transpose one !!!
	m_InverseProjection = glm::inverse(m_Projection);
	m_ProjectionView = m_Projection * m_View;

	// projection * view matrix is NOT orthogonal
	m_InverseProjectionView = glm::inverse(m_ProjectionView);
}

void Camera::UpdateOrthographicProjectionMatrix ( float i_Left, float i_Right, float i_Bottom, float i_Top, float i_ZNear, float i_ZFar )
{
	m_Projection = glm::ortho(i_Left, i_Right, i_Bottom, i_Top, i_ZNear, i_ZFar);

	// update the other matrices too
	// more info about projection matrix: http://www.songho.ca/opengl/gl_projectionmatrix.html
	// projection matrix is NOT orthogonal, so we can't replace inverse operation with transpose one !!!
	m_InverseProjection = glm::inverse(m_Projection);
	m_ProjectionView = m_Projection * m_View;

	// projection * view matrix is NOT orthogonal
	m_InverseProjectionView = glm::inverse(m_ProjectionView);
}

void Camera::UpdateOrientationWithMouse ( float i_Dx, float i_Dy )
{
	//pitch & yaw angles are in RADIANS

	// we compute pitch and yaw angles directly in radians, so no convertion is necessary!
	m_Pitch += i_Dy;
	m_Yaw += i_Dx;


	if (m_UseConstraints)
	{
		// limit the pitch angle
		if (m_Pitch > glm::half_pi<float>()) m_Pitch = glm::half_pi<float>();
		if (m_Pitch < -glm::half_pi<float>()) m_Pitch = -glm::half_pi<float>();
	}

	// we convert the polar spherical coordinates to cartesian coordinates
	m_Forward.x = glm::cos(m_Pitch) * glm::sin(m_Yaw);
	m_Forward.y = glm::sin(m_Pitch);
	m_Forward.z = glm::cos(m_Pitch) * glm::cos(m_Yaw);
	m_Forward = glm::normalize(m_Forward);

	m_Right.x = glm::sin(m_Yaw - glm::half_pi<float>());
	m_Right.y = 0.0f;
	m_Right.z = glm::cos(m_Yaw - glm::half_pi<float>());
	m_Right = glm::normalize(m_Right);
 
 	m_Up = glm::cross(m_Right, m_Forward);
}

void Camera::UpdatePositionWithKeyboard ( float i_Value, Camera::CAMERA_DIRECTIONS i_Dir )
{
	glm::vec3 deltaMove(0.0f);

	if (i_Dir == CAMERA_DIRECTIONS::CD_FORWARD) deltaMove = m_Forward * i_Value;
	if (i_Dir == CAMERA_DIRECTIONS::CD_BACKWARD) deltaMove = m_Forward * -i_Value;
	if (i_Dir == CAMERA_DIRECTIONS::CD_RIGHT) deltaMove = m_Right * i_Value;
	if (i_Dir == CAMERA_DIRECTIONS::CD_LEFT) deltaMove = m_Right * -i_Value;
	if (i_Dir == CAMERA_DIRECTIONS::CD_UP) deltaMove = m_Up * i_Value;
	if (i_Dir == CAMERA_DIRECTIONS::CD_DOWN) deltaMove = m_Up * -i_Value;

	m_Position += deltaMove;
}

const glm::vec3& Camera::GetPosition ( void ) const
{
	return m_Position;
}

float Camera::GetAltitude ( void ) const
{
	return m_Position.y;
}

const glm::vec3& Camera::GetForward ( void ) const
{
	return m_Forward;
}

const glm::vec3& Camera::GetRight ( void ) const
{
	return m_Right;
}

const glm::vec3& Camera::GetUp ( void ) const
{
	return m_Up;
}

float Camera::GetPitch ( void ) const
{
	return m_Pitch;
}

float Camera::GetYaw ( void ) const
{
	return m_Yaw;
}

float Camera::GetFOV ( void ) const
{
	return m_FOVy;
}

const glm::mat4& Camera::GetViewMatrix ( void ) const
{
	return m_View;
}

const glm::mat4& Camera::GetProjectionMatrix ( void ) const
{
	return m_Projection;
}

const glm::mat4& Camera::GetProjectionViewMatrix ( void ) const
{
	return m_ProjectionView;
}

const glm::mat4& Camera::GetInverseViewMatrix ( void ) const
{
	return m_InverseView;
}

const glm::mat4& Camera::GetInverseProjectionMatrix ( void ) const
{
	return m_InverseProjection;
}

const glm::mat4& Camera::GetInverseProjectionViewMatrix ( void ) const
{
	return m_InverseProjectionView;
}

void Camera::SetPosition ( const glm::vec3& i_Position )
{
	m_Position = i_Position;
}

void Camera::SetAltitude ( float i_Altitude )
{
	m_Position.y = i_Altitude;
}

void Camera::SetForward ( const glm::vec3& i_Forward )
{
	m_Forward = i_Forward;
}

void Camera::SetRight ( const glm::vec3& i_Right )
{
	m_Right = i_Right;
}

void Camera::SetUp ( const glm::vec3& i_Up )
{
	m_Up = i_Up;
}

void Camera::SetFOV ( float i_FOV )
{
	m_FOVy = i_FOV;
}

void Camera::ResetFOV ( void )
{
	m_FOVy = m_InitialFOVy;
}

void Camera::SetProjectionMatrix ( const glm::mat4& i_ProjectionMatrix )
{
	m_Projection = i_ProjectionMatrix;
}