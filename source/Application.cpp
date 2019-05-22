////////////////////////////////
////////////////////////////////
/*
TEXTURE UNIT ALLOCATION

5 : [0 - 4] = global usage
5 : [5 - 9] = sky usage
20 : [10 - 29] = ocean usage
5 : [30 - 34] = boat usage
*/
////////////////////////////////
////////////////////////////////

#include "Application.h"
#include "CommonHeaders.h"
#include "CustomTypes.h"
#include "HelperFunctions.h"
#include "GLConfig.h"
#include "GlobalConfig.h"
#define GLM_MESSAGES //info about GLM
// glm::vec3 comes from the header
#include "glm/common.hpp" //clamp()
#include "glm/gtc/type_ptr.hpp" //value_ptr()
#include "glm/gtc/matrix_transform.hpp" //scale()
#include "glm/gtc/constants.hpp" //epsilon
#include "PhysicsConstants.h"
#include "FrameBufferManager.h"
#include "PostProcessingManager.h"
#include "Camera.h"
#include "Sky.h"
#include "Ocean.h"
#include "MotorBoat.h"


Application::Application()
{
	LOG("Application successfully created!");
}

Application::Application(const GlobalConfig& i_Config, int i_WindowWidth, int i_WindowHeight)
	: m_WindowWidth(0), m_WindowHeight(0), m_pFBM(nullptr),
#ifdef USE_GUI
	  m_pGUIBar(nullptr),
#endif //USE_GUI
      m_pCamera(nullptr), m_pObservingCamera(nullptr), m_pCurrentViewingCamera(nullptr), m_pCurrentControllingCamera(nullptr),
	  m_pPostProcessingManager(nullptr), m_pSky(nullptr), m_pOcean(nullptr), m_pMotorBoat(nullptr),
	  m_TimeScale(0.0f), m_CrrTime(0.0f), m_DeltaTime(0.0f),
	  m_IsGUIVisible(false), m_IsCameraViewChanged(false), m_IsCameraControlChanged(false),
	  m_IsRenderWireframe(false), m_IsRenderPoints(false), m_IsCursorReleased(false),
      m_IsFrustumVisible(false), m_IsInBoatMode(false),
	  m_MouseOldXPos(0), m_MouseOldYPos(0), m_IsInDraggingMode(false),
	  m_KeySpeed(0.0f), m_MouseSpeed(0.0f), m_SunPhi(0.0f), m_SunTheta(0.0f)
{
	SetWindowSize(i_WindowWidth, i_WindowHeight);

	SetupGL(i_Config);

	SetupGlobals(i_Config);

	InitScene(i_Config);

	LOG("Application successfully created!");
}


Application::~Application()
{
	TerminateScene();

	LOG("Application successfully destroyed!");
}


void Application::SetupGlobals(const GlobalConfig& i_Config)
{
	m_TimeScale = i_Config.Simulation.TimeScale;
	m_IsGUIVisible = i_Config.Simulation.ShowGUI;

	m_KeySpeed = i_Config.Input.KeySpeed;
	m_MouseSpeed = i_Config.Input.MouseSpeed;
}

#ifdef USE_GUI
//// GUI Callbacks
void TW_CALL Application::GetCameraPosition(void *i_pValue, void *i_pClientData)
{
	*static_cast<glm::vec3 *>(i_pValue) = static_cast<const Camera *>(i_pClientData)->GetPosition();
}

void TW_CALL Application::GetCameraDirection(void *i_pValue, void *i_pClientData)
{
	*static_cast<glm::vec3 *>(i_pValue) = static_cast<const Camera *>(i_pClientData)->GetForward();
}


void TW_CALL Application::GetEnabledClouds(void *i_pValue, void *i_pClientData)
{
	*static_cast<bool *>(i_pValue) = static_cast<const Sky *>(i_pClientData)->GetEnabledClouds();
}

void TW_CALL Application::SetEnabledClouds(const void* i_pValue, void* i_pClientData)
{
	static_cast<Sky *>(i_pClientData)->SetEnabledClouds(*static_cast<const bool *>(i_pValue));
}

void TW_CALL Application::GetCloudsOctaves(void *i_pValue, void *i_pClientData)
{
	*static_cast<unsigned short *>(i_pValue) = static_cast<const Sky *>(i_pClientData)->GetCloudsOctaves();
}

void TW_CALL Application::SetCloudsOctaves(const void* i_pValue, void* i_pClientData)
{
	static_cast<Sky *>(i_pClientData)->SetCloudsOctaves(*static_cast<const unsigned short *>(i_pValue));
}

void TW_CALL Application::GetCloudsLacunarity(void *i_pValue, void *i_pClientData)
{
	*static_cast<float *>(i_pValue) = static_cast<const Sky *>(i_pClientData)->GetCloudsLacunarity();
}

void TW_CALL Application::SetCloudsLacunarity(const void* i_pValue, void* i_pClientData)
{
	static_cast<Sky *>(i_pClientData)->SetCloudsLacunarity(*static_cast<const float *>(i_pValue));
}

void TW_CALL Application::GetCloudsGain(void *i_pValue, void *i_pClientData)
{
	*static_cast<float *>(i_pValue) = static_cast<const Sky *>(i_pClientData)->GetCloudsGain();
}

void TW_CALL Application::SetCloudsGain(const void* i_pValue, void* i_pClientData)
{
	static_cast<Sky *>(i_pClientData)->SetCloudsGain(*static_cast<const float *>(i_pValue));
}

void TW_CALL Application::GetCloudsScaleFactor(void *i_pValue, void *i_pClientData)
{
	*static_cast<float *>(i_pValue) = static_cast<const Sky *>(i_pClientData)->GetCloudsScaleFactor();
}

