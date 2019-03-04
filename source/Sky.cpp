/* Author: BAIRAC MIHAI */

#include "Sky.h"
#include "GL/glew.h"
#include "Common.h"
#include "ErrorHandler.h"
// glm::vec3, glm::mat4 come from the header
#include "GlobalConfig.h"
#include "CubeMapSkyModel.h"
#include "ScatteringSkyModel.h"
#include "PrecomputedScatteringSkyModel.h"
#include "Camera.h"


Sky::Sky ( void )
	: m_pSkyModel(nullptr), m_ModelType(CustomTypes::Sky::ModelType::MT_COUNT)
{}

Sky::Sky ( const GlobalConfig& i_Config )
	: m_pSkyModel(nullptr), m_ModelType(CustomTypes::Sky::ModelType::MT_COUNT)
{
	Initialize(i_Config);
}

Sky::~Sky ( void )
{
	Destroy();
}

void Sky::Destroy ( void )
{
	SAFE_DELETE(m_pSkyModel);

	LOG("Sky has been destroyed successfully!");
}

void Sky::Initialize ( const GlobalConfig& i_Config )
{
	m_ModelType = i_Config.Scene.Sky.Model.Type;

	if (m_ModelType == CustomTypes::Sky::ModelType::MT_CUBE_MAP)
	{
		m_pSkyModel = new CubeMapSkyModel(i_Config);
	}
	
	else if (m_ModelType == CustomTypes::Sky::ModelType::MT_SCATTERING)
	{
		m_pSkyModel = new ScatteringSkyModel(i_Config);
	}
	
	else if (m_ModelType == CustomTypes::Sky::ModelType::MT_PRECOMPUTED_SCATTERING)
	{
		m_pSkyModel = new PrecomputedScatteringSkyModel(i_Config);
	}
	assert(m_pSkyModel != nullptr);

	LOG("Sky has been created successfully!");
}

void Sky::Update ( const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	if (m_pSkyModel)
	{
		m_pSkyModel->Update(i_Camera, i_IsUnderWater, i_IsWireframeMode, i_CrrTime);
	}
}

void Sky::UpdateReflected ( const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	if (m_pSkyModel)
	{
		m_pSkyModel->UpdateReflected(i_ScaleMatrix, i_Camera, i_IsUnderWater, i_IsWireframeMode, i_CrrTime);
	}
}

void Sky::UpdateRefracted ( const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	if (m_pSkyModel)
	{
		m_pSkyModel->UpdateRefracted(i_ScaleMatrix, i_Camera, i_IsUnderWater, i_IsWireframeMode, i_CrrTime);
	}
}

void Sky::Render ( void )
{
	if (m_pSkyModel)
	{
		m_pSkyModel->Render();
	}
}

void Sky::RenderReflected ( void )
{
	if (m_pSkyModel)
	{
		m_pSkyModel->RenderReflected();
	}
}

void Sky::RenderRefracted ( void )
{
	if (m_pSkyModel)
	{
		m_pSkyModel->RenderRefracted();
	}
}


void Sky::SetSunDirection ( float i_Phi, float i_Theta )
{
	if (m_pSkyModel)
	{
		m_pSkyModel->SetSunDirection(i_Phi, i_Theta);
	}
}

glm::vec3 Sky::GetSunDirection ( void ) const
{
	glm::vec3 sunDir(0.0f);

	if (m_pSkyModel)
	{
		sunDir = m_pSkyModel->GetSunDirection();
	}

	return sunDir;
}

bool Sky::GetAllowChangeDirWithMouse ( void ) const
{
	bool allowChangeDirWithMouse = false;

	if (m_pSkyModel)
	{
		allowChangeDirWithMouse = m_pSkyModel->GetAllowChangeDirWithMouse();
	}

	return allowChangeDirWithMouse;
}

void Sky::SetEnabledClouds ( bool i_Value )
{
	if (m_pSkyModel)
	{
		m_pSkyModel->SetEnabledClouds(i_Value);
	}
}

bool Sky::GetEnabledClouds ( void ) const
{
	bool value = false;

	if (m_pSkyModel)
	{
		value = m_pSkyModel->GetEnabledClouds();
	}

	return value;
}

void Sky::SetCloudsOctaves ( unsigned short i_Octaves )
{
	if (m_pSkyModel)
	{
		m_pSkyModel->SetCloudsOctaves(i_Octaves);
	}
}

unsigned short Sky::GetCloudsOctaves ( void ) const
{
	unsigned short octaves = 0;

	if (m_pSkyModel)
	{
		octaves = m_pSkyModel->GetCloudsOctaves();
	}

	return octaves;
}

void Sky::SetCloudsLacunarity ( float i_Lacunarity )
{
	if (m_pSkyModel)
	{
		m_pSkyModel->SetCloudsLacunarity(i_Lacunarity);
	}
}

float Sky::GetCloudsLacunarity ( void ) const
{
	float lacunarity = 0.0f;

	if (m_pSkyModel)
	{
		lacunarity = m_pSkyModel->GetCloudsLacunarity();
	}

	return lacunarity;
}

void Sky::SetCloudsGain ( float i_Gain )
{
	if (m_pSkyModel)
	{
		m_pSkyModel->SetCloudsGain(i_Gain);
	}
}

float Sky::GetCloudsGain ( void ) const
{
	float gain = 0.0f;

	if (m_pSkyModel)
	{
		gain = m_pSkyModel->GetCloudsGain();
	}

	return gain;
}

void Sky::SetCloudsScaleFactor ( float i_ScaleFactor )
{
	if (m_pSkyModel)
	{
		m_pSkyModel->SetCloudsScaleFactor(i_ScaleFactor);
	}
}

float Sky::GetCloudsScaleFactor ( void ) const
{
	float scaleFactor = 0.0f;

	if (m_pSkyModel)
	{
		scaleFactor = m_pSkyModel->GetCloudsScaleFactor();
	}

	return scaleFactor;
}

void Sky::SetCloudsNorm ( float i_Norm )
{
	if (m_pSkyModel)
	{
		m_pSkyModel->SetCloudsNorm(i_Norm);
	}
}

float Sky::GetCloudsNorm ( void ) const
{
	float norm = 0.0f;

	if (m_pSkyModel)
	{
		norm = m_pSkyModel->GetCloudsNorm();
	}

	return norm;
}

CustomTypes::Sky::ModelType Sky::GetModelType ( void ) const
{
	return m_ModelType;
}