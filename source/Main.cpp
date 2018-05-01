/* Author: BAIRAC MIHAI

 PROJECT ENTRY POINT

 The project uses the following libs:

 For OpenGL API
 GLEW: http://glew.sourceforge.net/
 License: https://github.com/nigels-com/glew#copyright-and-licensing

 For Window and Input Management
 GLFW: http://www.glfw.org/
 License: http://www.glfw.org/license.html

 For Mathematics
 GLM: https://glm.g-truc.net/0.9.8/index.html
 License: http://glm.g-truc.net/copying.txt

 For Image loading
 GLI: http://gli.g-truc.net/0.8.2/index.html
 License: http://gli.g-truc.net/copying.txt

 For FFT computation on CPU
 FFTW: http://www.fftw.org/
 License: http://www.fftw.org/doc/License-and-Copyright.html

 For GUI
 AntTweakBar: http://anttweakbar.sourceforge.net/doc/
 License: http://anttweakbar.sourceforge.net/doc/tools:anttweakbar:license

*/




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


#ifdef _DEBUG
//#include "vld.h" //// Visual Leak Detector - really helps !!!
#endif // DEBUG

//#define GLEW_STATIC // when linking to glew32s.lib instead of glew32.lib !
#include "GL/glew.h" //glew include must be set before glfw
#include "GL/glfw3.h"
#include "GUI/AntTweakBar.h"

#include <new>
#include <iostream> //test
#include <sstream>  //fps
#include <ctime>

#include "Common.h"

#include "CustomTypes.h"
#include "HelperFunctions.h"

#define GLM_MESSAGES //info about GLM
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp" //value_ptr()
#include "glm/gtc/matrix_transform.hpp" //scale()

//#define ENABLE_ERROR_CHECK
#include "ErrorHandler.h"

#include "PhysicsConstants.h"

#include "GlobalConfig.h"

///////// Timer - check elapsed time
/** Use to init the clock */
#define TIMER_INIT \
    LARGE_INTEGER frequency; \
    LARGE_INTEGER t1,t2; \
    double elapsedTime; \
    QueryPerformanceFrequency(&frequency);


/** Use to start the performance timer */
#define TIMER_START QueryPerformanceCounter(&t1);

/** Use to stop the performance timer and output the result to the standard stream. Less verbose than \c TIMER_STOP_VERBOSE */
#define TIMER_STOP \
    QueryPerformanceCounter(&t2); \
    elapsedTime=(float)(t2.QuadPart-t1.QuadPart)/frequency.QuadPart; \
    std::wcout<<elapsedTime<<L" sec"<< std::endl;
/////////

#define PPE

#define SKY
#define OCEAN
#define MOTOR_BOAT

GlobalConfig g_Config;

#include "Camera.h"

#if defined(PPE)
#include "PostProcessingManager.h"
#endif // PPE

#if defined(SKY)
#include "Sky.h"
#endif // SKY

#if defined(OCEAN)
#include "Ocean.h"
#endif // OCEAN

#if defined(MOTOR_BOAT)
#include "MotorBoat.h"
#endif // MOTOR_BOAT

///////////////////////////////////////////////////////
int g_WindowWidth = 0, g_WindowHeight = 0;

//////// Auxilliary FBO for reflected and refracted objects!
#include "FrameBufferManager.h"
FrameBufferManager g_FBM;
////////

// pointer to TwBar is required by the AntTweakBar library specs
TwBar *g_pGUIBar = nullptr;

Camera *g_pCamera = nullptr, *g_pObservingCamera = nullptr, *g_pCurrentViewingCamera = nullptr, *g_pCurrentControllingCamera = nullptr;

///////////////////
#if defined(PPE)
PostProcessingManager* g_pPostProcessingManager;
#endif // PPE

#if defined(SKY)
Sky* g_pSky = nullptr;
#endif // SKY

#if defined(OCEAN)
Ocean* g_pOcean = nullptr;
#endif // OCEAN

#if defined(MOTOR_BOAT)
MotorBoat* g_pMotorBoat = nullptr;
#endif // MOTOR_BOAT

///////////////////////////
float g_TimeScale = 0.0f;
bool g_IsGUIVisible = false;

bool g_IsCameraViewChanged = false, g_IsCameraControlChanged = false;
bool g_IsRenderWireframe = false, g_IsRenderPoints = false;
bool g_IsCursorReleased = false;

bool g_IsFrustumVisible = false;

bool g_IsUnderWater = false;

bool g_IsInBoatMode = false;

//// should be put inside InputManager
float g_XPos = 0.0f, g_YPos = 0.0f;
float g_OldXPos = 0.0f, g_OldYPos = 0.0f;
bool g_IsInDraggingMode = false;

bool g_Keys[1024];
float g_KeySpeed = 0.0f;
float g_MouseSpeed = 0.0f;
////

float g_SunPhi = 0.0f, g_SunTheta = 0.0f;

// underwater default framebuffer color
glm::vec3 g_UnderWaterColor;

// default framebuffer color
glm::vec3 g_DefaultColor;


void SetupGlobals ( void )
{
	g_TimeScale = g_Config.Simulation.TimeScale;
	g_IsGUIVisible = g_Config.Simulation.ShowGUI;

	g_KeySpeed = g_Config.Input.KeySpeed;
	g_MouseSpeed = g_Config.Input.MouseSpeed;
}


//// GUI Callbacks
void TW_CALL GetCameraPosition ( void *i_pValue, void *i_pClientData )
{
	*static_cast<glm::vec3 *>(i_pValue) = static_cast<const Camera *>(i_pClientData)->GetPosition();
}

void TW_CALL GetCameraDirection ( void *i_pValue, void *i_pClientData )
{
	*static_cast<glm::vec3 *>(i_pValue) = static_cast<const Camera *>(i_pClientData)->GetForward();
}


#if defined(SKY)
void TW_CALL GetEnabledClouds ( void *i_pValue, void *i_pClientData )
{
	*static_cast<bool *>(i_pValue) = static_cast<const Sky *>(i_pClientData)->GetEnabledClouds();
}

void TW_CALL SetEnabledClouds ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Sky *>(i_pClientData)->SetEnabledClouds(*static_cast<const bool *>(i_pValue));
}

void TW_CALL GetCloudsOctaves ( void *i_pValue, void *i_pClientData )
{
	*static_cast<unsigned short *>(i_pValue) = static_cast<const Sky *>(i_pClientData)->GetCloudsOctaves();
}

void TW_CALL SetCloudsOctaves ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Sky *>(i_pClientData)->SetCloudsOctaves(*static_cast<const unsigned short *>(i_pValue));
}

void TW_CALL GetCloudsLacunarity ( void *i_pValue, void *i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Sky *>(i_pClientData)->GetCloudsLacunarity();
}

