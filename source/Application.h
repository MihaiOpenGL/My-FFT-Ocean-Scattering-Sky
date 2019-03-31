#ifndef APPLICATION_H
#define APPLICATION_H

#include "GUI/AntTweakBar.h"
#include "SDL/SDL_events.h"
#define GLM_SWIZZLE //offers the possibility to use: .xx(), xy(), xyz(), ...
#include "glm/vec3.hpp"


class GlobalConfig;
class Camera;
class FrameBufferManager;
class PostProcessingManager;
class Sky;
class Ocean;
class MotorBoat;

/*
	Main application
*/
class Application
{
public:
	Application();
	Application(const GlobalConfig& i_Config, int i_WindowWidth, int i_WindowHeight);
	~Application();

	void Update(float i_CrrTime, float i_DeltaTime, const GlobalConfig& i_Config);

	void Render(const GlobalConfig& i_Config);

	void SwitchViewBetweenCameras();
	void SwitchControlBetweenCameras();

	void ResetFOV();

	void OnWindowResize(int i_Width, int i_Height);

	bool OnGUIMouseEventSDL(const SDL_Event& sdlEvent, unsigned char sdlMajorVersion, unsigned char sdlMinorVersion);

	void OnMouseMotion(int i_DX, int i_DY);
	void OnMouseScroll(int i_XOffset, int i_YOffset);

	void UpdateCameraMouseOrientation(int i_XPos, int i_YPos);

	void CameraMoveForward();
	void CameraMoveBackward();
	void CameraMoveRight();
	void CameraMoveLeft();
	void CameraMoveUp();
	void CameraMoveDown();

	void BoatAccelerate();
	void BoatDecelerate();
	void BoatTurnLeft();
	void BoatTurnRight();

	int GetWindowWidth();
	int GetWindowHeight();

	float GetCrrTime() const;
	float GetDeltaTime() const;

	float GetKeySpeed() const;
	void SetKeySpeed(float i_Value);

	bool GetIsCursorReleased() const;
	void SetIsCursorReleased(bool i_Value);

	bool GetIsInDraggingMode() const;
	void SetIsInDraggingMode(bool i_Value);

	bool GetIsRenderWireframe() const;
	void SetIsRenderWireframe(bool i_Value);

	bool GetIsRenderPoints() const;
	void SetIsRenderPoints(bool i_Value);

	bool GetIsInBoatMode() const;
	void SetIsInBoatMode(bool i_Value);

	bool GetIsGUIVisible() const;
	void SetIsGUIVisible(bool i_Value);

	bool GetIsFrustumVisible() const;
	void SetIsFrustumVisible(bool i_Value);

	bool GetIsViewCameraChanged() const;
	void SetIsViewCameraChanged(bool i_Value);

	bool GetIsControlCameraChanged() const;
	void SetIsControlCameraChanged(bool i_Value);


private:
	void SetupGlobals (const GlobalConfig& i_Config);

	void SetupGL(const GlobalConfig& i_Config);

	void InitScene(const GlobalConfig& i_Config);
	void TerminateScene(void);