void TW_CALL Application::SetCloudsScaleFactor(const void* i_pValue, void* i_pClientData)
{
	static_cast<Sky *>(i_pClientData)->SetCloudsScaleFactor(*static_cast<const float *>(i_pValue));
}

void TW_CALL Application::GetCloudsNorm(void *i_pValue, void *i_pClientData)
{
	*static_cast<float *>(i_pValue) = static_cast<const Sky *>(i_pClientData)->GetCloudsNorm();
}

void TW_CALL Application::SetCloudsNorm(const void* i_pValue, void* i_pClientData)
{
	static_cast<Sky *>(i_pClientData)->SetCloudsNorm(*static_cast<const float *>(i_pValue));
}

void TW_CALL Application::SetOceanAmplitude(const void* i_pValue, void* i_pClientData)
{
	static_cast<Ocean *>(i_pClientData)->SetWaveAmplitude(*static_cast<const float *>(i_pValue));
}

void TW_CALL Application::GetOceanAmplitude(void* i_pValue, void* i_pClientData)
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetWaveAmplitude();
}

void TW_CALL Application::SetOceanPatchSize(const void* i_pValue, void* i_pClientData)
{
	static_cast<Ocean *>(i_pClientData)->SetPatchSize(*static_cast<const unsigned short *>(i_pValue));
}

void TW_CALL Application::GetOceanPatchSize(void* i_pValue, void* i_pClientData)
{
	*static_cast<unsigned short *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetPatchSize();
}

void TW_CALL Application::SetOceanTileScale(const void* i_pValue, void* i_pClientData)
{
	static_cast<Ocean *>(i_pClientData)->SetTileScale(*static_cast<const float *>(i_pValue));
}

void TW_CALL Application::GetOceanTileScale(void* i_pValue, void* i_pClientData)
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetTileScale();
}

void TW_CALL Application::SetOceanWindSpeed(const void* i_pValue, void* i_pClientData)
{
	static_cast<Ocean *>(i_pClientData)->SetWindSpeed(*static_cast<const float *>(i_pValue));
}

void TW_CALL Application::GetOceanWindSpeed(void* i_pValue, void* i_pClientData)
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetWindSpeed();
}

void TW_CALL Application::SetOceanWindDirectionX(const void* i_pValue, void* i_pClientData)
{
	static_cast<Ocean *>(i_pClientData)->SetWindDirectionX(*static_cast<const float *>(i_pValue));
}

void TW_CALL Application::GetOceanWindDirectionX(void* i_pValue, void* i_pClientData)
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetWindDirectionX();
}

void TW_CALL Application::SetOceanWindDirectionZ(const void* i_pValue, void* i_pClientData)
{
	static_cast<Ocean *>(i_pClientData)->SetWindDirectionZ(*static_cast<const float *>(i_pValue));
}

void TW_CALL Application::GetOceanWindDirectionZ(void* i_pValue, void* i_pClientData)
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetWindDirectionZ();
}

void TW_CALL Application::GetOceanWindDir(void* i_pValue, void* i_pClientData)
{
	*static_cast<glm::vec3 *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetWindDir();
}

void TW_CALL Application::SetOceanOpposingWavesFactor(const void* i_pValue, void* i_pClientData)
{
	static_cast<Ocean *>(i_pClientData)->SetOpposingWavesFactor(*static_cast<const float *>(i_pValue));
}

void TW_CALL Application::GetOceanOpposingWavesFactor(void* i_pValue, void* i_pClientData)
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetOpposingWavesFactor();
}

void TW_CALL Application::SetOceanVerySmallWavesFactor(const void* i_pValue, void* i_pClientData)
{
	static_cast<Ocean *>(i_pClientData)->SetVerySmallWavesFactor(*static_cast<const float *>(i_pValue));
}

void TW_CALL Application::GetOceanVerySmallWavesFactor(void* i_pValue, void* i_pClientData)
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetVerySmallWavesFactor();
}

void TW_CALL Application::SetOceanChoppyScale(const void* i_pValue, void* i_pClientData)
{
	static_cast<Ocean *>(i_pClientData)->SetChoppyScale(*static_cast<const float *>(i_pValue));
}

void TW_CALL Application::GetOceanChoppyScale(void* i_pValue, void* i_pClientData)
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetChoppyScale();
}


void TW_CALL Application::SetFOV(const void* i_pValue, void* i_pClientData)
{
	static_cast<Camera *>(i_pClientData)->SetFOV(*static_cast<const float *>(i_pValue));
}

void TW_CALL Application::GetFOV(void* i_pValue, void* i_pClientData)
{
	*static_cast<float *>(i_pValue) = static_cast<const Camera *>(i_pClientData)->GetFOV();
}
#endif //USE_GUI

