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

#include <new>
#include <iostream> //test
#include <sstream>  //fps
#include <ctime>

//#define ENABLE_ERROR_CHECK
#include "ErrorHandler.h"
#include "Common.h"

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



#include "GlobalConfig.h"
GlobalConfig g_Config;

#include "Application.h"
Application* g_pApplication = nullptr;



void UpdateInput ( GLFWwindow* i_pWindow )
{
	if (g_pApplication)
	{
		if (g_pApplication->IsKeyPressed(GLFW_KEY_W))
		{
			g_pApplication->CameraMoveForward();
			g_pApplication->BoatAccelerate();
		}

		if (g_pApplication->IsKeyPressed(GLFW_KEY_S))
		{
			g_pApplication->CameraMoveBackward();
			g_pApplication->BoatDecelerate();
		}

		if (g_pApplication->IsKeyPressed(GLFW_KEY_D))
		{
			g_pApplication->CameraMoveRight();
			g_pApplication->BoatTurnRight();
		}

		if (g_pApplication->IsKeyPressed(GLFW_KEY_A))
		{
			g_pApplication->CameraMoveLeft();
			g_pApplication->BoatTurnLeft();
		}

		if (g_pApplication->IsKeyPressed(GLFW_KEY_U))
		{
			g_pApplication->CameraMoveUp();
		}

		if (g_pApplication->IsKeyPressed(GLFW_KEY_B))
		{
			g_pApplication->CameraMoveDown();
		}

		if (!g_pApplication->GetIsCursorReleased())
		{
			// update mouse data
			double xPos = 0.0, yPos = 0.0;
			glfwGetCursorPos(i_pWindow, &xPos, &yPos);
					
			// reset the cursor position for next frame
			float halfWidth = g_pApplication->GetWindowWidth() * 0.5f, halfHeight = g_pApplication->GetWindowHeight() * 0.5f;
			glfwSetCursorPos(i_pWindow, halfWidth, halfHeight);

			g_pApplication->UpdateCameraMouseOrientation(xPos, yPos);
		}
	}
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
			if (g_pApplication)
			{
				g_pApplication->SetKeySpeed(g_pApplication->GetKeySpeed() * 10.0f);
			}
		}
		else if(i_Action == GLFW_RELEASE)
		{
			if (g_pApplication)
			{
				g_pApplication->SetKeySpeed(g_pApplication->GetKeySpeed() / 10.0f);
			}
		}
	}

	// this setup allows us to use multiple camera keys simultaneously!
	if (i_Action == GLFW_PRESS)
	{
		if (g_pApplication)
		{
			g_pApplication->SetKeyPresed(i_Key, true);
		}

	}
	else if (i_Action == GLFW_RELEASE)
	{
		if (g_pApplication)
		{
			g_pApplication->SetKeyPresed(i_Key, false);
		}
	}

	//// OTHER INPUTS - I treat these here, because I need this to happen once(when the event is triggered), not in update
	if (i_Action == GLFW_RELEASE)
	{
		// other controls
		switch (i_Key)
		{
		case GLFW_KEY_GRAVE_ACCENT: //"`"		
			if (g_pApplication)
			{
				g_pApplication->SetIsCursorReleased(! g_pApplication->GetIsCursorReleased());
				if (g_pApplication->GetIsCursorReleased())
				{
					// show the cursor
					glfwSetInputMode(i_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
				else
				{
					g_pApplication->SetIsInDraggingMode(false);

					// hide the cursor + grabs it providing unlimited 360 cursor movement (usefull for camera)
					glfwSetInputMode(i_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}
			}
			break;
		case GLFW_KEY_1:
			if (g_pApplication)
			{
				g_pApplication->SetIsRenderWireframe(!g_pApplication->GetIsRenderWireframe());
				if (g_pApplication->GetIsRenderWireframe())
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //not available in OpenGL ES
				}
				else
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
			}
			break;
		case GLFW_KEY_2:
			if (g_pApplication)
			{
				g_pApplication->SetIsRenderPoints(! g_pApplication->GetIsRenderPoints());
				if (g_pApplication->GetIsRenderPoints())
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
				}
				else
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
			}
			break;
		case GLFW_KEY_3:
			if (g_pApplication)
			{
				g_pApplication->SetIsInBoatMode(! g_pApplication->GetIsInBoatMode());
			}
			break;
		case GLFW_KEY_4:
			if (g_pApplication)
			{
				g_pApplication->SetIsGUIVisible(!g_pApplication->GetIsGUIVisible());
			}
			break;
		case GLFW_KEY_C:
			if (g_pApplication)
			{
				g_pApplication->SwitchViewBetweenCameras();
			}
			break;
		case GLFW_KEY_V:
			if (g_pApplication)
			{
				g_pApplication->SwitchControlBetweenCameras();
			}
			break;
		case GLFW_KEY_F:
			if (g_pApplication)
			{
				g_pApplication->SetIsFrustumVisible(! g_pApplication->GetIsFrustumVisible());
			}
			break;
		case GLFW_KEY_R:
			if (g_pApplication)
			{
				g_pApplication->ResetFOV();
			}
			break;
		}
	}
}

static void ButtonCB ( GLFWwindow* i_pWindow, int i_Button, int i_Action, int i_Mods )
{
	if (g_pApplication)
	{
		g_pApplication->SetIsInDraggingMode(false);

		if (i_Button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if (g_pApplication->GetIsGUIVisible())
			{
				if (! g_pApplication->OnGUIMouseEventGLFW(i_Button, i_Action))
				{
					if (GLFW_PRESS == i_Action)
					{
						g_pApplication->SetIsInDraggingMode(true);
					}
				}
			}
			else
			{
				if (GLFW_PRESS == i_Action)
				{
					g_pApplication->SetIsInDraggingMode(true);
				}
			}
		}
	}
}

