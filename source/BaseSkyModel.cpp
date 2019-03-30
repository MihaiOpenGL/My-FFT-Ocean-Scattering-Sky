/* Author: BAIRAC MIHAI */

#include "BaseSkyModel.h"
#include "CommonHeaders.h"
#include "Camera.h"
// glm::vec3, glm::vec4 come from the header
#include "glm/trigonometric.hpp" //sin(), cos()
#include "glm/gtc/constants.hpp" //half_pi(), two_pi()
#include "GlobalConfig.h"


BaseSkyModel::BaseSkyModel ( void )
	: m_IndexCount(0), m_IsWireframeMode(false)
{
	LOG("BaseSkyModel successfully created!");
}

BaseSkyModel::BaseSkyModel (  const GlobalConfig& i_Config )
	: m_IndexCount(0), m_IsWireframeMode(false)
{
	Initialize(i_Config);
}

BaseSkyModel::~BaseSkyModel ( void )
{
	Destroy();
}

void BaseSkyModel::Destroy ( void )
{
	// should free resources

	LOG("BaseSkyModel successfully destroyed!");
}

void BaseSkyModel::Initialize ( const GlobalConfig& i_Config )
{
	LOG("BaseSkyModel successfully created!");
}


void BaseSkyModel::Update ( const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	//stub
}

void BaseSkyModel::UpdateReflected ( const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	//stub
}

void BaseSkyModel::UpdateRefracted ( const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	//stub
}

void BaseSkyModel::Render ( void )
{
	//stub
}

void BaseSkyModel::RenderReflected ( void )
{
	//stub
}

void BaseSkyModel::RenderRefracted ( void )
{
	//stub
}

void BaseSkyModel::UpdateSunPosition ( float i_CrrTime )
{
	if (m_SunData.IsDynamic)
	{
		float angle = m_SunData.MoveFactor * i_CrrTime * glm::two_pi<float>();
		SetSunDirection(angle, glm::half_pi<float>());
	}
}

void BaseSkyModel::SetSunDirection ( float i_Phi, float i_Theta )
{
	// more info: http://mathworld.wolfram.com/SphericalCoordinates.html

	m_SunData.Direction.x = glm::sin(i_Theta) * glm::cos(i_Phi);
	m_SunData.Direction.y = glm::sin(i_Theta) * glm::sin(i_Phi);
	m_SunData.Direction.z = glm::cos(i_Theta);
}

const glm::vec3& BaseSkyModel::GetSunDirection ( void ) const
{
	return m_SunData.Direction;
}

bool BaseSkyModel::GetAllowChangeDirWithMouse ( void ) const
{
	return m_SunData.AllowChangeDirWithMouse;
}

void BaseSkyModel::SetCloudsOctaves ( unsigned short i_Octaves )
{
	//stub
}

void BaseSkyModel::SetEnabledClouds ( bool i_Value )
{
	//stub
}

bool BaseSkyModel::GetEnabledClouds ( void ) const
{
	//stub
	return false;
}

unsigned short BaseSkyModel::GetCloudsOctaves ( void ) const
{
	//stub
	return 0;
}

void BaseSkyModel::SetCloudsLacunarity ( float i_Lacunarity )
{
	//stub
}

float BaseSkyModel::GetCloudsLacunarity ( void ) const
{
	//stub
	return 0.0f;
}

void BaseSkyModel::SetCloudsGain ( float i_Gain )
{
	//stub
}

float BaseSkyModel::GetCloudsGain ( void ) const
{
	//stub
	return 0.0f;
}

void BaseSkyModel::SetCloudsScaleFactor ( float i_ScaleFactor )
{
	//stub
}

float BaseSkyModel::GetCloudsScaleFactor ( void ) const
{
	//stub
	return 0.0f;
}

void BaseSkyModel::SetCloudsNorm ( float i_Norm )
{
	//stub
}
float BaseSkyModel::GetCloudsNorm ( void ) const
{
	//stub
	return 0.0f;
}