void TW_CALL SetCloudsLacunarity ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Sky *>(i_pClientData)->SetCloudsLacunarity(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetCloudsGain ( void *i_pValue, void *i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Sky *>(i_pClientData)->GetCloudsGain();
}

void TW_CALL SetCloudsGain ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Sky *>(i_pClientData)->SetCloudsGain(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetCloudsScaleFactor ( void *i_pValue, void *i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Sky *>(i_pClientData)->GetCloudsScaleFactor();
}

void TW_CALL SetCloudsScaleFactor ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Sky *>(i_pClientData)->SetCloudsScaleFactor(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetCloudsNorm ( void *i_pValue, void *i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Sky *>(i_pClientData)->GetCloudsNorm();
}

void TW_CALL SetCloudsNorm ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Sky *>(i_pClientData)->SetCloudsNorm(*static_cast<const float *>(i_pValue));
}
#endif // SKY

#if defined(OCEAN)
void TW_CALL SetOceanAmplitude(const void* i_pValue, void* i_pClientData)
{
	static_cast<Ocean *>(i_pClientData)->SetWaveAmplitude(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetOceanAmplitude ( void* i_pValue, void* i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetWaveAmplitude();
}

void TW_CALL SetOceanPatchSize ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Ocean *>(i_pClientData)->SetPatchSize(*static_cast<const unsigned short *>(i_pValue));
}

void TW_CALL GetOceanPatchSize ( void* i_pValue, void* i_pClientData )
{
	*static_cast<unsigned short *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetPatchSize();
}

void TW_CALL SetOceanTileScale ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Ocean *>(i_pClientData)->SetTileScale(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetOceanTileScale ( void* i_pValue, void* i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetTileScale();
}

void TW_CALL SetOceanWindSpeed ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Ocean *>(i_pClientData)->SetWindSpeed(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetOceanWindSpeed ( void* i_pValue, void* i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetWindSpeed();
}

void TW_CALL SetOceanWindDirectionX ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Ocean *>(i_pClientData)->SetWindDirectionX(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetOceanWindDirectionX ( void* i_pValue, void* i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetWindDirectionX();
}

void TW_CALL SetOceanWindDirectionZ ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Ocean *>(i_pClientData)->SetWindDirectionZ(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetOceanWindDirectionZ ( void* i_pValue, void* i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetWindDirectionZ();
}

void TW_CALL GetOceanWindDir ( void* i_pValue, void* i_pClientData )
{
	*static_cast<glm::vec3 *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetWindDir();
}

void TW_CALL SetOceanOpposingWavesFactor ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Ocean *>(i_pClientData)->SetOpposingWavesFactor(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetOceanOpposingWavesFactor ( void* i_pValue, void* i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetOpposingWavesFactor();
}

void TW_CALL SetOceanVerySmallWavesFactor ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Ocean *>(i_pClientData)->SetVerySmallWavesFactor(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetOceanVerySmallWavesFactor ( void* i_pValue, void* i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetVerySmallWavesFactor();
}

void TW_CALL SetOceanChoppyScale ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Ocean *>(i_pClientData)->SetChoppyScale(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetOceanChoppyScale ( void* i_pValue, void* i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetChoppyScale();
}

void TW_CALL SetOceanNumberOfSamples ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Ocean *>(i_pClientData)->SetGodRaysNumberOfSamples(*static_cast<const unsigned short *>(i_pValue));
}

void TW_CALL GetOceanNumberOfSamples ( void* i_pValue, void* i_pClientData )
{
	*static_cast<unsigned short *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetGodRaysNumberOfSamples();
}

void TW_CALL SetOceanExposure ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Ocean *>(i_pClientData)->SetGodRaysExposure(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetOceanExposure ( void* i_pValue, void* i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetGodRaysExposure();
}

void TW_CALL SetOceanDecay ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Ocean *>(i_pClientData)->SetGodRaysDecay(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetImprovedOceanDecay ( void* i_pValue, void* i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetGodRaysDecay();
}

void TW_CALL SetOceanDensity ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Ocean *>(i_pClientData)->SetGodRaysDensity(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetOceanDensity ( void* i_pValue, void* i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetGodRaysDensity();
}

void TW_CALL SetOceanWeight ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Ocean *>(i_pClientData)->SetGodRaysWeight(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetImprovedOceanWeight ( void* i_pValue, void* i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Ocean *>(i_pClientData)->GetGodRaysWeight();
}
#endif // OCEAN

void TW_CALL SetFOV ( const void* i_pValue, void* i_pClientData )
{
	static_cast<Camera *>(i_pClientData)->SetFOV(*static_cast<const float *>(i_pValue));
}

void TW_CALL GetFOV ( void* i_pValue, void* i_pClientData )
{
	*static_cast<float *>(i_pValue) = static_cast<const Camera *>(i_pClientData)->GetFOV();
}

// Small function that converts LDR to HDR
glm::vec3 HDR ( glm::vec3 L )
{
	L = L * g_Config.Rendering.HDR.Exposure;
	L.r = L.r < 1.413f ? pow(L.r * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(-L.r);
	L.g = L.g < 1.413f ? pow(L.g * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(-L.g);
	L.b = L.b < 1.413f ? pow(L.b * 0.38317f, 1.0f / 2.2f) : 1.0f - exp(-L.b);
	return L;
}

void InitScene ( void )
{
	// compute default framebuffer underwater color !!!	
	g_UnderWaterColor = HDR(g_Config.Scene.Ocean.UnderWater.Fog.Color);

	// default framebuffer color
	g_DefaultColor.r = 0.0f;
	g_DefaultColor.g = 0.0f;
	g_DefaultColor.b = 0.0f;

	////////////////

	g_pCamera = new Camera("Rendering", g_Config);
	assert(g_pCamera != nullptr);

	if (g_pCamera)
	{
		g_pCamera->UpdatePerspectiveProjectionMatrix(g_WindowWidth, g_WindowHeight);
	}

	g_pObservingCamera = new Camera("Observing", g_Config);
	assert(g_pObservingCamera != nullptr);

	if (g_pObservingCamera)
	{
		g_pObservingCamera->UpdatePerspectiveProjectionMatrix(g_WindowWidth, g_WindowHeight);
	}

	g_pCurrentViewingCamera = g_pCamera;
	g_pCurrentControllingCamera = g_pCamera;

	g_IsCursorReleased = false;
	g_IsRenderWireframe = false;
	g_IsRenderPoints = false;
	g_IsCameraViewChanged = false;
	g_IsCameraControlChanged = false;

	//////////////////////////////////////////////////////

#if defined(PPE)
	g_pPostProcessingManager = new PostProcessingManager(g_Config);
	assert(g_pPostProcessingManager != nullptr);
	std::cout << "PostProcessingManager is: " << sizeof(*g_pPostProcessingManager) << " bytes in size" << std::endl;
#endif // PPE

	//////////// GUI ///////////
	// add params to GUI
	int ret = 0;

	ret = TwAddVarRW(g_pGUIBar, "TimeScale", TW_TYPE_FLOAT, &g_TimeScale, "min=-20.0f; max=20.0f; step=1.0f group=Simulation");
	assert(ret != 0);

	ret = TwAddVarCB(g_pGUIBar, "CameraPosition", TW_TYPE_DIR3F, nullptr, GetCameraPosition, g_pCurrentControllingCamera, "group=Camera");
	assert(ret != 0);
	ret = TwAddVarCB(g_pGUIBar, "CameraDirection", TW_TYPE_DIR3F, nullptr, GetCameraDirection, g_pCurrentControllingCamera, "group=Camera");
	assert(ret != 0);


	//////// SKY ////////
#if defined(SKY)
	g_pSky = new Sky(g_Config);
	assert(g_pSky != nullptr);
	std::cout << "Sky is: " << sizeof(*g_pSky) << " bytes in size" << std::endl;

	if (g_pSky && g_pSky->GetModelType() != CustomTypes::Sky::ModelType::MT_CUBE_MAP)
	{
		ret = TwAddVarCB(g_pGUIBar, "Enabled", TW_TYPE_BOOL16, SetEnabledClouds, GetEnabledClouds, g_pSky, "group=Clouds");
		assert(ret != 0);
		ret = TwAddVarCB(g_pGUIBar, "Octaves", TW_TYPE_UINT16, SetCloudsOctaves, GetCloudsOctaves, g_pSky, "min=1; max=15.0; step=1 group=Clouds");
		assert(ret != 0);
		ret = TwAddVarCB(g_pGUIBar, "Lacunarity", TW_TYPE_FLOAT, SetCloudsLacunarity, GetCloudsLacunarity, g_pSky, "min=0.1; max=3.0; step=0.1 group=Clouds");
		assert(ret != 0);
		ret = TwAddVarCB(g_pGUIBar, "Gain", TW_TYPE_FLOAT, SetCloudsGain, GetCloudsGain, g_pSky, "min=0.1; max=1.0; step=0.1 group=Clouds");
		assert(ret != 0);

		if (g_pSky->GetModelType() == CustomTypes::Sky::ModelType::MT_SCATTERING)
		{
			ret = TwAddVarCB(g_pGUIBar, "ScaleFactor", TW_TYPE_FLOAT, SetCloudsScaleFactor, GetCloudsScaleFactor, g_pSky, "min=0.1; max=2.0; step=0.1 group=Clouds");
			assert(ret != 0);
		}
		else if (g_pSky->GetModelType() == CustomTypes::Sky::ModelType::MT_PRECOMPUTED_SCATTERING)
		{
			ret = TwAddVarCB(g_pGUIBar, "Norm", TW_TYPE_FLOAT, SetCloudsNorm, GetCloudsNorm, g_pSky, "min=0.1; max=1.0; step=0.1 group=Clouds");
			assert(ret != 0);
		}
	}
#endif // SKY

#if defined(OCEAN)
	g_pOcean = new Ocean(g_Config);
	assert(g_pOcean != nullptr);
	std::cout << "Ocean is: " << sizeof(*g_pOcean) << " bytes in size" << std::endl;

	// add params to GUI
	ret = TwAddVarCB(g_pGUIBar, "Amplitude", TW_TYPE_FLOAT, SetOceanAmplitude, GetOceanAmplitude, g_pOcean, "min=0.01; max=1.0; step=0.01 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(g_pGUIBar, "PatchSize", TW_TYPE_UINT16, SetOceanPatchSize, GetOceanPatchSize, g_pOcean, "min=128.0; max=1024.0; step=64.0 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(g_pGUIBar, "TileScale", TW_TYPE_FLOAT, SetOceanTileScale, GetOceanTileScale, g_pOcean, "min=0.01; max=5.0; step=0.01 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(g_pGUIBar, "WindSpeed", TW_TYPE_FLOAT, SetOceanWindSpeed, GetOceanWindSpeed, g_pOcean, "min=0.5; max=200; step=1.0 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(g_pGUIBar, "WindDirectionX", TW_TYPE_FLOAT, SetOceanWindDirectionX, GetOceanWindDirectionX, g_pOcean, "min=-1.0; max=1.0; step=0.2 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(g_pGUIBar, "WindDirectionZ", TW_TYPE_FLOAT, SetOceanWindDirectionZ, GetOceanWindDirectionZ, g_pOcean, "min=-1.0; max=1.0; step=0.2 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(g_pGUIBar, "WindDir", TW_TYPE_DIR3F, nullptr, GetOceanWindDir, g_pOcean, "group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(g_pGUIBar, "OpposingWavesFactor", TW_TYPE_FLOAT, SetOceanOpposingWavesFactor, GetOceanOpposingWavesFactor, g_pOcean, "min=0.0001; max=3.0; step=0.01 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(g_pGUIBar, "VerySmallWavesFactor", TW_TYPE_FLOAT, SetOceanVerySmallWavesFactor, GetOceanVerySmallWavesFactor, g_pOcean, "min=0.0001; max=0.1; step=0.0001 group=Waves");
	assert(ret != 0);
	ret = TwAddVarCB(g_pGUIBar, "ChoppyScale", TW_TYPE_FLOAT, SetOceanChoppyScale, GetOceanChoppyScale, g_pOcean, "min=0.1; max=3.0; step=0.1 group=Waves");
	assert(ret != 0);
#endif // OCEAN

	// add params to GUI
	ret = TwAddVarCB(g_pGUIBar, "FOV", TW_TYPE_FLOAT, SetFOV, GetFOV, g_pCurrentControllingCamera, "min=5.0; max=129.0; step=1.0 group=Rendering");
	assert(ret != 0);

#if defined(MOTOR_BOAT)
	g_pMotorBoat = new MotorBoat(g_Config);
	assert(g_pMotorBoat != nullptr);
	std::cout << "MotorBoat is: " << sizeof(*g_pMotorBoat) << " bytes in size" << std::endl;
#endif // MOTOR_BOAT
}

void UpdatePPE ( float i_CrrTime, float i_DeltaTime )
{
#if defined(PPE)
	if (g_pPostProcessingManager && g_Config.VisualEffects.PostProcessing.Enabled)
	{
		g_pPostProcessingManager->Update(i_CrrTime, i_DeltaTime);
	}
#endif // PPE
}

void UpdateCameraInput ( GLFWwindow* i_pWindow, float i_DeltaTime )
{
	if (g_pCurrentControllingCamera)
	{
		if (!g_IsInBoatMode)
		{
			//////////// KEYBOARD
			float val = g_KeySpeed * i_DeltaTime;

			if (g_Keys[GLFW_KEY_W]) g_pCurrentControllingCamera->UpdatePositionWithKeyboard(val, Camera::CAMERA_DIRECTIONS::CD_FORWARD);
			if (g_Keys[GLFW_KEY_S]) g_pCurrentControllingCamera->UpdatePositionWithKeyboard(val, Camera::CAMERA_DIRECTIONS::CD_BACKWARD);
			if (g_Keys[GLFW_KEY_D]) g_pCurrentControllingCamera->UpdatePositionWithKeyboard(val, Camera::CAMERA_DIRECTIONS::CD_RIGHT);
			if (g_Keys[GLFW_KEY_A]) g_pCurrentControllingCamera->UpdatePositionWithKeyboard(val, Camera::CAMERA_DIRECTIONS::CD_LEFT);
			if (g_Keys[GLFW_KEY_U]) g_pCurrentControllingCamera->UpdatePositionWithKeyboard(val, Camera::CAMERA_DIRECTIONS::CD_UP);
			if (g_Keys[GLFW_KEY_B]) g_pCurrentControllingCamera->UpdatePositionWithKeyboard(val, Camera::CAMERA_DIRECTIONS::CD_DOWN);

			/////////// MOUSE
			if (!g_IsCursorReleased)
			{
				// update mouse data
				double xPos = 0.0, yPos = 0.0;
				glfwGetCursorPos(i_pWindow, &xPos, &yPos);

				float halfWidth = g_WindowWidth * 0.5f, halfHeight = g_WindowHeight * 0.5f;

				float dX = (halfWidth - static_cast<float>(xPos)) * g_MouseSpeed * i_DeltaTime;
				float dY = (halfHeight - static_cast<float>(yPos)) * g_MouseSpeed * i_DeltaTime;

				if (g_pCurrentControllingCamera)
				{
					g_pCurrentControllingCamera->UpdateOrientationWithMouse(dX, dY);
				}

				// reset the cursor position for next frame
				glfwSetCursorPos(i_pWindow, halfWidth, halfHeight);
			}
			// update camera
			g_pCurrentControllingCamera->UpdateViewMatrix();
			g_pCurrentControllingCamera->UpdatePerspectiveProjectionMatrix(g_WindowWidth, g_WindowHeight);
		}
	}
}

void UpdateMotorBoatInput ( float i_CrrTime, float i_DeltaTime )
{
#if defined(MOTOR_BOAT)
	if (g_IsInBoatMode && g_pMotorBoat)
	{
		if (g_Keys[GLFW_KEY_W]) g_pMotorBoat->Accelerate(i_DeltaTime);
		if (g_Keys[GLFW_KEY_S]) g_pMotorBoat->Decelerate(i_DeltaTime);
		if (g_Keys[GLFW_KEY_D]) g_pMotorBoat->TurnRight(i_DeltaTime);
		if (g_Keys[GLFW_KEY_A]) g_pMotorBoat->TurnLeft(i_DeltaTime);
	}
#endif // MOTOR_BOAT
}

void UpdateReflectedScene ( float i_CrrTime, float i_DeltaTime )
{
	// Local Reflection
	// Transforms order: S * R * T (from left to right)
	glm::mat4 ScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));

	//// update the objects in scene

#if defined(MOTOR_BOAT) && defined(SKY)
	if (g_pMotorBoat && g_pSky && g_pCurrentViewingCamera)
	{
		g_pMotorBoat->UpdateReflected(ScaleMatrix, *g_pCurrentViewingCamera, g_pSky->GetSunDirection(), g_IsRenderWireframe, i_CrrTime);
	}
#endif // MOTOR_BOAT && SKY

#if defined(SKY)
	if (g_pSky && g_pCurrentViewingCamera)
	{
		g_pSky->UpdateReflected(ScaleMatrix, *g_pCurrentViewingCamera, g_pCurrentViewingCamera->GetAltitude() < 0.0f, g_IsRenderWireframe, i_CrrTime);
	}
#endif // SKY
}

void UpdateRefractedScene ( float i_CrrTime, float i_DeltaTime )
{
	// Local Refraction
	glm::mat4 ScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.75f, 1.0f));

	//// update the objects in scene

#if defined(MOTOR_BOAT) && defined(SKY)
	if (g_pMotorBoat && g_pSky && g_pCurrentViewingCamera)
	{
		g_pMotorBoat->UpdateRefracted(ScaleMatrix, *g_pCurrentViewingCamera, g_pSky->GetSunDirection(), g_IsRenderWireframe, i_CrrTime);
	}
#endif // MOTOR_BOAT && SKY

#if defined(SKY)
	if (g_pSky && g_pCurrentViewingCamera)
	{
		g_pSky->UpdateRefracted(ScaleMatrix, *g_pCurrentViewingCamera, g_pCamera->GetAltitude() < 0.0f, g_IsRenderWireframe, i_CrrTime);
	}
#endif // SKY
}

/*
More info about boat buoyancy:
 
 http://www.gamasutra.com/view/news/237528/Water_interaction_model_for_boats_in_video_games.php
 http://www.gamasutra.com/view/news/263237/Water_interaction_model_for_boats_in_video_games_Part_2.php
 http://www.habrador.com/tutorials/unity-boat-tutorial/
*/

void ComputeBuoyancy ( float i_DeltaTime )
{
#if defined(OCEAN) && defined(MOTOR_BOAT)
	// Compute Boat Buoyancy
	if (g_pOcean && g_pMotorBoat && g_pMotorBoat->GetBoatVelocity() < glm::epsilon<float>() && g_Config.Scene.Ocean.Surface.BoatEffects.Buoyancy.Enabled)
	{
		static float boatYVelocity = 0.0f;

		static float boatYPos = g_pMotorBoat->GetBoatYPos();

		float waterHeight = g_pOcean->ComputeWaterHeightAt(g_pMotorBoat->GetBoatPosition().xz());
		waterHeight *= 0.5f; // reduce the height for better approximation of the effect

		float waterDisplacedVolume = g_pMotorBoat->GetBoatArea() * glm::clamp(waterHeight - (boatYPos - g_pMotorBoat->GetBoatHeight() * 0.5f), 0.0f, g_pMotorBoat->GetBoatHeight());

		// compute forces of buoyancy

		// gravitational force
		float gravitationalForce = g_pMotorBoat->GetBoatMass() * - PhysicsConstants::kG;

		// buoyancy force
		float buoyancyForce = waterDisplacedVolume * PhysicsConstants::kWaterDensity * PhysicsConstants::kG;

		// drag force - computed only when the boat is in water
		float dragForce = 0.0f;
		if (boatYPos < waterHeight)
		{
			dragForce = -0.5f * (boatYVelocity * boatYVelocity) * PhysicsConstants::kWaterDensity * g_pMotorBoat->GetBoatArea() * g_pMotorBoat->GetBoatDragCoefficient();

			if (boatYVelocity < 0.0f)
			{
				dragForce *= -1.0f;
			}
		}

		float netForce = gravitationalForce + buoyancyForce + dragForce;

		float boatYAccel = netForce / g_pMotorBoat->GetBoatMass();

		boatYPos += boatYAccel * i_DeltaTime * g_pMotorBoat->GetBoatYAccelerationFactor();

		g_pMotorBoat->SetBoatYPos(boatYPos);
	}
#endif // OCEAN && MOTOR_BOAT
}

void UpdateScene ( float i_CrrTime, float i_DeltaTime )
{
#if defined(SKY)
	if (g_pSky && g_pCurrentViewingCamera && g_pCurrentViewingCamera->GetAltitude() > -10.0f)
	{
		g_pSky->Update(*g_pCurrentViewingCamera, g_pCurrentViewingCamera->GetAltitude() < 0.0f, g_IsRenderWireframe, i_CrrTime);
	}
#endif // SKY

#if defined(OCEAN) && defined(SKY)
	if (g_pOcean && g_pSky && g_pCamera)
	{
		g_pOcean->Update(*g_pCamera, g_pSky->GetSunDirection(), g_IsRenderWireframe, g_IsFrustumVisible, i_CrrTime);
	}
#endif // OCEAN && SKY

#if defined(OCEAN) && defined(MOTOR_BOAT)
	if (g_pOcean && g_pMotorBoat)
	{
		g_pOcean->UpdateBoatEffects(*g_pMotorBoat);
	}
#endif // OCEAN && MOTOR_BOAT

#if defined(MOTOR_BOAT) && defined(SKY)
	if (g_pMotorBoat && g_pSky && g_pCurrentViewingCamera)
	{
		g_pMotorBoat->Update(*g_pCurrentViewingCamera, g_pSky->GetSunDirection(), g_IsRenderWireframe, i_CrrTime);
	}
#endif // MOTOR_BOAT && SKY

	ComputeBuoyancy(i_DeltaTime);
}

void RenderPPE ( void )
{
#if defined(PPE)
	if (g_pPostProcessingManager && g_Config.VisualEffects.PostProcessing.Enabled)
	{
		g_pPostProcessingManager->Render();
	}
#endif // PPE
}

void RenderReflectedScene ( void )
{
	// Local Reflection
	g_FBM.Bind();
	g_FBM.SetupDrawBuffers(1, 0);

	// clear the buffers for the current frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CLIP_DISTANCE0);

	// render the reflected objects in scene


	// NOTE! In our case we render above water boat reflection
#if defined(MOTOR_BOAT)
	if (g_pMotorBoat)
	{
		g_pMotorBoat->RenderReflected();
	}
#endif // MOTOR_BOAT

	// the sky has to be drawn last !!!
	// as explined in here: https://learnopengl.com/#!Advanced-OpenGL/Cubemaps
#if defined(SKY)
	if (g_pSky)
	{
		g_pSky->RenderReflected();
	}
#endif // SKY

	glDisable(GL_CLIP_DISTANCE0);

	g_FBM.UnBind();
}

void RenderRefractedScene ( void )
{
	// Local Refraction
	g_FBM.Bind();
	g_FBM.SetupDrawBuffers(1, 1);

	// clear the buffers for the current frame
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CLIP_DISTANCE1);

	// render the refracted objects in scene

	// NOTE! In our case we render under water boat refraction
#if defined(MOTOR_BOAT)
	if (g_pMotorBoat)
	{
		g_pMotorBoat->RenderRefracted();
	}
#endif // MOTOR_BOAT

	// the sky has to be drawn last !!!
	// as explined in here: https://learnopengl.com/#!Advanced-OpenGL/Cubemaps
#if defined(SKY)
	if (g_pSky)
	{
		g_pSky->RenderRefracted();
	}
#endif // SKY

	glDisable(GL_CLIP_DISTANCE1);

	g_FBM.UnBind();
}

void RenderScene ( void )
{
#if defined(PPE)
	if (g_pPostProcessingManager && g_Config.VisualEffects.PostProcessing.Enabled)
	{
		g_pPostProcessingManager->BindFB();
	}
#endif // PPE

	//// clear the buffers for the current frame

	glm::vec3 color = (g_pCamera->GetAltitude() > 0.0f ? g_DefaultColor : g_UnderWaterColor);
	
	glClearColor(color.r, color.g, color.b, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//// RENDERING SCENE ////

	if (g_Config.VisualEffects.PostProcessing.Enabled == false)
	{
		// NOTE! Because of the underwater godrays screen space algorithm
		// all opaque objects in scene must be rendered before the ocean is !!!
#if defined(MOTOR_BOAT)
		if (g_pMotorBoat)
		{
			g_pMotorBoat->Render();
		}
#endif // MOTOR_BOAT
	}
////////////////////

#if defined(MOTOR_BOAT)
	if (g_pMotorBoat && g_pCurrentViewingCamera && g_pCurrentViewingCamera->GetAltitude() > 0.0f
		&& g_Config.Scene.Ocean.Surface.BoatEffects.HideInsideWater)
	{
		///////////// MAKE THE WATER IN THE BOAT DISAPPEAR !!! ///////////
		// actually what this does - creates a hole in the ocean in the place of the flattened boat
		glEnable(GL_STENCIL_TEST);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		glDepthMask(GL_FALSE);
		glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		g_pMotorBoat->RenderFlattened();

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDepthMask(GL_TRUE);
		///////////////
	}
#endif // MOTOR_BOAT

#if defined(OCEAN)
	g_FBM.BindColorAttachmentByIndex(0, true); // reflection map
	g_FBM.BindColorAttachmentByIndex(1, true); // refraction map

	if (g_pOcean && g_pCurrentViewingCamera)
	{
		g_pOcean->Render(*g_pCurrentViewingCamera);
	}
#endif // OCEAN

#if defined(MOTOR_BOAT)
	if (g_pCurrentViewingCamera && g_pCurrentViewingCamera->GetAltitude() >= 0.0f
		&& g_Config.Scene.Ocean.Surface.BoatEffects.HideInsideWater)
	{
		glDisable(GL_STENCIL_TEST);
	}
#endif // MOTOR_BOAT
	/////////////////////////////////////

	// the sky has to be drawn last !!!
	// as explined in here: https://learnopengl.com/#!Advanced-OpenGL/Cubemaps

#if defined(SKY)
	if (g_pSky && g_pCurrentViewingCamera && g_pCurrentViewingCamera->GetAltitude() > 0.0f)
	{
		g_pSky->Render();
	}
#endif // SKY

	if (g_Config.VisualEffects.PostProcessing.Enabled)
	{
#if defined(MOTOR_BOAT)
		if (g_pMotorBoat)
		{
			g_pMotorBoat->Render();
		}
#endif // MOTOR_BOAT
	}

#if defined(PPE)
	if (g_pPostProcessingManager && g_Config.VisualEffects.PostProcessing.Enabled)
	{
		g_pPostProcessingManager->UnBindFB();
	}
#endif // PPE
}

void TerminateScene ( void )
{
	g_pCurrentViewingCamera = nullptr;
	g_pCurrentControllingCamera = nullptr;
	SAFE_DELETE(g_pCamera);
	SAFE_DELETE(g_pObservingCamera);

#if defined(PPE)
	SAFE_DELETE(g_pPostProcessingManager);
#endif // PPE

#if defined(SKY)
	SAFE_DELETE(g_pSky);
#endif // SKY

#if defined(OCEAN)
	SAFE_DELETE(g_pOcean);
#endif // OCEAN

#if defined(MOTOR_BOAT)
	SAFE_DELETE(g_pMotorBoat);
#endif // MOTOR_BOAT
}

//////////////////////////////////////////////

// GLFW callbacks
static void ErrorCB ( int i_Error, const char* i_pDescription )
{
	fputs(i_pDescription, stderr);
}

static void KeyCB ( GLFWwindow* i_pWindow, int i_Key, int i_ScanCode, int i_Action, int i_Mods )
{
	// window control
	if (i_Key == GLFW_KEY_ESCAPE && i_Action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(i_pWindow, true);
	}

	if (i_Key == GLFW_KEY_LEFT_SHIFT)
	{
		if (i_Action == GLFW_PRESS)
		{
			g_KeySpeed *= 10.0f;
		}
		else if(i_Action == GLFW_RELEASE)
		{
			g_KeySpeed /= 10.0f;
		}
	}

	// this setup allows us to use multiple camera keys simultaneously!
	if (i_Key >= 0 && i_Key < 1024)
	{
		if (i_Action == GLFW_PRESS)
		{
			g_Keys[i_Key] = true;
		}
		else if (i_Action == GLFW_RELEASE)
		{
			g_Keys[i_Key] = false;
		}
	}

	//// OTHER INPUTS - I treat these here, because I need this to happen once(when the event is triggered), not in update
	if (i_Action == GLFW_RELEASE)
	{
		// other controls
		switch (i_Key)
		{
		case GLFW_KEY_GRAVE_ACCENT: //"`"
			g_IsCursorReleased = !g_IsCursorReleased;
			if (g_IsCursorReleased)
			{
				// show the cursor
				glfwSetInputMode(i_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else
			{
				g_IsInDraggingMode = false;

				// hide the cursor + grabs it providing unlimited 360 cursor movement (usefull for camera)
				glfwSetInputMode(i_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			break;
		case GLFW_KEY_1:
			g_IsRenderWireframe = !g_IsRenderWireframe;
#if  !defined(OCEAN)
			if (g_IsRenderWireframe)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //not available in OpenGL ES
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
#endif // OCEAN
			break;
		case GLFW_KEY_2:
			g_IsRenderPoints = !g_IsRenderPoints;
			if (g_IsRenderPoints)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			break;
		case GLFW_KEY_3:
			g_IsInBoatMode = !g_IsInBoatMode;			
			break;
		case GLFW_KEY_4:
			g_IsGUIVisible = !g_IsGUIVisible;
			break;
		case GLFW_KEY_C:
			if (g_IsCameraViewChanged)
			{
				g_pCurrentViewingCamera = g_pCamera;
				g_IsCameraViewChanged = false;
			}
			else
			{
				g_pCurrentViewingCamera = g_pObservingCamera;
				g_IsCameraViewChanged = true;
			}
			break;
		case GLFW_KEY_V:
			if (g_IsCameraControlChanged)
			{
				g_pCurrentControllingCamera = g_pCamera;
				g_IsCameraControlChanged = false;
			}
			else
			{
				g_pCurrentControllingCamera = g_pObservingCamera;
				g_IsCameraControlChanged = true;
			}
			break;
		case GLFW_KEY_F:
#if defined(OCEAN)
			g_IsFrustumVisible = !g_IsFrustumVisible;
#endif // OCEAN
			break;
		case GLFW_KEY_R:
			if (g_pCurrentControllingCamera)
			{
				g_pCurrentControllingCamera->ResetFOV();
			}
			break;
		}
	}
}

static void ButtonCB ( GLFWwindow* i_pWindow, int i_Button, int i_Action, int i_Mods )
{
	g_IsInDraggingMode = false;

	if (i_Button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (g_IsGUIVisible)
		{
			if (!TwEventMouseButtonGLFW(i_Button, i_Action))
			{
				if (GLFW_PRESS == i_Action)
				{
					g_IsInDraggingMode = true;
				}
			}
		}
		else
		{
			if (GLFW_PRESS == i_Action)
			{
				g_IsInDraggingMode = true;
			}
		}
	}
}


static void CursorPositionCB ( GLFWwindow* i_pWindow, double i_XPos, double i_YPos )
{
	if (g_IsInDraggingMode)
	{
		// more info: http://mathworld.wolfram.com/SphericalCoordinates.html

#if defined(SKY)
		if (g_pSky && g_pSky->GetAllowChangeDirWithMouse())
		{

			if (g_pSky->GetModelType() == CustomTypes::Sky::ModelType::MT_PRECOMPUTED_SCATTERING)
			{
				// precomputed scattering model
				g_SunPhi += (g_OldXPos - static_cast<float>(i_XPos)) / 800.0f; //azimuth - dX
				g_SunTheta += (g_OldYPos - static_cast<float>(i_YPos)) / 800.0f; //zenith - dY
			}
			else
			{
				// scattering and cubemap models
				g_SunPhi += (g_OldYPos - static_cast<float>(i_YPos)) / 800.0f; //zenith - dY
				g_SunTheta += (g_OldXPos - static_cast<float>(i_XPos)) / 800.0f; //azimuth - dX
			}

			g_pSky->SetSunDirection(g_SunPhi, g_SunTheta);

			g_OldXPos = static_cast<float>(i_XPos);
			g_OldYPos = static_cast<float>(i_YPos);
		}
#endif // SKY
	}
	else
	{
		g_OldXPos = static_cast<float>(i_XPos);
		g_OldYPos = static_cast<float>(i_YPos);

		if (g_IsGUIVisible)
		{
			int ret = TwMouseMotion(static_cast<int>(i_XPos), static_cast<int>(i_YPos));
			//assert(ret != 0);
		}
	}
}

static void ScrollCB ( GLFWwindow* pWindow, double i_Xoffset, double i_Yoffset )
{
	if (g_pCurrentControllingCamera)
	{
		float fov = g_pCurrentControllingCamera->GetFOV();

		if (fov >= 5.0f && fov <= 129.0f)
			fov -= static_cast<float>(i_Yoffset);
		if (fov <= 5.0f)
			fov = 5.0f;
		if (fov >= 129.0f)
			fov = 129.0f;

		g_pCurrentControllingCamera->SetFOV(fov);
	}
}

void SetupAuxiliaryFrameBuffer(void);

static void ResizeWindowCB ( GLFWwindow* i_pWindow, int i_Width, int i_Height )
{
	g_WindowWidth = i_Width;
	g_WindowHeight = i_Height;

	glViewport(0, 0, g_WindowWidth, g_WindowHeight);

	SetupAuxiliaryFrameBuffer();

	if (g_pCurrentControllingCamera)
	{
		g_pCurrentControllingCamera->UpdatePerspectiveProjectionMatrix(g_WindowWidth, g_WindowHeight);
	}

	if (g_IsGUIVisible)
	{
		int ret = TwWindowSize(g_WindowWidth, g_WindowHeight);
		assert(ret != 0);
	}

#if defined(OCEAN)
	if (g_pOcean)
	{
		g_pOcean->UpdateGrid(g_WindowWidth, g_WindowHeight);
	}
#endif OCEAN

	// TODO - All textures that depend on window size like: reflection, refraction, post processing, etc.
	// should be resized here !!!
}

void SetupDefaultFrameBuffer ( void )
{
	
}

void SetupAuxiliaryFrameBuffer ( void )
{
	g_FBM.Initialize("Main Auxiliary FrameBuffer");
	g_FBM.CreateSimple(2, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, g_WindowWidth, g_WindowHeight, GL_CLAMP_TO_EDGE, GL_LINEAR, g_Config.TexUnit.Global.ReflectionMap, 5, false, FrameBufferManager::DEPTH_BUFFER_TYPE::DBT_RENDER_BUFFER_DEPTH_STENCIL);
}

void SetupGL ( void )
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
	glEnable(GL_STENCIL_TEST);

	// Point Size
	glPointSize(1.0f);
	glEnable(GL_PROGRAM_POINT_SIZE);

	// Anti-Aliasing Sample Mode
	glEnable(GL_MULTISAMPLE); //anti-aliasing

	////////////////////////////////////////////////
	SetupDefaultFrameBuffer();

	SetupAuxiliaryFrameBuffer();

	// Viewport
	glViewport(0, 0, g_WindowWidth, g_WindowHeight);
}

GLFWwindow* InitGLContext(void)
{
	//////////////// Init GLFW Context - one OpenGL context + one window /////////////////

	// Set error callback - MUST be set before GLFW init
	GLFWwindow* pWindow = nullptr;
	glfwSetErrorCallback(ErrorCB);

	// Init GLFW context
	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW!" << std::endl;
		return nullptr;
	}

	// NOTE! The application may run faster without windows hints !!!
	// !!!  IMPORTANT !!! On other PCs or nVidia cards the app may not work with this activated !!!
	if (g_Config.Window.UseWindowHints)
	{
		//// Window hints - these hints work only for OpenGL 3.2 and above !!!
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, g_Config.OpenGLContext.OpenGLVersion.major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, g_Config.OpenGLContext.OpenGLVersion.minor);
		////

		//// OpenGL profiles only work for OpenGL 3.2 and above !!!
		if (g_Config.OpenGLContext.OpenGLVersion.minor >= 2)
		{	
			if (g_Config.OpenGLContext.IsCoreProfile)
			{
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			}
			else
			{
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
			}
		}
		////

		//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		//	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

		//	glfwWindowHint(GLFW_SAMPLES, 8); // big perfornace penalty!
		//	glfwWindowHint(GLFW_STEREO, GL_FALSE);

		glfwWindowHint(GLFW_RESIZABLE, g_Config.Window.IsWindowResizable);
	}

	GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();

	if (!pMonitor)
	{
		glfwTerminate();
		std::cout << "Failed to create GLFW Monitor" << std::endl;
		return nullptr;
	}

	if (pMonitor)
	{
		const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);

		if (!pMode)
		{
			glfwTerminate();
			std::cout << "Failed to obtain GLFW Video Mode" << std::endl;
			return nullptr;
		}

		if (pMode)
		{
			// Create a new window
			if (g_Config.Window.IsWindowMode)
			{
				pWindow = glfwCreateWindow(pMode->width, pMode->height, "Mihai FFT Ocean", nullptr, nullptr); //Windowed																		 
			}
			else
			{
				pWindow = glfwCreateWindow(pMode->width, pMode->height, "Mihai FFT Ocean", pMonitor, nullptr); //Full Screen

				g_Config.Window.IsWindowResizable = false; // NOTE! In FullScreen Mode one cannot resize the window!
			}
		}
	}

	if (!pWindow)
	{
		glfwTerminate();
		std::cout << "Failed to create GLFW Window" << std::endl;
		return nullptr;
	}

	// Set the specified window
	glfwMakeContextCurrent(pWindow);

	// Get initial width and height of the specified window
	glfwGetFramebufferSize(pWindow, &g_WindowWidth, &g_WindowHeight);

	// Other setup
//	glfwSetInputMode(pWindow, GLFW_STICKY_KEYS, GL_FALSE);// TRUE);

	// hide the cursor
	glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//set the cursor position to be the center of the screen
	glfwSetCursorPos(pWindow, g_WindowWidth * 0.5f, g_WindowHeight * 0.5f);

	/////////// GLFW callbacks /////////////

	if (g_Config.Window.UseWindowHints && g_Config.Window.IsWindowResizable)
	{
		// Resize window callback
		glfwSetFramebufferSizeCallback(pWindow, ResizeWindowCB);
	}

	// Set keyboard callbacks
	glfwSetKeyCallback(pWindow, KeyCB);

	// Set mouse callbacks
	glfwSetMouseButtonCallback(pWindow, ButtonCB);
	glfwSetCursorPosCallback(pWindow, CursorPositionCB);
	glfwSetScrollCallback(pWindow, ScrollCB);

	//////// Init GLEW Context - OpenGL loader ///////////
	if (g_Config.OpenGLContext.IsCoreProfile)
	{
		glewExperimental = GL_TRUE; // Needed for core profile - is defined in GLEW, glew bug!
	}
	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW: " << glewGetErrorString(error) << std::endl;
		return nullptr;
	}

	///////////// GENERAL INFO ////////////

	// Print GLEW context information
	std::cout << "///////////////// OpenGL/GLEW context info ////////////////" << std::endl;
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	int maxTexUnits = 0;
	//glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexUnits);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTexUnits);
	std::cout << "Max Texture Units: " << maxTexUnits << std::endl;

	int maxImageUnits = 0;
	glGetIntegerv(GL_MAX_IMAGE_UNITS, &maxImageUnits);
	std::cout << "Max Image Units: " << maxImageUnits << std::endl;

	//// http://stackoverflow.com/questions/29707968/get-maximum-number-of-framebuffer-color-attachments
	int maxColorAtts = 0;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAtts);
	std::cout << "Max Color Attachments: " << maxColorAtts << std::endl;

	int maxTextureArrayLayers = 0;
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTextureArrayLayers);
	std::cout << "Max Texture Array Layers: " << maxTextureArrayLayers << std::endl;

	int maxDrawBuf = 0;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuf);
	std::cout << "Max Draw Buffers: " << maxDrawBuf << std::endl;

	int maxVertexAttribs = 0;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
	std::cout << "Max Vertex Attributes: " << maxVertexAttribs << std::endl;

	int maxTexCoords = 0;
	glGetIntegerv(GL_MAX_TEXTURE_COORDS, &maxTexCoords);
	std::cout << "Max Texture Coordinates: " << maxTexCoords << std::endl;

	int maxVertexUniforms = 0;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertexUniforms);
	std::cout << "Max Vertex Uniforms: " << maxVertexUniforms << std::endl;

	int maxFragmentUniforms = 0;
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxFragmentUniforms);
	std::cout << "Max Fragment Uniforms: " << maxFragmentUniforms << std::endl;

	int maxFloatVaryings = 0;
	glGetIntegerv(GL_MAX_VARYING_FLOATS, &maxFloatVaryings);
	std::cout << "Max Float Varyings: " << maxFloatVaryings << std::endl;


	//////// TODO - ADD SUPPORT FOR EXTENSIONS AND ADD GUARDS FOR THOSE THAT ARE NEED FOR SOME FEATURES
	int numExtensions = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

	std::vector<std::string> supportedExtensions;
	supportedExtensions.resize(numExtensions);

	std::cout << "Supported Extensions:\n";
	for (short i = 0; i < numExtensions; ++i)
	{
		const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
		supportedExtensions[i] = extension;
	}

	//// NOTE! To diferentiate core feature support from extensions !
	// E.g. GL_ARB_geometry_shader4 would mean that geoemtry shaders are supported,
	// but only in opengl 4.2 core, if opengl 3.3 is used then if 
	// GL_EXT_geometry_shader4 is available geometry shaders can be used as an extension

	////
	if (glewGetExtension("GL_ARB_geometry_shader4"))
	{
		std::cout << "YES to Geometry Shaders!" << std::endl;
	}
	else
	{
		std::cout << "NO to Geometry Shaders!" << std::endl;

		// Fallback to world_space grid
		g_Config.Scene.Ocean.Grid.Type = CustomTypes::Ocean::GridType::GT_WORLD_SPACE;
	}

	int maxGeometryOutputVertices = 0;
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &maxGeometryOutputVertices);
	std::cout << "Max Geometry Output Vertices: " << maxGeometryOutputVertices << std::endl;

	int maxGeometryOutputComponents = 0;
	glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &maxGeometryOutputComponents);
	std::cout << "Max Geometry Output Components: " << maxGeometryOutputComponents << std::endl;

	if (glewGetExtension("GL_ARB_compute_shader"))
	{
		std::cout << "YES to Compute Shaders!" << std::endl;
	}
	else
	{
		std::cout << "NO to Compute Shaders!" << std::endl;
	}

	// See if we support the function DispatchComputeGroupSizeARB
	if (glewGetExtension("GL_ARB_compute_variable_group_size"))
	{
		std::cout << "YES to compute_variable_group_size!" << std::endl;
	}
	else
	{
		std::cout << "NO to compute_variable_group_size!" << std::endl;
	}

	int maxWorkGroupsCount[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxWorkGroupsCount[0]); //X
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxWorkGroupsCount[1]); //Y
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxWorkGroupsCount[2]); //Z

	std::cout << "Max dispatchable Work Groups: X: " << maxWorkGroupsCount[0] << std::endl;
	std::cout << "Max dispatchable Work Groups: Y: " << maxWorkGroupsCount[1] << std::endl;
	std::cout << "Max dispatchable Work Groups: Z: " << maxWorkGroupsCount[2] << std::endl;

	int maxWorkGroupsSize[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxWorkGroupsSize[0]); //X
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxWorkGroupsSize[1]); //Y
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxWorkGroupsSize[2]); //Z

	std::cout << "Max Work Groups Size: X: " << maxWorkGroupsSize[0] << std::endl;
	std::cout << "Max Work Groups Size: Y: " << maxWorkGroupsSize[1] << std::endl;
	std::cout << "Max Work Groups Size: Z: " << maxWorkGroupsSize[2] << std::endl;

	int maxInvagationsWithinWorkGroup = 0;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxInvagationsWithinWorkGroup);
	std::cout << "Max Invovations with a work group: " << maxInvagationsWithinWorkGroup << std::endl;

	int maxSahredMemorySize = 0;
	glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &maxSahredMemorySize);
	std::cout << "Max Shared Storage Size: " << maxSahredMemorySize << std::endl;

	if (glewGetExtension("GL_EXT_texture_filter_anisotropic"))
	{
		std::cout << "YES to Anisotropic Filtering!" << std::endl;
	}
	else
	{
		std::cout << "NO to Anisotropic Filtering!" << std::endl;
	}

	if (glewGetExtension("GL_EXT_texture_compression_s3tc"))
	{
		std::cout << "YES to Compressed DDS textures!" << std::endl;
	}
	else
	{
		std::cout << "NO to Compressed DDS textures!" << std::endl;
	}


	std::cout << "//////////////////////////////////////////////////////" << std::endl;
	///////////////////////////////

	SetupGL();

	////////////// Init AntTweakBar - GUI //////////
	int ret = 0;

	if (g_Config.OpenGLContext.IsCoreProfile)
	{
		ret = TwInit(TW_OPENGL_CORE, nullptr);
	}
	else
	{
		ret = TwInit(TW_OPENGL, nullptr);
	}

	if (!ret)
	{
		std::cout << "Failed to initialize AntTweakBar!" << std::endl;
	}

	/// set the windows size
	ret = TwWindowSize(g_WindowWidth, g_WindowHeight);
	assert(ret != 0);

	//// init the bar
	g_pGUIBar = TwNewBar("Parameters");
	assert(g_pGUIBar != nullptr);
	ret = TwDefine("Parameters size='350 400'");
	assert(ret != 0);

	return pWindow;
}

float CalcFPS ( GLFWwindow* i_pWindow, float i_TimeInterval = 1.0f, std::string io_WindowTitle = "NONE" )
{
	// Static values which only get initialised the first time the function runs
	static float startTime = static_cast<float>(glfwGetTime()); // Set the initial time to now
	static float fps = 0.0f;           // Set the initial FPS value to 0.0

	// Set the initial frame count to -1.0 (it gets set to 0.0 on the next line). Because
	// we don't have a start time we simply cannot get an accurate FPS value on our very
	// first read if the time interval is zero, so we'll settle for an FPS value of zero instead.
	static float frameCount = -1.0f;

	// Here again? Increment the frame count
	frameCount ++;

	// Ensure the time interval between FPS checks is sane (low cap = 0.0 i.e. every frame, high cap = 10.0s)
	if (i_TimeInterval < 0.0f)
	{
		i_TimeInterval = 0.0f;
	}
	else if (i_TimeInterval > 10.0f)
	{
		i_TimeInterval = 10.0f;
	}

	// Get the duration in seconds since the last FPS reporting interval elapsed
	// as the current time minus the interval start time
	float duration = static_cast<float>(glfwGetTime()) - startTime;

	// If the time interval has elapsed...
	if (duration > i_TimeInterval)
	{
		// Calculate the FPS as the number of frames divided by the duration in seconds
		fps = frameCount / duration;

		// If the user specified a window title to append the FPS value to...
		if (io_WindowTitle != "NONE")
		{
			// Convert the fps value into a string using an output stringstream
			std::ostringstream stream;
			stream << " | FPS: " << fps << " | Frame Time: " << duration;

			// Append the FPS value amd Frame Time to the window title details
			io_WindowTitle += stream.str();

			// Convert the new window title to a c_str and set it
			const char* pszConstString = io_WindowTitle.c_str();
			glfwSetWindowTitle(i_pWindow, pszConstString);
		}
		else // If the user didn't specify a window to append the FPS to then output the FPS to the console
		{
			std::cout << "FPS: " << fps << std::endl;
		}

		// Reset the frame count to zero and set the initial time to be now
		frameCount = 0.0;
		startTime = static_cast<float>(glfwGetTime());
	}

	// Return the current FPS - doesn't have to be used if you don't want it!
	return fps;
}

void Run ( GLFWwindow* i_pWindow )
{
	ERROR_CHECK_START
	InitScene();
	ERROR_CHECK_END

	while (!glfwWindowShouldClose(i_pWindow))
	{

#if defined(CHECK_TIME_IN_UPDATE)
clock_t begin = clock();
#endif // CHECK_TIME_IN_UPDATE

		// Calculate deltatime of current frame
		float crrTime = static_cast<float>(glfwGetTime());
		static float oldTime = 0.0f;
		float deltaTime = crrTime - oldTime;
		oldTime = crrTime;

		crrTime *= g_TimeScale;

		////////////////////////////

		UpdatePPE(crrTime, deltaTime);

		UpdateCameraInput(i_pWindow, deltaTime);
		UpdateMotorBoatInput(crrTime, deltaTime);

	ERROR_CHECK_START
		// NOTE! Reflected stuff is only above water
		if (g_Config.VisualEffects.ShowReflections && g_pCurrentViewingCamera && g_pCurrentViewingCamera->GetAltitude() > 0.0f)
		{
			UpdateReflectedScene(crrTime, deltaTime);
			RenderReflectedScene();
		}

		// NOTE! Refracted stuff is only under water
		if (g_Config.VisualEffects.ShowRefractions && g_pCurrentViewingCamera && g_pCurrentViewingCamera->GetAltitude() < 0.0f)
		{
			UpdateRefractedScene(crrTime, deltaTime);
			RenderRefractedScene();
		}

		UpdateScene(crrTime, deltaTime);
		RenderScene();

		RenderPPE();
	ERROR_CHECK_END

		CalcFPS(i_pWindow, 1.0f, "My FFT Ocean + Scattering Sky");

		if (g_IsGUIVisible)
		{
			//Draw AntTweakBar bars
			int ret = TwDraw(); // returneaza 'Invalid Value' ca eroare OpenGL, dar e ok!
			//assert(ret != 0);
		}

		// Swap front buffer with back buffer of the specified window
		glfwSwapBuffers(i_pWindow);

		// Process all pending events
		glfwPollEvents();

#if defined(CHECK_TIME_IN_UPDATE)
clock_t end = clock();
double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
std::cout << "time spent: " << time_spent << std::endl;
#endif // CHECK_TIME_IN_UPDATE

	}

	TerminateScene();
}

void TerminateGLContext ( GLFWwindow* i_pWindow )
{
	// Destroy specified window
	glfwDestroyWindow(i_pWindow);
	i_pWindow = nullptr;

	// Terminate AntTweakBar
	int ret = TwTerminate();
	assert(ret != 0);

	// Terminate GLFW context
	glfwTerminate();
}

int main ( int argc, char **argv )
{
	g_Config.Initialize("../resources/GlobalConfig.xml");

	SetupGlobals();

	///////////
	GLFWwindow* pWindow = nullptr;

	pWindow = InitGLContext();
	if (!pWindow)
	{
		return -1;
	}

	Run(pWindow);

	TerminateGLContext(pWindow);

	return 0;
}