static void CursorPositionCB ( GLFWwindow* i_pWindow, double i_XPos, double i_YPos )
{
	if (g_pApplication)
	{
		g_pApplication->OnCursorPosition(i_XPos, i_YPos);
	}
}

static void ScrollCB ( GLFWwindow* pWindow, double i_XOffset, double i_YOffset )
{
	if (g_pApplication)
	{
		g_pApplication->OnMouseScroll(i_XOffset, i_YOffset);
	}
}

static void ResizeWindowCB ( GLFWwindow* i_pWindow, int i_Width, int i_Height )
{
	if (g_pApplication)
	{
		g_pApplication->OnWindowResize(i_Width, i_Height);
	}
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

			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); /// MAC OS X fix !!!
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

	// Other setup
	//glfwSetInputMode(pWindow, GLFW_STICKY_KEYS, GL_FALSE);

	// hide the cursor
	glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Get initial width and height of the specified window
	int windowWidth = 0, windowHeight = 0;
	glfwGetFramebufferSize(pWindow, &windowWidth, &windowHeight);
	//set the cursor position to be the center of the screen
	glfwSetCursorPos(pWindow, windowWidth * 0.5f, windowHeight * 0.5f);

	/////////// GLFW callbacks /////////////

	if (/*g_Config.Window.UseWindowHints &&*/ g_Config.Window.IsWindowResizable)
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
	
	int maxVertexOutputs = 0;
	glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &maxVertexOutputs);
	std::cout << "Max Vertex Outputs: " << maxVertexOutputs << std::endl;

	int maxVertexUniforms = 0;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertexUniforms);
	std::cout << "Max Vertex Uniforms: " << maxVertexUniforms << std::endl;

	int maxFragmentInputs = 0;
	glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &maxFragmentInputs);
	std::cout << "Max Fragment Inputs: " << maxFragmentInputs << std::endl;

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
		
			int maxGeometryOutputVertices = 0;
			glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &maxGeometryOutputVertices);
			std::cout << "Max Geometry Output Vertices: " << maxGeometryOutputVertices << std::endl;

			int maxGeometryInputComponents = 0;
			glGetIntegerv(GL_MAX_GEOMETRY_INPUT_COMPONENTS, &maxGeometryInputComponents);
			std::cout << "Max Geometry Input Components: " << maxGeometryInputComponents << std::endl;

			int maxGeometryOutputComponents = 0;
			glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS, &maxGeometryOutputComponents);
			std::cout << "Max Geometry Output Components: " << maxGeometryOutputComponents << std::endl;

			int maxGeometryTotalOutputComponents = 0;
			glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &maxGeometryTotalOutputComponents);
			std::cout << "Max Geometry Total Output Components: " << maxGeometryTotalOutputComponents << std::endl;
			
			int maxGeometryUniformComponents = 0;
			glGetIntegerv(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS, &maxGeometryUniformComponents);
			std::cout << "Max Geometry Uniform Components" << maxGeometryUniformComponents << std::endl;
	}
	else
	{
		std::cout << "NO to Geometry Shaders!" << std::endl;

		// Fallback to world_space grid
		g_Config.Scene.Ocean.Grid.Type = CustomTypes::Ocean::GridType::GT_WORLD_SPACE;
	}

	if (glewGetExtension("GL_ARB_compute_shader"))
	{
		std::cout << "YES to Compute Shaders!" << std::endl;
		
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

		int maxInvocationsWithinWorkGroup = 0;
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxInvocationsWithinWorkGroup);
		std::cout << "Max Invocations with a work group: " << maxInvocationsWithinWorkGroup << std::endl;

		int maxSharedMemorySize = 0;
		glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &maxSharedMemorySize);
		std::cout << "Max Shared Storage Size: " << maxSharedMemorySize << std::endl;
		
		int maxImageUnits = 0;
		glGetIntegerv(GL_MAX_IMAGE_UNITS, &maxImageUnits);
		std::cout << "Max Image Units: " << maxImageUnits << std::endl;
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
	int windowWidth = 0, windowHeight = 0;
	glfwGetFramebufferSize(i_pWindow, &windowWidth, &windowHeight);

ERROR_CHECK_START
	g_pApplication = new Application(g_Config, windowWidth, windowHeight);
	assert(g_pApplication != nullptr);
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

		crrTime *= g_Config.Simulation.TimeScale;

ERROR_CHECK_START

		////////////////////////////
		if (g_pApplication)
		{
			g_pApplication->Update(crrTime, deltaTime, g_Config);
			g_pApplication->Render(g_Config);
		}

		UpdateInput(i_pWindow);

ERROR_CHECK_END

		CalcFPS(i_pWindow, 1.0f, "My FFT Ocean + Scattering Sky");

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
	
	SAFE_DELETE(g_pApplication);
}

void TerminateGLContext ( GLFWwindow* i_pWindow )
{
	// Destroy specified window
	glfwDestroyWindow(i_pWindow);
	i_pWindow = nullptr;

	// Terminate GLFW context
	glfwTerminate();
}

int main ( int argc, char **argv )
{
	g_Config.Initialize("../resources/GlobalConfig.xml");

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