	static void TW_CALL GetCameraPosition(void *i_pValue, void *i_pClientData);
	static void TW_CALL GetCameraDirection(void *i_pValue, void *i_pClientData);
	static void TW_CALL GetEnabledClouds(void *i_pValue, void *i_pClientData);
	static void TW_CALL SetEnabledClouds(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetCloudsOctaves(void *i_pValue, void *i_pClientData);
	static void TW_CALL SetCloudsOctaves(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetCloudsLacunarity(void *i_pValue, void *i_pClientData);
	static void TW_CALL SetCloudsLacunarity(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetCloudsGain(void *i_pValue, void *i_pClientData);
	static void TW_CALL SetCloudsGain(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetCloudsScaleFactor(void *i_pValue, void *i_pClientData);
	static void TW_CALL SetCloudsScaleFactor(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetCloudsNorm(void *i_pValue, void *i_pClientData);
	static void TW_CALL SetCloudsNorm(const void* i_pValue, void* i_pClientData);
	static void TW_CALL SetOceanAmplitude(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetOceanAmplitude(void* i_pValue, void* i_pClientData);
	static void TW_CALL SetOceanPatchSize(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetOceanPatchSize(void* i_pValue, void* i_pClientData);
	static void TW_CALL SetOceanTileScale(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetOceanTileScale(void* i_pValue, void* i_pClientData);
	static void TW_CALL SetOceanWindSpeed(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetOceanWindSpeed(void* i_pValue, void* i_pClientData);
	static void TW_CALL SetOceanWindDirectionX(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetOceanWindDirectionX(void* i_pValue, void* i_pClientData);
	static void TW_CALL SetOceanWindDirectionZ(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetOceanWindDirectionZ(void* i_pValue, void* i_pClientData);
	static void TW_CALL GetOceanWindDir(void* i_pValue, void* i_pClientData);
	static void TW_CALL SetOceanOpposingWavesFactor(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetOceanOpposingWavesFactor(void* i_pValue, void* i_pClientData);
	static void TW_CALL SetOceanVerySmallWavesFactor(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetOceanVerySmallWavesFactor(void* i_pValue, void* i_pClientData);
	static void TW_CALL SetOceanChoppyScale(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetOceanChoppyScale(void* i_pValue, void* i_pClientData);
	static void TW_CALL SetFOV(const void* i_pValue, void* i_pClientData);
	static void TW_CALL GetFOV(void* i_pValue, void* i_pClientData);

	void SetWindowSize(int i_WindowWidth, int i_WindowHeight);

	void SetupDefaultFrameBuffer(const GlobalConfig& i_Config);
	void SetupAuxiliaryFrameBuffer(const GlobalConfig& i_Config);

	glm::vec3 HDR(glm::vec3 i_L, float i_E);

	void UpdateCameraMatrices();

	void UpdatePPE(float i_CrrTime, float i_DeltaTime, const GlobalConfig& i_Config);
	void UpdateReflectedScene(float i_CrrTime, float i_DeltaTime);
	void UpdateRefractedScene(float i_CrrTime, float i_DeltaTime);
	void UpdateScene(float i_CrrTime, float i_DeltaTime, const GlobalConfig& i_Config);

	void ComputeBuoyancy(float i_DeltaTime, bool i_IsBuyoancyEnabled);

	void RenderPPE(const GlobalConfig& i_Config);
	void RenderReflectedScene(void);
	void RenderRefractedScene(void);
	void RenderScene(const GlobalConfig& i_Config);
	void RenderGUI();

	// Variables
	int m_WindowWidth, m_WindowHeight;

	//////// Auxilliary FBO for reflected and refracted objects!
	FrameBufferManager* m_pFBM;
	////////

	// pointer to TwBar is required by the AntTweakBar library specs
	TwBar *m_pGUIBar;

	Camera *m_pCamera, *m_pObservingCamera, *m_pCurrentViewingCamera, *m_pCurrentControllingCamera;

	PostProcessingManager* m_pPostProcessingManager;
	Sky* m_pSky;
	Ocean* m_pOcean;
	MotorBoat* m_pMotorBoat;

	float m_TimeScale, m_CrrTime, m_DeltaTime;
	bool m_IsGUIVisible;

	bool m_IsCameraViewChanged, m_IsCameraControlChanged;
	bool m_IsRenderWireframe, m_IsRenderPoints;
	bool m_IsCursorReleased;

	bool m_IsFrustumVisible;

	bool m_IsUnderWater;

	bool m_IsInBoatMode;

	//// should be put inside InputManager
	int m_MouseOldXPos, m_MouseOldYPos;
	bool m_IsInDraggingMode;

	float m_KeySpeed;
	float m_MouseSpeed;
	////

	float m_SunPhi, m_SunTheta;

	// underwater default framebuffer color
	glm::vec3 m_UnderWaterColor;

	// default framebuffer color
	glm::vec3 m_DefaultColor;
};


#endif /* APPLICATION_H */