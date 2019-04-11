/* Author: BAIRAC MIHAI

 PROJECT ENTRY POINT

 The project uses the following libs:

 For OpenGL/ES Loading
 GLAD: https://github.com/Dav1dde/glad
 License: 

 For GL Context, Window and Input Management
 SDL2: https://www.libsdl.org/
 License: https://www.libsdl.org/license.php

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


#ifdef _DEBUG
//#include "vld.h" //// Visual Leak Detector - really helps !!!
#endif // DEBUG

#include "SDLConfig.h"
#include "SDL/SDL.h"
#include "GLConfig.h"
#include "CommonHeaders.h"
#include <new>
#include <iostream> //test
#include <sstream>  //fps

#include "GlobalConfig.h"
GlobalConfig g_Config;

#include "Application.h"
Application* g_pApplication = nullptr;

// Quit app
bool g_quit = false;



void UpdateContinuousInput(SDL_Window* i_pWindow )
{
	////////// Keyboard Update
	// we use the state, because we want to support multiple pressed keys at the same time
	const Uint8 *state = SDL_GetKeyboardState(NULL);

	if (!state)
	{
		ERR("Invalid keyboard state: %s!", state);
		return;
	}

	if (state && g_pApplication)
	{
		if (state[SDL_SCANCODE_W])
		{
			g_pApplication->CameraMoveForward();
			g_pApplication->BoatAccelerate();
		}

		if (state[SDL_SCANCODE_S])
		{
			g_pApplication->CameraMoveBackward();
			g_pApplication->BoatDecelerate();
		}

		if (state[SDL_SCANCODE_D])
		{
			g_pApplication->CameraMoveRight();
			g_pApplication->BoatTurnRight();
		}

		if (state[SDL_SCANCODE_A])
		{
			g_pApplication->CameraMoveLeft();
			g_pApplication->BoatTurnLeft();
		}

		if (state[SDL_SCANCODE_U])
		{
			g_pApplication->CameraMoveUp();
		}

		if (state[SDL_SCANCODE_B])
		{
			g_pApplication->CameraMoveDown();
		}

		//////// Mouse Update
		if (!g_pApplication->GetIsCursorReleased())
		{
			// the SDL_GetRelativeMouseState returns the delta values for each axis based on the difference between
			// the center of the screen and the current value in the direction of the respective axis be it OX or OY
			int dX, dY;
			SDL_GetRelativeMouseState(&dX, &dY);

			g_pApplication->UpdateCameraMouseOrientation(dX, dY);
		}
	}
}

//////////////////////////////////////////////

void ProcessKeyEvents (const SDL_Event& event)
{
	if (event.type == SDL_KEYDOWN)
	{
		switch (event.key.keysym.sym)
		{
		case SDLK_ESCAPE:
			g_quit = true;
			break;
		case SDLK_LSHIFT:
			if (g_pApplication)
			{
				g_pApplication->SetKeySpeed(g_pApplication->GetKeySpeed() * 10.0f);
			}
			break;
		}
	}
	else if (event.type == SDL_KEYUP)
	{
		switch (event.key.keysym.sym)
		{
		case SDLK_LSHIFT:
			if (g_pApplication)
			{
				g_pApplication->SetKeySpeed(g_pApplication->GetKeySpeed() / 10.0f);
			}
			break;
			//// OTHER INPUTS - I treat these here, because I need this to happen once(when the event is triggered), not in update
		case SDLK_BACKQUOTE: //"`"
			if (g_pApplication)
			{
				g_pApplication->SetIsCursorReleased(!g_pApplication->GetIsCursorReleased());
				if (g_pApplication->GetIsCursorReleased())
				{
					// show the cursor
					SDL_SetRelativeMouseMode(SDL_FALSE);
				}
				else
				{
					g_pApplication->SetIsInDraggingMode(false);

					// hide the cursor + grabs it providing unlimited 360 cursor movement (usefull for camera)
					SDL_SetRelativeMouseMode(SDL_TRUE);
				}
			}
			break;
		case SDLK_1:
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
		case SDLK_2:
			if (g_pApplication)
			{
				g_pApplication->SetIsRenderPoints(!g_pApplication->GetIsRenderPoints());
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
		case SDLK_3:
			if (g_pApplication)
			{
				g_pApplication->SetIsInBoatMode(!g_pApplication->GetIsInBoatMode());
			}
			break;
		case SDLK_4:
			if (g_pApplication)
			{
				g_pApplication->SetIsGUIVisible(!g_pApplication->GetIsGUIVisible());
			}
			break;
		case SDLK_c:
			if (g_pApplication)
			{
				g_pApplication->SwitchViewBetweenCameras();
			}
			break;
		case SDLK_v:
			if (g_pApplication)
			{
				g_pApplication->SwitchControlBetweenCameras();
			}
			break;
		case SDLK_f:
			if (g_pApplication)
			{
				g_pApplication->SetIsFrustumVisible(!g_pApplication->GetIsFrustumVisible());
			}
			break;
		case SDLK_r:
			if (g_pApplication)
			{
				g_pApplication->ResetFOV();
			}
			break;
		}
	}
}

void ProcessMouseButtonEvents(const SDL_Event& event)
{
	if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
	{
		if (g_pApplication)
		{
			g_pApplication->SetIsInDraggingMode(false);

			if (event.button.button == SDL_BUTTON_LEFT)
			{
				if (g_pApplication->GetIsGUIVisible())
				{
					if (!g_pApplication->OnGUIMouseEventSDL(event, SDL_MAJOR_VERSION, SDL_MINOR_VERSION))
					{
						if (event.type == SDL_MOUSEBUTTONDOWN)
						{
							g_pApplication->SetIsInDraggingMode(true);
						}
					}
				}
				else
				{
					if (event.type == SDL_MOUSEBUTTONDOWN)
					{
						g_pApplication->SetIsInDraggingMode(true);
					}
				}
			}
		}
	}
}

void ProcessMouseMotionEvents ( const SDL_Event& event )
{
	if (event.type == SDL_MOUSEMOTION)
	{
		if (g_pApplication)
		{
			g_pApplication->OnMouseMotion(event.motion.x, event.motion.y);
		}
	}
}

void ProcessMouseScrollEvents (const SDL_Event& event)
{
	if (event.type == SDL_MOUSEWHEEL)
	{
		if (g_pApplication)
		{
			g_pApplication->OnMouseScroll(event.wheel.x, event.wheel.y);
		}
	}
}

void ProcessResizeWindowEvents (const SDL_Event& event)
{
	if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
	{
		if (g_pApplication)
		{
			g_pApplication->OnWindowResize(event.window.data1, event.window.data2);
		}
	}
}


SDL_Window* InitGLContext(void)
{
	//////////////// Init GL Context - one OpenGL context + one window /////////////////

	// Init SDL2 video mode
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		ERR("Failed to initialize SDL2!");
		return nullptr;
	}

	// NOTE! The application may run faster without windows hints !!!
	// !!!  IMPORTANT !!! On other PCs or nVidia cards the app may not work with this activated !!!
	if (g_Config.Window.UseWindowHints)
	{
		//// Window hints - these hints work only for OpenGL 3.2 and above !!!
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, g_Config.OpenGLContext.OpenGLVersion.major);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, g_Config.OpenGLContext.OpenGLVersion.minor);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
		////

		//// OpenGL profiles only work for OpenGL 3.2 and above !!!
		if (g_Config.OpenGLContext.OpenGLVersion.major >= 3 && g_Config.OpenGLContext.OpenGLVersion.minor >= 2)
		{	
			if (g_Config.OpenGLContext.IsCoreProfile)
			{
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_CORE);
			}
			else
			{
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
			}
		}

		SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
	}

	///////////////
	LOG("/////// GL Context Specs ///////////");
	int attrVal = 0;
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &attrVal);
	LOG("DoubleBuffering: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_ACCELERATED_VISUAL, &attrVal);
	LOG("AcceleratedVisual: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE, &attrVal);
	LOG("BufferSize: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &attrVal);
	LOG("DepthSize: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &attrVal);
	LOG("StencilSize: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, &attrVal);
	LOG("FramebufferSRGBCapable: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &attrVal);
	LOG("RedSize: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &attrVal);
	LOG("GreenSize: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &attrVal);
	LOG("BlueSize: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &attrVal);
	LOG("AlphaSize: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &attrVal);
	LOG("MultisampleBuffers: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &attrVal);
	LOG("MultisampleSamples: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &attrVal);
	LOG("GLContextMajorVersion: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &attrVal);
	LOG("GLContextMinorrVersion: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &attrVal);
	LOG("GLContextFlags: %d", attrVal);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &attrVal);
	LOG("GLContextProfileMask: %d", attrVal);

	LOG("/////// GL Context Specs ///////////");

	SDL_DisplayMode currentDisplayMode;
	if (SDL_GetCurrentDisplayMode(0, &currentDisplayMode) != 0)
	{
		ERR("Failed get the current display mode!");
		SDL_Quit();
		return nullptr;
	}

	// Create a new window
	SDL_Window* pWindow = nullptr;

	if (g_Config.Window.IsWindowMode)
	{
		if (g_Config.Window.IsWindowResizable)
		{
			pWindow = SDL_CreateWindow("Mihai FFT Ocean", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, currentDisplayMode.w, currentDisplayMode.h - 50, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		}
		else
		{
			pWindow = SDL_CreateWindow("Mihai FFT Ocean", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, currentDisplayMode.w, currentDisplayMode.h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		}
	}
	else
	{
		pWindow = SDL_CreateWindow("Mihai FFT Ocean", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, currentDisplayMode.w, currentDisplayMode.h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);

		g_Config.Window.IsWindowResizable = false; // NOTE! In FullScreen Mode one cannot resize the window!
	}

	if (!pWindow)
	{
		ERR("Failed to create SDL2 Window!");
		SDL_Quit();
		return nullptr;
	}

	SDL_SetWindowResizable(pWindow, (SDL_bool)g_Config.Window.IsWindowResizable);

	SDL_GLContext glContext = SDL_GL_CreateContext(pWindow);
	if (!glContext)
	{
		ERR("Failed to create OpenGL context!");
		SDL_Quit();
		return nullptr;
	}


	// Set the specified gl contex associated with the window
	SDL_GL_MakeCurrent(pWindow, glContext);

	// Other setup

	// Get initial width and height of the specified window
	int windowWidth = 0, windowHeight = 0;
	SDL_GL_GetDrawableSize(pWindow, &windowWidth, &windowHeight);

	///// Input setup //////

	// setting the mosue to relative mode we perform the following:
	// 1) hide the cursor if the relative mode is true, show it otherwise
	// 2) compute the mouse delta position on OX & OY based on
	// the difference between the center of the screen and each coordinate value on its respective axis
	SDL_SetRelativeMouseMode(SDL_TRUE);

	///////// INIT GL LOADER //////
	// Load GL functions & extension via SDL proc
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		ERR("Failed to initialize GLAD OpenGL Loader !");
		SDL_Quit();
		return nullptr;
	}

	LOG("Loaded GL Version: %d %d", GLVersion.major, GLVersion.minor);


	/////// Init extension info
	g_Config.GLExtVars.Initialize();

	///////////// GENERAL INFO ////////////

	// Print OpenGL Renderer context information
	LOG("///////////////// OpenGL Renderer Info ////////////////\n");
	LOG("Vendor: %s", glGetString(GL_VENDOR));
	LOG("Renderer: %s", glGetString(GL_RENDERER));
	LOG("Version: %s", glGetString(GL_VERSION));
	LOG("GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	int numExtensions = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

	LOG("Supported OpenGL Extensions:");
	for (short i = 0; i < numExtensions; ++i)
	{
		const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
		LOG(extension);
	}
	LOG("Required extensions: \n%s", g_Config.GLExtVars.RequiredGLExtensions);

	/////////////////////////////
	int maxTexUnits = 0;
	//glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTexUnits);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTexUnits);
	LOG("Max Texture Units: %d", maxTexUnits);
	
	//// http://stackoverflow.com/questions/29707968/get-maximum-number-of-framebuffer-color-attachments
	int maxColorAtts = 0;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAtts);
	LOG("Max Color Attachments: %d", maxColorAtts);

	int maxTextureArrayLayers = 0;
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTextureArrayLayers);
	LOG("Max Texture Array Layers: %d", maxTextureArrayLayers);

	int maxDrawBuf = 0;
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuf);
	LOG("Max Draw Buffers: %d", maxDrawBuf);

	int maxVertexAttribs = 0;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
	LOG("Max Vertex Attributes: %d", maxVertexAttribs);
	
	int maxVertexOutputs = 0;
	glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &maxVertexOutputs);
	LOG("Max Vertex Outputs: %d", maxVertexOutputs);

	int maxVertexUniforms = 0;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertexUniforms);
	LOG("Max Vertex Uniforms: %d", maxVertexUniforms);

	int maxFragmentInputs = 0;
	glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &maxFragmentInputs);
	LOG("Max Fragment Inputs: %d", maxFragmentInputs);

	int maxFragmentUniforms = 0;
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxFragmentUniforms);
	LOG("Max Fragment Uniforms: %d", maxFragmentUniforms);

	int maxFloatVaryings = 0;
	glGetIntegerv(GL_MAX_VARYING_FLOATS, &maxFloatVaryings);
	LOG("Max Float Varyings: %d", maxFloatVaryings);

	//// NOTE! To differentiate core feature support from extensions !
	// E.g. GL_ARB_geometry_shader4 would mean that geometry shaders are supported,
	// but only in opengl 4.2 core, if opengl 3.3 is used then if 
	// GL_EXT_geometry_shader4 is available geometry shaders can be used as an extension

	////
	if (g_Config.GLExtVars.IsGeometryShaderSupported)
	{
		LOG("YES to Geometry Shaders!");
		
		int maxGeometryOutputVertices = 0;
		glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &maxGeometryOutputVertices);
		LOG("Max Geometry Output Vertices: %d", maxGeometryOutputVertices);

		int maxGeometryInputComponents = 0;
		glGetIntegerv(GL_MAX_GEOMETRY_INPUT_COMPONENTS, &maxGeometryInputComponents);
		LOG("Max Geometry Input Components: %d", maxGeometryInputComponents);

		int maxGeometryOutputComponents = 0;
		glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS, &maxGeometryOutputComponents);
		LOG("Max Geometry Output Components: %d", maxGeometryOutputComponents);

		int maxGeometryTotalOutputComponents = 0;
		glGetIntegerv(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS, &maxGeometryTotalOutputComponents);
		LOG("Max Geometry Total Output Components: %d", maxGeometryTotalOutputComponents);
			
		int maxGeometryUniformComponents = 0;
		glGetIntegerv(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS, &maxGeometryUniformComponents);
		LOG("Max Geometry Uniform Components: %d", maxGeometryUniformComponents);
	}
	else
	{
		LOG("NO to Geometry Shaders!");

		// Fallback to world_space grid
		g_Config.Scene.Ocean.Grid.Type = CustomTypes::Ocean::GridType::GT_WORLD_SPACE;
	}

	if (g_Config.GLExtVars.IsComputeShaderSupported)
	{
		LOG("YES to Compute Shaders!");
		
		int maxWorkGroupsCount[3];
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxWorkGroupsCount[0]); //X
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxWorkGroupsCount[1]); //Y
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxWorkGroupsCount[2]); //Z

		LOG("Max dispatchable Work Groups: X: %d", maxWorkGroupsCount[0]);
		LOG("Max dispatchable Work Groups: Y: %d", maxWorkGroupsCount[1]);
		LOG("Max dispatchable Work Groups: Z: %d", maxWorkGroupsCount[2]);

		int maxWorkGroupsSize[3];
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxWorkGroupsSize[0]); //X
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &maxWorkGroupsSize[1]); //Y
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &maxWorkGroupsSize[2]); //Z

		LOG("Max Work Groups Size: X: %d", maxWorkGroupsSize[0]);
		LOG("Max Work Groups Size: Y: %d", maxWorkGroupsSize[1]);
		LOG("Max Work Groups Size: Z: %d", maxWorkGroupsSize[2]);

		int maxInvocationsWithinWorkGroup = 0;
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxInvocationsWithinWorkGroup);
		LOG("Max Invocations with a work group: %d", maxInvocationsWithinWorkGroup);

		int maxSharedMemorySize = 0;
		glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &maxSharedMemorySize);
		LOG("Max Shared Storage Size: %d", maxSharedMemorySize);
		
		int maxImageUnits = 0;
		glGetIntegerv(GL_MAX_IMAGE_UNITS, &maxImageUnits);
		LOG("Max Image Units: %d", maxImageUnits);
	}
	else
	{
		LOG("NO to Compute Shaders!");
	}

	if (g_Config.GLExtVars.IsTexAnisoFilterSupported)
	{
		LOG("YES to Anisotropic Filtering!");
	}
	else
	{
		LOG("NO to Anisotropic Filtering!");
	}

	if (g_Config.GLExtVars.IsTexDDSSupported)
	{
		LOG("YES to Compressed DDS textures!");
	}
	else
	{
		LOG("NO to Compressed DDS textures!");
	}


	LOG("//////////////////////////////////////////////////////\n");
	///////////////////////////////

	return pWindow;
}

float CalcFPS ( SDL_Window* i_pWindow, float i_TimeInterval = 1.0f, std::string io_WindowTitle = "NONE" )
{
	// Static values which only get initialised the first time the function runs
	static float startTime = static_cast<float>(SDL_GetTicks() / 1000.0f); // Set the initial time to now
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
	float duration = static_cast<float>(SDL_GetTicks() / 1000.0f) - startTime;

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
			SDL_SetWindowTitle(i_pWindow, pszConstString);
		}
		else // If the user didn't specify a window to append the FPS to then output the FPS to the console
		{
			std::cout << "FPS: " << fps << std::endl;
		}

		// Reset the frame count to zero and set the initial time to be now
		frameCount = 0.0;
		startTime = static_cast<float>(SDL_GetTicks() / 1000.0f);
	}

	// Return the current FPS - doesn't have to be used if you don't want it!
	return fps;
}

void Run ( SDL_Window* i_pWindow )
{
	int windowWidth = 0, windowHeight = 0;
	SDL_GL_GetDrawableSize(i_pWindow, &windowWidth, &windowHeight);

GL_ERROR_CHECK_START
	g_pApplication = new Application(g_Config, windowWidth, windowHeight);
	assert(g_pApplication != nullptr);
GL_ERROR_CHECK_END

	// TODO - fix SDL_GetTicks() division by 1000
	while (!g_quit)
	{
		// Calculate deltatime of current frame
		float crrTime = static_cast<float>(SDL_GetTicks() / 1000.0f);
		static float oldTime = 0.0f;
		float deltaTime = crrTime - oldTime;
		oldTime = crrTime;

		crrTime *= g_Config.Simulation.TimeScale;

GL_ERROR_CHECK_START

		////////////////////////////
		if (g_pApplication)
		{
			g_pApplication->Update(crrTime, deltaTime, g_Config);
			g_pApplication->Render(g_Config);
		}

GL_ERROR_CHECK_END

		CalcFPS(i_pWindow, 1.0f, "My FFT Ocean + Scattering Sky");

		// Swap front buffer with back buffer of the specified window
		SDL_GL_SwapWindow(i_pWindow);

		// split this function into key & mouse handling
		UpdateContinuousInput(i_pWindow);

		// Process all pending events
		SDL_Event ev;
		while (SDL_PollEvent(&ev) != 0)
		{
			//User requests quit
			if (ev.type == SDL_QUIT)
			{
				g_quit = true;
			}

			if (g_Config.Window.IsWindowResizable)
			{
				ProcessResizeWindowEvents(ev);
			}

			if (SDL_GetKeyboardState(nullptr)) // keyboard is supported
			{
				ProcessKeyEvents(ev);
			}

			// TODO - fix
		//	if (SDL_GetMouseState(nullptr, nullptr)) // mouse is supported
			{
				ProcessMouseButtonEvents(ev);
				ProcessMouseMotionEvents(ev);
				ProcessMouseScrollEvents(ev);
			}
		}
	}
	
	SAFE_DELETE(g_pApplication);
}

void TerminateGLContext ( SDL_Window* i_pWindow )
{
	// Destroy specified window
	SDL_DestroyWindow(i_pWindow);
	i_pWindow = nullptr;

	// Terminate GLFW context
	SDL_Quit();
}

int main ( int argc, char **argv )
{
	if (!g_Config.Initialize("../resources/GlobalConfig.xml"))
	{
		return -1;
	}

	///////////
	SDL_Window* pWindow = nullptr;

	pWindow = InitGLContext();
	if (!pWindow)
	{
		return -1;
	}

	Run(pWindow);

	TerminateGLContext(pWindow);

	return 0;
}