// Small function that converts LDR to HDR
glm::vec3 Application::HDR(glm::vec3 i_L, float i_E)
{
	i_L = i_L * i_E;
	i_L.r = i_L.r < 1.413f ? pow(i_L.r * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(- i_L.r);
	i_L.g = i_L.g < 1.413f ? pow(i_L.g * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(- i_L.g);
	i_L.b = i_L.b < 1.413f ? pow(i_L.b * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(- i_L.b);
	return i_L;
}

void Application::InitScene(const GlobalConfig& i_Config)
{
#ifdef USE_GUI
	////////////// Init AntTweakBar - GUI //////////
	int ret = 0;

	if (i_Config.OpenGLContext.IsCoreProfile)
	{
		ret = TwInit(TW_OPENGL_CORE, nullptr);
	}
	else
	{
		ret = TwInit(TW_OPENGL, nullptr);
	}

	if (!ret)
	{
		ERR("Failed to initialize AntTweakBar!");
	}

	/// set the windows size
	ret = TwWindowSize(m_WindowWidth, m_WindowHeight);
	assert(ret != 0);

	//// init the bar
	m_pGUIBar = TwNewBar("Parameters");
	assert(m_pGUIBar != nullptr);
	ret = TwDefine("Parameters size='350 400'");
	assert(ret != 0);
#endif //USE_GUI
	//////////////////////////////////////////

	// compute default framebuffer underwater color !!!	
	m_UnderWaterColor = HDR(i_Config.Scene.Ocean.UnderWater.Fog.Color, i_Config.Rendering.HDR.Exposure);

	// default framebuffer color
	m_DefaultColor.r = 0.0f;
	m_DefaultColor.g = 0.0f;
	m_DefaultColor.b = 0.0f;

	////////////////

	m_pCamera = new Camera("Rendering", i_Config);
	assert(m_pCamera != nullptr);

	if (m_pCamera)
	{
		m_pCamera->UpdatePerspectiveProjectionMatrix(m_WindowWidth, m_WindowHeight);
	}

	m_pObservingCamera = new Camera("Observing", i_Config);
	assert(m_pObservingCamera != nullptr);

	if (m_pObservingCamera)
	{
		m_pObservingCamera->UpdatePerspectiveProjectionMatrix(m_WindowWidth, m_WindowHeight);
	}

	m_pCurrentViewingCamera = m_pCamera;
	m_pCurrentControllingCamera = m_pCamera;

	m_IsCursorReleased = false;
	m_IsRenderWireframe = false;
	m_IsRenderPoints = false;
	m_IsCameraViewChanged = false;
	m_IsCameraControlChanged = false;

#ifdef USE_GUI
	// add params to GUI
	ret = TwAddVarRW(m_pGUIBar, "TimeScale", TW_TYPE_FLOAT, &m_TimeScale, "min=-20.0f; max=20.0f; step=1.0f group=Simulation");
	assert(ret != 0);

	ret = TwAddVarCB(m_pGUIBar, "CameraPosition", TW_TYPE_DIR3F, nullptr, GetCameraPosition, m_pCurrentControllingCamera, "group=Camera");
	assert(ret != 0);
	ret = TwAddVarCB(m_pGUIBar, "CameraDirection", TW_TYPE_DIR3F, nullptr, GetCameraDirection, m_pCurrentControllingCamera, "group=Camera");
	assert(ret != 0);
#endif //USE_GUI
	//////////////////////////////////////////////////////

	m_pPostProcessingManager = new PostProcessingManager(i_Config);
	assert(m_pPostProcessingManager != nullptr);
	LOG("PostProcessingManager is: %d bytes in size", sizeof(*m_pPostProcessingManager));

	//////// SKY ////////
	m_pSky = new Sky(i_Config);
	assert(m_pSky != nullptr);
	LOG("Sky is: %d bytes in size", sizeof(*m_pSky));

#ifdef USE_GUI
	if (m_pSky && m_pSky->GetModelType() != CustomTypes::Sky::ModelType::MT_CUBE_MAP)
	{
		ret = TwAddVarCB(m_pGUIBar, "Enabled", TW_TYPE_BOOL16, SetEnabledClouds, GetEnabledClouds, m_pSky, "group=Clouds");
		assert(ret != 0);
		ret = TwAddVarCB(m_pGUIBar, "Octaves", TW_TYPE_UINT16, SetCloudsOctaves, GetCloudsOctaves, m_pSky, "min=1; max=15.0; step=1 group=Clouds");
		assert(ret != 0);
		ret = TwAddVarCB(m_pGUIBar, "Lacunarity", TW_TYPE_FLOAT, SetCloudsLacunarity, GetCloudsLacunarity, m_pSky, "min=0.1; max=3.0; step=0.1 group=Clouds");
		assert(ret != 0);
		ret = TwAddVarCB(m_pGUIBar, "Gain", TW_TYPE_FLOAT, SetCloudsGain, GetCloudsGain, m_pSky, "min=0.1; max=1.0; step=0.1 group=Clouds");
		assert(ret != 0);

		if (m_pSky->GetModelType() == CustomTypes::Sky::ModelType::MT_SCATTERING)
		{
			ret = TwAddVarCB(m_pGUIBar, "ScaleFactor", TW_TYPE_FLOAT, SetCloudsScaleFactor, GetCloudsScaleFactor, m_pSky, "min=0.1; max=2.0; step=0.1 group=Clouds");
			assert(ret != 0);
		}
		else if (m_pSky->GetModelType() == CustomTypes::Sky::ModelType::MT_PRECOMPUTED_SCATTERING)
		{
			ret = TwAddVarCB(m_pGUIBar, "Norm", TW_TYPE_FLOAT, SetCloudsNorm, GetCloudsNorm, m_pSky, "min=0.1; max=1.0; step=0.1 group=Clouds");
			assert(ret != 0);
		}
	}
#endif //USE_GUI

	///////// OCEAN ///////////
	m_pOcean = new Ocean(i_Config);
	assert(m_pOcean != nullptr);
	LOG("Ocean is: %d bytes in size", sizeof(*m_pSky));

#ifdef USE_GUI
	// add params to GUI
	ret = TwAddVarCB(m_pGUIBar, "Amplitude", TW_TYPE_FLOAT, SetOceanAmplitude, GetOceanAmplitude, m_pOcean, "min=0.01; max=1.0; step=0.01 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(m_pGUIBar, "PatchSize", TW_TYPE_UINT16, SetOceanPatchSize, GetOceanPatchSize, m_pOcean, "min=128.0; max=1024.0; step=64.0 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(m_pGUIBar, "TileScale", TW_TYPE_FLOAT, SetOceanTileScale, GetOceanTileScale, m_pOcean, "min=0.01; max=5.0; step=0.01 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(m_pGUIBar, "WindSpeed", TW_TYPE_FLOAT, SetOceanWindSpeed, GetOceanWindSpeed, m_pOcean, "min=0.5; max=200; step=1.0 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(m_pGUIBar, "WindDirectionX", TW_TYPE_FLOAT, SetOceanWindDirectionX, GetOceanWindDirectionX, m_pOcean, "min=-1.0; max=1.0; step=0.2 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(m_pGUIBar, "WindDirectionZ", TW_TYPE_FLOAT, SetOceanWindDirectionZ, GetOceanWindDirectionZ, m_pOcean, "min=-1.0; max=1.0; step=0.2 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(m_pGUIBar, "WindDir", TW_TYPE_DIR3F, nullptr, GetOceanWindDir, m_pOcean, "group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(m_pGUIBar, "OpposingWavesFactor", TW_TYPE_FLOAT, SetOceanOpposingWavesFactor, GetOceanOpposingWavesFactor, m_pOcean, "min=0.0001; max=3.0; step=0.01 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(m_pGUIBar, "VerySmallWavesFactor", TW_TYPE_FLOAT, SetOceanVerySmallWavesFactor, GetOceanVerySmallWavesFactor, m_pOcean, "min=0.0001; max=0.1; step=0.0001 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(m_pGUIBar, "ChoppyScale", TW_TYPE_FLOAT, SetOceanChoppyScale, GetOceanChoppyScale, m_pOcean, "min=0.1; max=3.0; step=0.1 group=Waves");
	assert(ret != 0);

	// add params to GUI
	ret = TwAddVarCB(m_pGUIBar, "FOV", TW_TYPE_FLOAT, SetFOV, GetFOV, m_pCurrentControllingCamera, "min=5.0; max=129.0; step=1.0 group=Rendering");
	assert(ret != 0);
#endif //USE_GUI

	//////// MOTOR BOAT ////////
	m_pMotorBoat = new MotorBoat(i_Config);
	assert(m_pMotorBoat != nullptr);
	LOG("Motor Boat is: %d bytes in size", sizeof(*m_pSky));
}

void Application::Update(float i_CrrTime, float i_DeltaTime, const GlobalConfig& i_Config)
{
	m_CrrTime = i_CrrTime;
	m_DeltaTime = i_DeltaTime;

	// NOTE! Reflected stuff is only above water
	if (i_Config.VisualEffects.ShowReflections && m_pCurrentViewingCamera && m_pCurrentViewingCamera->GetAltitude() > 0.0f)
	{
		UpdateReflectedScene(i_CrrTime, i_DeltaTime);

		RenderReflectedScene();
	}

	// NOTE! Refracted stuff is only under water
	if (i_Config.VisualEffects.ShowRefractions && m_pCurrentViewingCamera && m_pCurrentViewingCamera->GetAltitude() < 0.0f)
	{
		UpdateRefractedScene(i_CrrTime, i_DeltaTime);

		RenderRefractedScene();
	}

	UpdateScene(i_CrrTime, i_DeltaTime, i_Config);

	// update
	UpdatePPE(i_CrrTime, i_DeltaTime, i_Config);
}

void Application::UpdatePPE(float i_CrrTime, float i_DeltaTime, const GlobalConfig& i_Config)
{
	if (i_Config.VisualEffects.PostProcessing.Enabled && m_pPostProcessingManager)
	{
		m_pPostProcessingManager->Update(i_CrrTime, i_DeltaTime);
	}
}

void Application::UpdateReflectedScene(float i_CrrTime, float i_DeltaTime)
{
	// Local Reflection
	// Transforms order: S * R * T (from left to right)
	glm::mat4 ScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));

	//// update the objects in scene

	if (m_pMotorBoat && m_pSky && m_pCurrentViewingCamera)
	{
		m_pMotorBoat->UpdateReflected(ScaleMatrix, *m_pCurrentViewingCamera, m_pSky->GetSunDirection(), m_IsRenderWireframe, i_CrrTime);
	}

	if (m_pSky && m_pCurrentViewingCamera)
	{
		m_pSky->UpdateReflected(ScaleMatrix, *m_pCurrentViewingCamera, m_pCurrentViewingCamera->GetAltitude() < 0.0f, m_IsRenderWireframe, i_CrrTime);
	}
}

void Application::UpdateRefractedScene(float i_CrrTime, float i_DeltaTime)
{
	// Local Refraction
	glm::mat4 ScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.75f, 1.0f));

	//// update the objects in scene

	if (m_pMotorBoat && m_pSky && m_pCurrentViewingCamera)
	{
		m_pMotorBoat->UpdateRefracted(ScaleMatrix, *m_pCurrentViewingCamera, m_pSky->GetSunDirection(), m_IsRenderWireframe, i_CrrTime);
	}

	if (m_pSky && m_pCurrentViewingCamera)
	{
		m_pSky->UpdateRefracted(ScaleMatrix, *m_pCurrentViewingCamera, m_pCamera->GetAltitude() < 0.0f, m_IsRenderWireframe, i_CrrTime);
	}
}

/*
More info about boat buoyancy:

 http://www.gamasutra.com/view/news/237528/Water_interaction_model_for_boats_in_video_games.php
 http://www.gamasutra.com/view/news/263237/Water_interaction_model_for_boats_in_video_games_Part_2.php
 http://www.habrador.com/tutorials/unity-boat-tutorial/
*/

void Application::ComputeBuoyancy(float i_DeltaTime, bool i_IsBuyoancyEnabled)
{
	// Compute Boat Buoyancy
	if (i_IsBuyoancyEnabled && m_pOcean && m_pMotorBoat && m_pMotorBoat->GetBoatVelocity() < glm::epsilon<float>())
	{
		static float boatYVelocity = 0.0f;

		static float boatYPos = m_pMotorBoat->GetBoatYPos();

	//	float waterHeight = m_pOcean->ComputeWaterHeightAt(m_pMotorBoat->GetBoatPosition().xz()); //TODO - swizzle fails
		float waterHeight = m_pOcean->ComputeWaterHeightAt(glm::vec2(m_pMotorBoat->GetBoatPosition().x, m_pMotorBoat->GetBoatPosition().z));
		waterHeight *= 0.5f; // reduce the height for better approximation of the effect

		float waterDisplacedVolume = m_pMotorBoat->GetBoatArea() * glm::clamp(waterHeight - (boatYPos - m_pMotorBoat->GetBoatHeight() * 0.5f), 0.0f, m_pMotorBoat->GetBoatHeight());

		// compute forces of buoyancy

		// gravitational force
		float gravitationalForce = m_pMotorBoat->GetBoatMass() * -PhysicsConstants::kG;

		// buoyancy force
		float buoyancyForce = waterDisplacedVolume * PhysicsConstants::kWaterDensity * PhysicsConstants::kG;

		// drag force - computed only when the boat is in water
		float dragForce = 0.0f;
		if (boatYPos < waterHeight)
		{
			dragForce = -0.5f * (boatYVelocity * boatYVelocity) * PhysicsConstants::kWaterDensity * m_pMotorBoat->GetBoatArea() * m_pMotorBoat->GetBoatDragCoefficient();

			if (boatYVelocity < 0.0f)
			{
				dragForce *= -1.0f;
			}
		}

		float netForce = gravitationalForce + buoyancyForce + dragForce;

		float boatYAccel = netForce / m_pMotorBoat->GetBoatMass();

		boatYPos += boatYAccel * i_DeltaTime * m_pMotorBoat->GetBoatYAccelerationFactor();

		m_pMotorBoat->SetBoatYPos(boatYPos);
	}
}

void Application::UpdateScene(float i_CrrTime, float i_DeltaTime, const GlobalConfig& i_Config)
{
	if (m_pSky && m_pCurrentViewingCamera && m_pCurrentViewingCamera->GetAltitude() > -10.0f)
	{
		m_pSky->Update(*m_pCurrentViewingCamera, m_pCurrentViewingCamera->GetAltitude() < 0.0f, m_IsRenderWireframe, i_CrrTime);
	}

	if (m_pOcean && m_pSky && m_pCamera)
	{
		m_pOcean->Update(*m_pCamera, m_pSky->GetSunDirection(), m_IsRenderWireframe, m_IsFrustumVisible, i_CrrTime);
	}

	if (m_pOcean && m_pMotorBoat)
	{
		m_pOcean->UpdateBoatEffects(*m_pMotorBoat);
	}

	if (m_pMotorBoat && m_pSky && m_pCurrentViewingCamera)
	{
		m_pMotorBoat->Update(*m_pCurrentViewingCamera, m_pSky->GetSunDirection(), m_IsRenderWireframe, i_CrrTime);
	}

	ComputeBuoyancy(i_DeltaTime, i_Config.Scene.Ocean.Surface.BoatEffects.Buoyancy.Enabled);
}

void Application::Render(const GlobalConfig& i_Config)
{
	RenderScene(i_Config);

	RenderPPE(i_Config);

#ifdef USE_GUI
	RenderGUI();
#endif //USE_GUI
}

void Application::RenderPPE(const GlobalConfig& i_Config)
{
	if (i_Config.VisualEffects.PostProcessing.Enabled && m_pPostProcessingManager)
	{
		m_pPostProcessingManager->Render();
	}
}

void Application::RenderReflectedScene(void)
{
	// Local Reflection
	if (m_pFBM)
	{
		m_pFBM->Bind();
		m_pFBM->SetupDrawBuffers(1, 0);
	}

	// clear the buffers for the current frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CLIP_DISTANCE0);

	// render the reflected objects in scene

	// NOTE! In our case we render above water boat reflection
	if (m_pMotorBoat)
	{
		m_pMotorBoat->RenderReflected();
	}

	// the sky has to be drawn last !!!
	// as explined in here: https://learnopengl.com/#!Advanced-OpenGL/Cubemaps
	if (m_pSky)
	{
		m_pSky->RenderReflected();
	}

	glDisable(GL_CLIP_DISTANCE0);

	if (m_pFBM)
	{
		m_pFBM->UnBind();
	}
}

void Application::RenderRefractedScene(void)
{
	// Local Refraction
	if (m_pFBM)
	{
		m_pFBM->Bind();
		m_pFBM->SetupDrawBuffers(1, 1);
	}

	// clear the buffers for the current frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CLIP_DISTANCE1);

	// render the refracted objects in scene

	// NOTE! In our case we render under water boat refraction
	if (m_pMotorBoat)
	{
		m_pMotorBoat->RenderRefracted();
	}

	// the sky has to be drawn last !!!
	// as explined in here: https://learnopengl.com/#!Advanced-OpenGL/Cubemaps
	if (m_pSky)
	{
		m_pSky->RenderRefracted();
	}

	glDisable(GL_CLIP_DISTANCE1);

	if (m_pFBM)
	{
		m_pFBM->UnBind();
	}
}

void Application::RenderScene(const GlobalConfig& i_Config)
{
	if (i_Config.VisualEffects.PostProcessing.Enabled && m_pPostProcessingManager)
	{
		m_pPostProcessingManager->BindFB();
	}

	//// clear the buffers for the current frame

	glm::vec3 color = (m_pCamera->GetAltitude() > 0.0f ? m_DefaultColor : m_UnderWaterColor);

	glClearColor(color.r, color.g, color.b, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//// RENDERING SCENE ////

	if (i_Config.VisualEffects.PostProcessing.Enabled == false)
	{
		// NOTE! Because of the underwater godrays screen space algorithm
		// all opaque objects in scene must be rendered before the ocean is !!!
		if (m_pMotorBoat)
		{
			m_pMotorBoat->Render();
		}
	}

	if (i_Config.Scene.Ocean.Surface.BoatEffects.HideInsideWater && 
		m_pMotorBoat && m_pCurrentViewingCamera && m_pCurrentViewingCamera->GetAltitude() > 0.0f)
	{
        //TODO Fix this when using PPE
        
		///////////// MAKE THE WATER IN THE BOAT DISAPPEAR !!! ///////////
		// actually what this does - creates a hole in the ocean in the place of the flattened boat
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 0xFF); //always pass the stencil test, ref value = 1
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // if both the stencil test and the depth test succeed however, we want to replace the stored stencil value with the reference value
		glStencilMask(0xFF); // enable writing to the stencil buffer

        glClear(GL_STENCIL_BUFFER_BIT);
        
		// for all default buffers
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); //disable writing to color buffer

		// by drawing something we write 1 in the stencil buffer for each drawn fragment
		m_pMotorBoat->RenderFlattened();

		// for all default buffers
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); //enable writing to color buffer

		glStencilMask(0x00); // disable writing to the stencil buffer
		glStencilFunc(GL_EQUAL, 0, 0xFF); // pass the stencil test only if the stored stencil value is 0, same as not equal to 1
	}

	if (m_pFBM)
	{
		m_pFBM->BindColorAttachmentByIndex(0, true); // reflection map
		m_pFBM->BindColorAttachmentByIndex(1, true); // refraction map
	}

	if (m_pOcean && m_pCurrentViewingCamera)
	{
		m_pOcean->Render(*m_pCurrentViewingCamera);
	}

	if (i_Config.Scene.Ocean.Surface.BoatEffects.HideInsideWater &&
		m_pCurrentViewingCamera && m_pCurrentViewingCamera->GetAltitude() > 0.0f)
	{
		glDisable(GL_STENCIL_TEST);
	}
	/////////////////////////////////////

	// the sky has to be drawn last !!!
	// as explined in here: https://learnopengl.com/#!Advanced-OpenGL/Cubemaps

	if (m_pSky && m_pCurrentViewingCamera && m_pCurrentViewingCamera->GetAltitude() > 0.0f)
	{
		m_pSky->Render();
	}

	if (i_Config.VisualEffects.PostProcessing.Enabled)
	{
		if (m_pMotorBoat)
		{
			m_pMotorBoat->Render();
		}
	}

	if (i_Config.VisualEffects.PostProcessing.Enabled && m_pPostProcessingManager)
	{
		m_pPostProcessingManager->UnBindFB();
	}
}

void Application::RenderGUI()
{
#ifdef USE_GUI
	if (m_IsGUIVisible)
	{
		//Draw AntTweakBar bars
		int ret = TwDraw(); // returneaza 'Invalid Value' ca eroare OpenGL, dar e ok!
		//assert(ret != 0);
	}
#endif //USE_GUI
}

void Application::TerminateScene(void)
{
	m_pCurrentViewingCamera = nullptr;
	m_pCurrentControllingCamera = nullptr;
	SAFE_DELETE(m_pCamera);
	SAFE_DELETE(m_pObservingCamera);

	SAFE_DELETE(m_pPostProcessingManager);

	SAFE_DELETE(m_pFBM);

	SAFE_DELETE(m_pSky);

	SAFE_DELETE(m_pOcean);

	SAFE_DELETE(m_pMotorBoat);

#ifdef USE_GUI
	// Terminate AntTweakBar
	int ret = TwTerminate();
	assert(ret != 0);
#endif //USE_GUI
}


void Application::SetupDefaultFrameBuffer(const GlobalConfig& i_Config)
{

}

void Application::SetupAuxiliaryFrameBuffer(const GlobalConfig& i_Config)
{
	m_pFBM = new FrameBufferManager("Main Auxiliary FrameBuffer", i_Config);
	assert(m_pFBM != nullptr);
	m_pFBM->CreateSimple(2, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, m_WindowWidth, m_WindowHeight, GL_CLAMP_TO_EDGE, GL_LINEAR, i_Config.TexUnit.Global.ReflectionMap, 5, false, FrameBufferManager::DEPTH_BUFFER_TYPE::DBT_RENDER_BUFFER_DEPTH_STENCIL);
}

void Application::SetupGL(const GlobalConfig& i_Config)
{
	// Cull Face
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	// Facing
	glFrontFace(GL_CCW);

	// Depth
	glClearDepth(1.0f);
	glDepthRange(0.0f, 1.0f);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	// Stencil
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
	glStencilMask(0x0);
	glDisable(GL_STENCIL_TEST);

	// Point Size
	glPointSize(1.0f);
	glEnable(GL_PROGRAM_POINT_SIZE);

	// Anti-Aliasing Sample Mode
	glEnable(GL_MULTISAMPLE); //anti-aliasing

	////////////////////////////////////////////////
	SetupDefaultFrameBuffer(i_Config);

	SetupAuxiliaryFrameBuffer(i_Config);

	// Viewport
	glViewport(0, 0, m_WindowWidth, m_WindowHeight);
}

void Application::SwitchViewBetweenCameras()
{
	if (m_IsCameraViewChanged)
	{
		m_pCurrentViewingCamera = m_pCamera;
		m_IsCameraViewChanged = false;
	}
	else
	{
		m_pCurrentViewingCamera = m_pObservingCamera;
		m_IsCameraViewChanged = true;
	}
}

void Application::SwitchControlBetweenCameras()
{
	if (m_IsCameraControlChanged)
	{
		m_pCurrentControllingCamera = m_pCamera;
		m_IsCameraControlChanged = false;
	}
	else
	{
		m_pCurrentControllingCamera = m_pObservingCamera;
		m_IsCameraControlChanged = true;
	}
}

void Application::ResetFOV()
{
	if (m_pCurrentControllingCamera)
	{
		m_pCurrentControllingCamera->ResetFOV();
	}
}

void Application::OnWindowResize(int i_Width, int i_Height)
{
	SetWindowSize(i_Width, i_Height);

	glViewport(0, 0, m_WindowWidth, m_WindowHeight);


	if (m_pCurrentControllingCamera)
	{
		m_pCurrentControllingCamera->UpdatePerspectiveProjectionMatrix(m_WindowWidth, m_WindowHeight);
	}

	if (m_pFBM)
	{
		m_pFBM->UpdateColorAttachmentSize(0, m_WindowWidth, m_WindowHeight); //reflection
		m_pFBM->UpdateColorAttachmentSize(1, m_WindowWidth, m_WindowHeight); //refraction
		m_pFBM->UpdateDepthBufferSize(m_WindowWidth, m_WindowHeight);
	}

	if (m_pOcean)
	{
		m_pOcean->UpdateGrid(m_WindowWidth, m_WindowHeight);
	}

	if (m_pPostProcessingManager)
	{
		m_pPostProcessingManager->UpdateSize(m_WindowWidth, m_WindowHeight);
	}

#ifdef USE_GUI
	if (m_IsGUIVisible)
	{
		int ret = TwWindowSize(m_WindowWidth, m_WindowHeight);
		assert(ret != 0);
	}
#endif //USE_GUI
}

bool Application::OnGUIMouseEventSDL(const SDL_Event& sdlEvent, unsigned char sdlMajorVersion, unsigned char sdlMinorVersion)
{
#ifdef USE_GUI
	if (m_IsGUIVisible)
	{
		return TwEventSDL(&sdlEvent, sdlMajorVersion, sdlMinorVersion);
	}
#endif //USE_GUI

	return false;
}

void Application::OnMouseMotion(int i_XPos, int i_YPos)
{
	if (m_IsInDraggingMode)
	{
		// more info: http://mathworld.wolfram.com/SphericalCoordinates.html

		if (m_pSky && m_pSky->GetAllowChangeDirWithMouse())
		{

			switch(m_pSky->GetModelType())
			{ 
				case CustomTypes::Sky::ModelType::MT_PRECOMPUTED_SCATTERING:
					// precomputed scattering model
					m_SunPhi += (m_MouseOldXPos - i_XPos) / 800.0f; //azimuth - dX
					m_SunTheta += (m_MouseOldYPos - i_YPos) / 800.0f; //zenith - dY
					break;
				case CustomTypes::Sky::ModelType::MT_CUBE_MAP:
				case CustomTypes::Sky::ModelType::MT_SCATTERING:
					// scattering and cubemap models
					m_SunPhi += (m_MouseOldYPos - i_YPos) / 800.0f; //zenith - dY
					m_SunTheta += (m_MouseOldXPos - i_XPos) / 800.0f; //azimuth - dX
					break;
				case CustomTypes::Sky::ModelType::MT_COUNT:
				default: ERR("Invalid sky model type!");
			}

			m_pSky->SetSunDirection(m_SunPhi, m_SunTheta);

			m_MouseOldXPos = i_XPos;
			m_MouseOldYPos = i_YPos;
		}
	}
	else
	{
		m_MouseOldXPos = i_XPos;
		m_MouseOldYPos = i_YPos;

#ifdef USE_GUI
		if (m_IsGUIVisible && m_IsCursorReleased)
		{
			int ret = TwMouseMotion(i_XPos, i_YPos);
			//assert(ret != 0);
		}
#endif //USE_GUI
	}
}

void Application::OnMouseScroll(int i_XOffset, int i_YOffset)
{
	if (m_pCurrentControllingCamera)
	{
		// NOTE! For now we use only the y offset to update the FOVy

		int fov = m_pCurrentControllingCamera->GetFOV();

		if (fov >= 5 && fov <= 129)
			fov -= i_YOffset;
		if (fov <= 5)
			fov = 5;
		if (fov >= 129)
			fov = 129;

		m_pCurrentControllingCamera->SetFOV(fov);
	}
}

void Application::UpdateCameraMouseOrientation(int i_DX, int i_DY)
{
	// we negate the received deltas for each axis as we want to move the camera in the opposite direction
	float dX = - i_DX * m_MouseSpeed * m_DeltaTime;
	float dY = - i_DY * m_MouseSpeed * m_DeltaTime;

	if (m_pCurrentControllingCamera)
	{
		m_pCurrentControllingCamera->UpdateOrientationWithMouse(dX, dY);
	}

	UpdateCameraMatrices();
}

void Application::CameraMoveForward()
{
	if (!m_IsInBoatMode && m_pCurrentControllingCamera)
	{
		float val = m_KeySpeed * m_DeltaTime;

		m_pCurrentControllingCamera->UpdatePositionWithKeyboard(val, Camera::CAMERA_DIRECTIONS::CD_FORWARD);

		UpdateCameraMatrices();
	}
}

void Application::CameraMoveBackward()
{
	if (!m_IsInBoatMode && m_pCurrentControllingCamera)
	{
		float val = m_KeySpeed * m_DeltaTime;

		m_pCurrentControllingCamera->UpdatePositionWithKeyboard(val, Camera::CAMERA_DIRECTIONS::CD_BACKWARD);

		UpdateCameraMatrices();
	}
}

void Application::CameraMoveRight()
{
	if (!m_IsInBoatMode && m_pCurrentControllingCamera)
	{
		float val = m_KeySpeed * m_DeltaTime;

		m_pCurrentControllingCamera->UpdatePositionWithKeyboard(val, Camera::CAMERA_DIRECTIONS::CD_RIGHT);

		UpdateCameraMatrices();
	}
}

void Application::CameraMoveLeft()
{
	if (!m_IsInBoatMode && m_pCurrentControllingCamera)
	{
		float val = m_KeySpeed * m_DeltaTime;

		m_pCurrentControllingCamera->UpdatePositionWithKeyboard(val, Camera::CAMERA_DIRECTIONS::CD_LEFT);

		UpdateCameraMatrices();
	}
}

void Application::CameraMoveUp()
{
	if (!m_IsInBoatMode && m_pCurrentControllingCamera)
	{
		float val = m_KeySpeed * m_DeltaTime;

		m_pCurrentControllingCamera->UpdatePositionWithKeyboard(val, Camera::CAMERA_DIRECTIONS::CD_UP);

		UpdateCameraMatrices();
	}
}

void Application::CameraMoveDown()
{
	if (!m_IsInBoatMode && m_pCurrentControllingCamera)
	{
		float val = m_KeySpeed * m_DeltaTime;

		m_pCurrentControllingCamera->UpdatePositionWithKeyboard(val, Camera::CAMERA_DIRECTIONS::CD_DOWN);

		UpdateCameraMatrices();
	}
}

void Application::UpdateCameraMatrices()
{
	// update camera
	if (m_pCurrentControllingCamera)
	{
		m_pCurrentControllingCamera->UpdateViewMatrix();
		m_pCurrentControllingCamera->UpdatePerspectiveProjectionMatrix(m_WindowWidth, m_WindowHeight);
	}
}

void Application::BoatAccelerate()
{
	if (m_IsInBoatMode && m_pMotorBoat)
	{
		m_pMotorBoat->Accelerate(m_DeltaTime);
	}
}

void Application::BoatDecelerate()
{
	if (m_IsInBoatMode && m_pMotorBoat)
	{
		m_pMotorBoat->Decelerate(m_DeltaTime);
	}
}

void Application::BoatTurnLeft()
{
	if (m_IsInBoatMode && m_pMotorBoat)
	{
		m_pMotorBoat->TurnLeft(m_DeltaTime);
	}
}

void Application::BoatTurnRight()
{
	if (m_IsInBoatMode && m_pMotorBoat)
	{
		m_pMotorBoat->TurnRight(m_DeltaTime);
	}
}


////////////////////////
void Application::SetWindowSize(int i_WindowWidth, int i_WindowHeight)
{
	assert(i_WindowWidth > 0);
	assert(i_WindowHeight > 0);

	m_WindowWidth = i_WindowWidth;
	m_WindowHeight = i_WindowHeight;
}

int Application::GetWindowWidth()
{
	return m_WindowWidth;
}

int Application::GetWindowHeight()
{
	return m_WindowHeight;
}

float Application::GetCrrTime() const
{
	return m_CrrTime;
}

float Application::GetDeltaTime() const
{
	return m_DeltaTime;
}

float Application::GetKeySpeed() const
{
	return m_KeySpeed;
}

void Application::SetKeySpeed(float i_Value)
{
	m_KeySpeed = i_Value;
}

bool Application::GetIsCursorReleased() const
{
	return m_IsCursorReleased;
}

void Application::SetIsCursorReleased(bool i_Value)
{
	m_IsCursorReleased = i_Value;
}

bool Application::GetIsInDraggingMode() const
{
	return m_IsInDraggingMode;
}

void Application::SetIsInDraggingMode(bool i_Value)
{
	m_IsInDraggingMode = i_Value;
}

bool Application::GetIsRenderWireframe() const
{
	return m_IsRenderWireframe;
}

void Application::SetIsRenderWireframe(bool i_Value)
{
	m_IsRenderWireframe = i_Value;
}

bool Application::GetIsRenderPoints() const
{
	return m_IsRenderPoints;
}

void Application::SetIsRenderPoints(bool i_Value)
{
	m_IsRenderPoints = i_Value;
}

bool Application::GetIsInBoatMode() const
{
	return m_IsInBoatMode;
}

void Application::SetIsInBoatMode(bool i_Value)
{
	m_IsInBoatMode = i_Value;
}

bool Application::GetIsGUIVisible() const
{
	return m_IsGUIVisible;
}

void Application::SetIsGUIVisible(bool i_Value)
{
	m_IsGUIVisible = i_Value;
}

bool Application::GetIsFrustumVisible() const
{
	return m_IsFrustumVisible;
}

void Application::SetIsFrustumVisible(bool i_Value)
{
	m_IsFrustumVisible = i_Value;
}

bool Application::GetIsViewCameraChanged() const
{
	return m_IsCameraViewChanged;
}

void Application::SetIsViewCameraChanged(bool i_Value)
{
	m_IsCameraViewChanged = i_Value;
}

bool Application::GetIsControlCameraChanged() const
{
	return m_IsCameraControlChanged;
}

void Application::SetIsControlCameraChanged(bool i_Value)
{
	m_IsCameraControlChanged = i_Value;
}