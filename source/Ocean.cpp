/* Author: BAIRAC MIHAI */

#include "Ocean.h"
#include "CommonHeaders.h"
#include "GLConfig.h"
// glm::vec2, glm::vec3, glm::mat4 come from the header
#include "glm/vec4.hpp"
#include "glm/common.hpp" //ceil()
#include <glm/gtc/matrix_transform.hpp> //glm::translate()
#include <glm/gtx/rotate_vector.hpp> //rotateY()
#include "glm/gtc/constants.hpp" //pi()
#include "glm/gtc/type_ptr.hpp" //value_ptr()
#include "glm/vector_relational.hpp" //any(), notEqual()
#include "HelperFunctions.h"
#include "FileUtils.h"
#include "PhysicsConstants.h"
#include "GlobalConfig.h"
#include "Camera.h"
#include "FFTOceanPatchGPUFrag.h"
#include "FFTOceanPatchGPUComp.h"
#include "FFTOceanPatchCPUFFTW.h"
#include "MotorBoat.h"
#include <sstream> // std::stringstream
#include <time.h>


Ocean::Ocean ( void )
	: m_pFFTOceanPatch(nullptr), m_pCurrentCamera(nullptr), 
	  m_GridVertexCount(0), m_GridIndexCount(0), m_ScreenSpaceGridResolution(0.0f),
	  m_OccluderIndexCount(0), m_IsWireframeMode(false), m_IsFrustumVisible(false),
	  m_GridType(CustomTypes::Ocean::GridType::GT_COUNT), m_SkyModelType(CustomTypes::Sky::ModelType::MT_COUNT),
	  m_SurfaceUseGridCorners(false), m_BottomUseGridCorners(false), m_EnableBottomCaustics(false),
	  m_EnableBoatKelvinWake(false), m_EnableBoatPropellerWash(false), m_EnableUnderWaterGodRays(false),
	  m_GodRaysMapWidth(0), m_GodRaysMapHeight(0), m_CausticsMapSize(0),
	  m_PerlinNoiseSpeed(0.0f), m_SunDirY(0.0f)
{
	LOG("Ocean successfully created!");
}

Ocean::Ocean ( const GlobalConfig& i_Config )
	: m_pFFTOceanPatch(nullptr), m_pCurrentCamera(nullptr),
	  m_GridVertexCount(0), m_GridIndexCount(0), m_ScreenSpaceGridResolution(0.0f),
	  m_OccluderIndexCount(0), m_IsWireframeMode(false), m_IsFrustumVisible(false),
	  m_GridType(CustomTypes::Ocean::GridType::GT_COUNT), m_SkyModelType(CustomTypes::Sky::ModelType::MT_COUNT),
	  m_SurfaceUseGridCorners(false), m_BottomUseGridCorners(false), m_EnableBottomCaustics(false),
	  m_EnableBoatKelvinWake(false), m_EnableBoatPropellerWash(false), m_EnableUnderWaterGodRays(false),
	  m_GodRaysMapWidth(0), m_GodRaysMapHeight(0), m_CausticsMapSize(0),
	  m_PerlinNoiseSpeed(0.0f), m_SunDirY(0.0f)
{
	Initialize(i_Config);
}

Ocean::~Ocean ( void )
{
	Destroy();
}

void Ocean::Destroy ( void )
{
	SAFE_DELETE(m_pFFTOceanPatch);
	LOG("Ocean successfully destroyed!");
}

void Ocean::Initialize ( const GlobalConfig& i_Config )
{
	m_GridType = i_Config.Scene.Ocean.Grid.Type;
	m_ScreenSpaceGridResolution = i_Config.Scene.Ocean.Grid.ScreenSpace.GridResolution;

	m_SkyModelType = i_Config.Scene.Sky.Model.Type;
	m_SurfaceUseGridCorners = i_Config.Scene.Ocean.Surface.Projector.UseGridCorners;
	m_BottomUseGridCorners = i_Config.Scene.Ocean.Bottom.Projector.UseGridCorners;
	m_EnableUnderWaterGodRays = i_Config.Scene.Ocean.UnderWater.GodRays.Enabled;
	m_EnableBottomCaustics = i_Config.Scene.Ocean.Bottom.Caustics.Enabled;
	m_EnableBoatKelvinWake = i_Config.Scene.Ocean.Surface.BoatEffects.KelvinWake.Enabled;
	m_EnableBoatPropellerWash = i_Config.Scene.Ocean.Surface.BoatEffects.PropellerWash.Enabled;

	m_UnderWaterGodRaysData.NumberOfSamples = 0;
	m_UnderWaterGodRaysData.Exposure = 0.0f;
	m_UnderWaterGodRaysData.Decay = 0.0f;
	m_UnderWaterGodRaysData.Density = 0.0f;
	m_UnderWaterGodRaysData.Weight = 0.0f;

	m_CausticsMapSize = i_Config.Scene.Ocean.Bottom.Caustics.MapSize;

	m_PerlinNoiseSpeed = i_Config.Scene.Ocean.Surface.PerlinNoise.Speed;

	///////////////////////////

	m_WaveProjector.Initialize(i_Config, Projector::PROJ_TYPE::PT_SURFACE);

	m_BottomProjector.Initialize(i_Config, Projector::PROJ_TYPE::PT_UNDERWATER);

	//////////////////////////////

	if (i_Config.Scene.Ocean.Surface.OceanPatch.ComputeFFT.Type == CustomTypes::Ocean::ComputeFFTType::CFT_GPU_FRAG)
	{
		m_pFFTOceanPatch = new FFTOceanPatchGPUFrag(i_Config);
	}
	else if (i_Config.Scene.Ocean.Surface.OceanPatch.ComputeFFT.Type == CustomTypes::Ocean::ComputeFFTType::CFT_GPU_COMP)
	{
		m_pFFTOceanPatch = new FFTOceanPatchGPUComp(i_Config);
	}
	else if (i_Config.Scene.Ocean.Surface.OceanPatch.ComputeFFT.Type == CustomTypes::Ocean::ComputeFFTType::CFT_CPU_FFTW)
	{
		m_pFFTOceanPatch = new FFTOceanPatchCPUFFTW(i_Config);
	}
	assert(m_pFFTOceanPatch != nullptr);

	///////////

	SetupGrid(i_Config);
	SetupOceanSurface(i_Config);

	SetupOceanBottom(i_Config);
	SetupOceanBottomCaustics(i_Config);
	SetupOceanBottomGodRays(i_Config);

	SetupDebugFrustum(i_Config);

	SetupTextures(i_Config);

	LOG("Ocean successfully created!");
}

void Ocean::SetupGrid ( const GlobalConfig& i_Config )
{
	std::vector<MeshBufferManager::VertexData> gridVertexData;
	std::vector<unsigned int> gridIndices;

	if (m_GridType == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
	{
		glm::ivec4 viewport;
		glGetIntegerv(GL_VIEWPORT, &viewport[0]);

		unsigned short screenWidth = viewport.z;
		unsigned short screenHeight = viewport.w;

		SetupScreenSpaceGrid(screenWidth, screenHeight);
	}
	else if (m_GridType == CustomTypes::Ocean::GridType::GT_WORLD_SPACE)
	{
		unsigned short i_GridWidth = i_Config.Scene.Ocean.Grid.WorldSpace.Width;
		unsigned short i_GridHeight = i_Config.Scene.Ocean.Grid.WorldSpace.Height;

		std::vector<MeshBufferManager::VertexData> gridVertexData;
		std::vector<unsigned int> gridIndices;

		gridVertexData.resize(i_GridWidth * i_GridHeight);

		float du = 1.0f / static_cast<float>(i_GridWidth - 1),
			dv = 1.0f / static_cast<float>(i_GridHeight - 1),
			u = 0.0f, v = 0.0f;
		unsigned int index = 0;
		for (unsigned short i = 0; i < i_GridHeight; ++i)
		{
			u = 0.0f;
			for (unsigned short j = 0; j < i_GridWidth; ++j)
			{
				index = i * i_GridWidth + j;

				gridVertexData[index].uv.x = u;
				gridVertexData[index].uv.y = v;

				u += du;
			}
			v += dv;
		}

		/////////// Indices //////////////
		gridIndices.resize(2 * i_GridWidth * (i_GridHeight - 1));

		for (unsigned short i = 0; i < i_GridHeight - 1; ++i)
		{
			for (unsigned short j = 0; j < i_GridWidth; ++j)
			{
				index = (j + i * i_GridWidth) * 2;
				if (i % 2 == 0)
				{
					gridIndices[index] = j + (i + 1) * i_GridWidth;
					gridIndices[index + 1] = j + i * i_GridWidth;
				}
				else
				{
					gridIndices[index] = i_GridWidth - 1 - j + i * i_GridWidth;
					gridIndices[index + 1] = i_GridWidth - 1 - j + (i + 1) * i_GridWidth;
				}
			}
		}

		///////////////////////////////////

		m_GridVertexCount = gridVertexData.size();
		m_GridIndexCount = gridIndices.size();


		//////////
		m_GridMBM.Initialize("Ocean Grid");
		m_GridMBM.CreateModel(gridVertexData, gridIndices, MeshBufferManager::ACCESS_TYPE::AT_STATIC);
	}
}

void Ocean::SetupScreenSpaceGrid ( unsigned short i_WindowWidth, unsigned short i_WindowHeight )
{
	std::vector<MeshBufferManager::VertexData> gridVertexData;
	std::vector<unsigned int> gridIndices;

	const float s = 1.0f;

	const float vmargin = 0.1f;
	const float hmargin = 0.1f;


	unsigned int index = 0, size = 0;

	//// generating a screen space grid
	unsigned int rows = static_cast<unsigned int>(glm::ceil(i_WindowHeight * (s + vmargin) / m_ScreenSpaceGridResolution) + 5.0f);
	unsigned int cols = static_cast<unsigned int>(glm::ceil(i_WindowWidth * (1.0f + 2.0f * hmargin) / m_ScreenSpaceGridResolution) + 5.0f);

	size = rows * cols;
	gridVertexData.resize(size);

	index = 0;
	unsigned int n = 0, r = 0, c = 0;
	for (float j = i_WindowHeight * s - 0.1f; j > -i_WindowHeight * vmargin - m_ScreenSpaceGridResolution; j -= m_ScreenSpaceGridResolution)
	{
		n = 0; c = 0;
		for (float i = -i_WindowWidth * hmargin; i < i_WindowWidth * (1.0f + hmargin) + m_ScreenSpaceGridResolution; i += m_ScreenSpaceGridResolution)
		{
			gridVertexData[index].position = glm::vec3(-1.0f + 2.0f * i / i_WindowWidth, -1.0f + 2.0f * j / i_WindowHeight, 0.0f);
			gridVertexData[index].uv = glm::vec2(static_cast<float>(c) / static_cast<float>(cols), static_cast<float>(r) / static_cast<float>(rows));
			++index;
			++n;
			++c;
		}
		++r;
	}

	////////// Indices //////
	size = 2 * (rows - 2) * (cols - 1);
	gridIndices.resize(size);

	index = 0;
	unsigned int ni = 0;
	for (float i = i_WindowHeight * s - vmargin; i > -i_WindowHeight * vmargin; i -= m_ScreenSpaceGridResolution)
	{
		unsigned int nj = 0;
		for (float j = -i_WindowWidth * hmargin; j < i_WindowWidth * (1.0f + hmargin); j += m_ScreenSpaceGridResolution)
		{
			index = (ni * n + nj) * 2;

			// CCW winding
			if (ni % 2 == 0)
			{
				gridIndices[index] = nj + ni * n;
				gridIndices[index + 1] = nj + (ni + 1) * n;
			}
			else
			{
				gridIndices[index] = n - 1 - nj + (ni + 1) * n;
				gridIndices[index + 1] = n - 1 - nj + ni * n;
			}

			++nj;
		}
		++ni;
	}

	m_GridVertexCount = gridVertexData.size();
	m_GridIndexCount = gridIndices.size();


	//////////
	m_GridMBM.Initialize("Ocean");
	m_GridMBM.CreateModel(gridVertexData, gridIndices, MeshBufferManager::ACCESS_TYPE::AT_STATIC);
}

void Ocean::SetupOceanSurface ( const GlobalConfig& i_Config )
{
	m_OceanSurfaceSM.Initialize("Ocean Surface");

	std::string fragmentShaderPath;
	if (m_SkyModelType == CustomTypes::Sky::ModelType::MT_PRECOMPUTED_SCATTERING)
	{
		fragmentShaderPath = "../resources/shaders/OceanSurfacePrecomputedScattering.frag.glsl";
	}
	else
	{
		fragmentShaderPath = "../resources/shaders/OceanSurface.frag.glsl";
	}

	if (m_GridType == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
	{
		m_OceanSurfaceSM.BuildRenderingProgram("../resources/shaders/OceanScreenGrid.vert.glsl", "../resources/shaders/OceanSurfaceScreenGrid.geom.glsl", fragmentShaderPath, i_Config);
	}
	else if (m_GridType == CustomTypes::Ocean::GridType::GT_WORLD_SPACE)
	{
		m_OceanSurfaceSM.BuildRenderingProgram("../resources/shaders/OceanSurfaceWorldGrid.vert.glsl", fragmentShaderPath, i_Config);
	}

	m_OceanSurfaceSM.UseProgram();

	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> oceanSurfaceAttributes;

	if (m_GridType == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
	{
		oceanSurfaceAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_OceanSurfaceSM.GetAttributeLocation("a_position");
	}
	else if (m_GridType == CustomTypes::Ocean::GridType::GT_WORLD_SPACE)
	{
		oceanSurfaceAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV] = m_OceanSurfaceSM.GetAttributeLocation("a_uv");
	}

	m_OceanSurfaceUniforms["u_HDRExposure"] = m_OceanSurfaceSM.GetUniformLocation("u_HDRExposure");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_HDRExposure")->second, i_Config.Rendering.HDR.Exposure);
	m_OceanSurfaceUniforms["u_CrrTime"] = m_OceanSurfaceSM.GetUniformLocation("u_CrrTime");

	// Projector data
	m_OceanSurfaceUniforms["u_ProjectingMatrix"] = m_OceanSurfaceSM.GetUniformLocation("u_ProjectingMatrix");
	m_OceanSurfaceUniforms["u_PlaneDistance"] = m_OceanSurfaceSM.GetUniformLocation("u_PlaneDistance");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_PlaneDistance")->second, m_WaveProjector.GetPlaneDistance());

	// Matrices
	m_OceanSurfaceUniforms["u_WorldToCameraMatrix"] = m_OceanSurfaceSM.GetUniformLocation("u_WorldToCameraMatrix");
	m_OceanSurfaceUniforms["u_WorldToClipMatrix"] = m_OceanSurfaceSM.GetUniformLocation("u_WorldToClipMatrix");

	if (m_GridType == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
	{
		m_OceanSurfaceUniforms["u_ClipToCameraMatrix"] = m_OceanSurfaceSM.GetUniformLocation("u_ClipToCameraMatrix");
		m_OceanSurfaceUniforms["u_CameraToWorldMatrix"] = m_OceanSurfaceSM.GetUniformLocation("u_CameraToWorldMatrix");
	}

	m_OceanSurfaceUniforms["u_CameraPosition"] = m_OceanSurfaceSM.GetUniformLocation("u_CameraPosition");

	// FFT Ocean Patch data
	if (m_pFFTOceanPatch)
	{
		m_OceanSurfaceUniforms["u_FFTOceanPatchData.FFTWaveDataMap"] = m_OceanSurfaceSM.GetUniformLocation("u_FFTOceanPatchData.FFTWaveDataMap");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_FFTOceanPatchData.FFTWaveDataMap")->second, m_pFFTOceanPatch->GetFFTWaveDataTexUnitId());

		m_OceanSurfaceUniforms["u_FFTOceanPatchData.NormalGradientFoldingMap"] = m_OceanSurfaceSM.GetUniformLocation("u_FFTOceanPatchData.NormalGradientFoldingMap");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_FFTOceanPatchData.NormalGradientFoldingMap")->second, m_pFFTOceanPatch->GetNormalGradientFoldingTexUnitId());

		m_OceanSurfaceUniforms["u_FFTOceanPatchData.PatchSize"] = m_OceanSurfaceSM.GetUniformLocation("u_FFTOceanPatchData.PatchSize");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_FFTOceanPatchData.PatchSize")->second, static_cast<float>(i_Config.Scene.Ocean.Surface.OceanPatch.FFTSize));

		m_OceanSurfaceUniforms["u_FFTOceanPatchData.WaveAmplitude"] = m_OceanSurfaceSM.GetUniformLocation("u_FFTOceanPatchData.WaveAmplitude");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_FFTOceanPatchData.WaveAmplitude")->second, m_pFFTOceanPatch->GetWaveAmplitude());

		m_OceanSurfaceUniforms["u_FFTOceanPatchData.WindSpeed"] = m_OceanSurfaceSM.GetUniformLocation("u_FFTOceanPatchData.WindSpeed");

		m_OceanSurfaceUniforms["u_FFTOceanPatchData.WindSpeedMixLimit"] = m_OceanSurfaceSM.GetUniformLocation("u_FFTOceanPatchData.WindSpeedMixLimit");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_FFTOceanPatchData.WindSpeedMixLimit")->second, i_Config.Scene.Ocean.Surface.OceanPatch.WindSpeedMixLimit);

		m_OceanSurfaceUniforms["u_FFTOceanPatchData.ChoppyScale"] = m_OceanSurfaceSM.GetUniformLocation("u_FFTOceanPatchData.ChoppyScale");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_FFTOceanPatchData.ChoppyScale")->second, m_pFFTOceanPatch->GetChoppyScale());

		m_OceanSurfaceUniforms["u_FFTOceanPatchData.TileScale"] = m_OceanSurfaceSM.GetUniformLocation("u_FFTOceanPatchData.TileScale");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_FFTOceanPatchData.TileScale")->second, m_pFFTOceanPatch->GetTileScale());

		m_OceanSurfaceUniforms["u_FFTOceanPatchData.UseFFTSlopes"] = m_OceanSurfaceSM.GetUniformLocation("u_FFTOceanPatchData.UseFFTSlopes");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_FFTOceanPatchData.UseFFTSlopes")->second, i_Config.Scene.Ocean.Surface.OceanPatch.ComputeFFT.UseFFTSlopes);
	}

	// Perlin Noise data
	m_OceanSurfaceUniforms["u_PerlinNoiseData.DisplacementMap"] = m_OceanSurfaceSM.GetUniformLocation("u_PerlinNoiseData.DisplacementMap");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_PerlinNoiseData.DisplacementMap")->second, i_Config.TexUnit.Ocean.Surface.PerlinDisplacementMap);
	m_OceanSurfaceUniforms["u_PerlinNoiseData.Movement"] = m_OceanSurfaceSM.GetUniformLocation("u_PerlinNoiseData.Movement");
	m_OceanSurfaceUniforms["u_PerlinNoiseData.Octaves"] = m_OceanSurfaceSM.GetUniformLocation("u_PerlinNoiseData.Octaves");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_PerlinNoiseData.Octaves")->second, 1, glm::value_ptr(i_Config.Scene.Ocean.Surface.PerlinNoise.Octaves), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
	m_OceanSurfaceUniforms["u_PerlinNoiseData.Amplitudes"] = m_OceanSurfaceSM.GetUniformLocation("u_PerlinNoiseData.Amplitudes");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_PerlinNoiseData.Amplitudes")->second, 1, glm::value_ptr(i_Config.Scene.Ocean.Surface.PerlinNoise.Amplitudes), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
	m_OceanSurfaceUniforms["u_PerlinNoiseData.Gradients"] = m_OceanSurfaceSM.GetUniformLocation("u_PerlinNoiseData.Gradients");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_PerlinNoiseData.Gradients")->second, 1, glm::value_ptr(i_Config.Scene.Ocean.Surface.PerlinNoise.Gradients), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

	////
	m_OceanSurfaceUniforms["u_WaveBlending.Begin"] = m_OceanSurfaceSM.GetUniformLocation("u_WaveBlending.Begin");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_WaveBlending.Begin")->second, i_Config.Scene.Ocean.Surface.WaveBlending.Begin);
	m_OceanSurfaceUniforms["u_WaveBlending.End"] = m_OceanSurfaceSM.GetUniformLocation("u_WaveBlending.End");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_WaveBlending.End")->second, i_Config.Scene.Ocean.Surface.WaveBlending.End);

	///////////////////////
	if (m_SkyModelType == CustomTypes::Sky::ModelType::MT_PRECOMPUTED_SCATTERING)
	{
		m_OceanSurfaceUniforms["u_TransmittanceMap"] = m_OceanSurfaceSM.GetUniformLocation("u_TransmittanceMap");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_TransmittanceMap")->second, i_Config.TexUnit.Sky.PrecomputedScatteringSkyModel.TransmittanceMap);

		m_OceanSurfaceUniforms["u_PrecomputedScatteringData.TransmittanceMap"] = m_OceanSurfaceSM.GetUniformLocation("u_PrecomputedScatteringData.TransmittanceMap");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_PrecomputedScatteringData.TransmittanceMap")->second, i_Config.TexUnit.Sky.PrecomputedScatteringSkyModel.TransmittanceMap);
		m_OceanSurfaceUniforms["u_PrecomputedScatteringData.EarthRadius"] = m_OceanSurfaceSM.GetUniformLocation("u_PrecomputedScatteringData.EarthRadius");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_PrecomputedScatteringData.EarthRadius")->second, PhysicsConstants::kEarthRadius);
		m_OceanSurfaceUniforms["u_PrecomputedScatteringData.SunIntensity"] = m_OceanSurfaceSM.GetUniformLocation("u_PrecomputedScatteringData.SunIntensity");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_PrecomputedScatteringData.SunIntensity")->second, i_Config.Scene.Sky.Model.PrecomputedScattering.Atmosphere.SunIntensity);
		m_OceanSurfaceUniforms["u_PrecomputedScatteringData.PI"] = m_OceanSurfaceSM.GetUniformLocation("u_PrecomputedScatteringData.PI");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_PrecomputedScatteringData.PI")->second, glm::pi<float>());
		m_OceanSurfaceUniforms["u_PrecomputedScatteringData.Rgtl"] = m_OceanSurfaceSM.GetUniformLocation("u_PrecomputedScatteringData.Rgtl");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_PrecomputedScatteringData.Rgtl")->second, 1, glm::value_ptr(i_Config.Scene.Sky.Model.PrecomputedScattering.Atmosphere.Rgtl), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
	}

	//////////////////////
	m_OceanSurfaceUniforms["u_ReflectionMap"] = m_OceanSurfaceSM.GetUniformLocation("u_ReflectionMap");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_ReflectionMap")->second, i_Config.TexUnit.Global.ReflectionMap);
	m_OceanSurfaceUniforms["u_RefractionMap"] = m_OceanSurfaceSM.GetUniformLocation("u_RefractionMap");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_RefractionMap")->second, i_Config.TexUnit.Global.RefractionMap);

	m_OceanSurfaceUniforms["u_ReflectionDistortFactor"] = m_OceanSurfaceSM.GetUniformLocation("u_ReflectionDistortFactor");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_ReflectionDistortFactor")->second, i_Config.Scene.Ocean.Surface.ReflectionDistortFactor);
	m_OceanSurfaceUniforms["u_RefractionDistortFactor"] = m_OceanSurfaceSM.GetUniformLocation("u_RefractionDistortFactor");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_RefractionDistortFactor")->second, i_Config.Scene.Ocean.Surface.RefractionDistortFactor);

	m_OceanSurfaceUniforms["u_WaterColor"] = m_OceanSurfaceSM.GetUniformLocation("u_WaterColor");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_WaterColor")->second, 1, glm::value_ptr(i_Config.Scene.Ocean.Surface.WaterColor), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
	m_OceanSurfaceUniforms["u_WaterRefrColor"] = m_OceanSurfaceSM.GetUniformLocation("u_WaterRefrColor");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_WaterRefrColor")->second, 1, glm::value_ptr(i_Config.Scene.Ocean.Surface.WaterRefrColor), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

	m_OceanSurfaceUniforms["u_WavesFoamData.Map"] = m_OceanSurfaceSM.GetUniformLocation("u_WavesFoamData.Map");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_WavesFoamData.Map")->second, i_Config.TexUnit.Ocean.Surface.WavesFoamMap);
	m_OceanSurfaceUniforms["u_WavesFoamData.ScaleFactor"] = m_OceanSurfaceSM.GetUniformLocation("u_WavesFoamData.ScaleFactor");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_WavesFoamData.ScaleFactor")->second, i_Config.Scene.Ocean.Surface.Foam.ScaleFactor);

	if (i_Config.Scene.Ocean.Surface.SubSurfaceScattering.Enabled)
	{
		m_OceanSurfaceUniforms["u_WavesSSSData.Color"] = m_OceanSurfaceSM.GetUniformLocation("u_WavesSSSData.Color");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_WavesSSSData.Color")->second, 1, glm::value_ptr(i_Config.Scene.Ocean.Surface.SubSurfaceScattering.Color), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
		m_OceanSurfaceUniforms["u_WavesSSSData.Scale"] = m_OceanSurfaceSM.GetUniformLocation("u_WavesSSSData.Scale");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_WavesSSSData.Scale")->second, i_Config.Scene.Ocean.Surface.SubSurfaceScattering.Scale);
		m_OceanSurfaceUniforms["u_WavesSSSData.Power"] = m_OceanSurfaceSM.GetUniformLocation("u_WavesSSSData.Power");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_WavesSSSData.Power")->second, i_Config.Scene.Ocean.Surface.SubSurfaceScattering.Power);
		m_OceanSurfaceUniforms["u_WavesSSSData.WaveHeightScale"] = m_OceanSurfaceSM.GetUniformLocation("u_WavesSSSData.WaveHeightScale");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_WavesSSSData.WaveHeightScale")->second, i_Config.Scene.Ocean.Surface.SubSurfaceScattering.WaveHeightScale);
		m_OceanSurfaceUniforms["u_WavesSSSData.MaxAllowedValue"] = m_OceanSurfaceSM.GetUniformLocation("u_WavesSSSData.MaxAllowedValue");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_WavesSSSData.MaxAllowedValue")->second, i_Config.Scene.Ocean.Surface.SubSurfaceScattering.MaxAllowedValue);
	}

	// Under Water Color
	m_OceanSurfaceUniforms["u_UnderWaterColor"] = m_OceanSurfaceSM.GetUniformLocation("u_UnderWaterColor");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_UnderWaterColor")->second, 1, glm::value_ptr(i_Config.Scene.Ocean.Surface.UnderWater.Color), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

	// Under Water Fog
	m_OceanSurfaceUniforms["u_UnderWaterFog.Color"] = m_OceanSurfaceSM.GetUniformLocation("u_UnderWaterFog.Color");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_UnderWaterFog.Color")->second, 1, glm::value_ptr(i_Config.Scene.Ocean.Surface.UnderWater.Fog.Color), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
	m_OceanSurfaceUniforms["u_UnderWaterFog.Density"] = m_OceanSurfaceSM.GetUniformLocation("u_UnderWaterFog.Density");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_UnderWaterFog.Density")->second, i_Config.Scene.Ocean.Surface.UnderWater.Fog.Density);

	m_OceanSurfaceUniforms["u_MaxFadeAltitude"] = m_OceanSurfaceSM.GetUniformLocation("u_MaxFadeAltitude");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_MaxFadeAltitude")->second, i_Config.Scene.Ocean.Surface.MaxFadeAltitude);

	m_OceanSurfaceUniforms["u_IsUnderWater"] = m_OceanSurfaceSM.GetUniformLocation("u_IsUnderWater");
	m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_IsUnderWater")->second, false);


	// Sun data
	if (m_SkyModelType == CustomTypes::Sky::ModelType::MT_CUBE_MAP || m_SkyModelType == CustomTypes::Sky::ModelType::MT_SCATTERING)
	{
		float sunShininess = (m_SkyModelType == CustomTypes::Sky::ModelType::MT_CUBE_MAP ? 
			i_Config.Scene.Sky.Model.Cubemap.Sun.Shininess : i_Config.Scene.Sky.Model.Scattering.Sun.Shininess);
		float sunStrength = (m_SkyModelType == CustomTypes::Sky::ModelType::MT_CUBE_MAP ?
			i_Config.Scene.Sky.Model.Cubemap.Sun.Strength : i_Config.Scene.Sky.Model.Scattering.Sun.Strength);

		m_OceanSurfaceUniforms["u_SunData.Shininess"] = m_OceanSurfaceSM.GetUniformLocation("u_SunData.Shininess");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_SunData.Shininess")->second, sunShininess);
		m_OceanSurfaceUniforms["u_SunData.Strength"] = m_OceanSurfaceSM.GetUniformLocation("u_SunData.Strength");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_SunData.Strength")->second, sunStrength);
	}

	m_OceanSurfaceUniforms["u_SunData.Direction"] = m_OceanSurfaceSM.GetUniformLocation("u_SunData.Direction");

	if (i_Config.Scene.Ocean.Surface.BoatEffects.Foam.Enabled)
	{
		m_OceanSurfaceUniforms["u_BoatFoamData.Map"] = m_OceanSurfaceSM.GetUniformLocation("u_BoatFoamData.Map");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_BoatFoamData.Map")->second, i_Config.TexUnit.Ocean.Surface.BoatFoamMap);
		m_OceanSurfaceUniforms["u_BoatFoamData.Scale"] = m_OceanSurfaceSM.GetUniformLocation("u_BoatFoamData.Scale");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_BoatFoamData.Scale")->second, i_Config.Scene.Ocean.Surface.BoatEffects.Foam.Scale);
	}

	if (m_EnableBoatKelvinWake)
	{
		m_OceanSurfaceUniforms["u_BoatKelvinWakeData.DispNormMap"] = m_OceanSurfaceSM.GetUniformLocation("u_BoatKelvinWakeData.DispNormMap");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_BoatKelvinWakeData.DispNormMap")->second, i_Config.TexUnit.Ocean.Surface.KelvinWakeDispNormMap);

		m_OceanSurfaceUniforms["u_BoatKelvinWakeData.FoamMap"] = m_OceanSurfaceSM.GetUniformLocation("u_BoatKelvinWakeData.FoamMap");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_BoatKelvinWakeData.FoamMap")->second, i_Config.TexUnit.Ocean.Surface.KelvinWakeFoamMap);

		m_OceanSurfaceUniforms["u_BoatKelvinWakeData.Scale"] = m_OceanSurfaceSM.GetUniformLocation("u_BoatKelvinWakeData.Scale");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_BoatKelvinWakeData.Scale")->second, i_Config.Scene.Ocean.Surface.BoatEffects.KelvinWake.Scale);

		m_OceanSurfaceUniforms["u_BoatKelvinWakeData.BoatPosition"] = m_OceanSurfaceSM.GetUniformLocation("u_BoatKelvinWakeData.BoatPosition");
		m_OceanSurfaceUniforms["u_BoatKelvinWakeData.WakePosition"] = m_OceanSurfaceSM.GetUniformLocation("u_BoatKelvinWakeData.WakePosition");
		m_OceanSurfaceUniforms["u_BoatKelvinWakeData.Amplitude"] = m_OceanSurfaceSM.GetUniformLocation("u_BoatKelvinWakeData.Amplitude");
		m_OceanSurfaceUniforms["u_BoatKelvinWakeData.FoamAmount"] = m_OceanSurfaceSM.GetUniformLocation("u_BoatKelvinWakeData.FoamAmount");
	}

	if (m_EnableBoatPropellerWash)
	{
		m_OceanSurfaceUniforms["u_BoatPropellerWashData.Map"] = m_OceanSurfaceSM.GetUniformLocation("u_BoatPropellerWashData.Map");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_BoatPropellerWashData.Map")->second, i_Config.TexUnit.Ocean.Surface.PropellerWashMap);
		m_OceanSurfaceUniforms["u_BoatPropellerWashData.DistortFactor"] = m_OceanSurfaceSM.GetUniformLocation("u_BoatPropellerWashData.DistortFactor");
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_BoatPropellerWashData.DistortFactor")->second, i_Config.Scene.Ocean.Surface.BoatEffects.PropellerWash.DistortFactor);
	}

	m_OceanSurfaceSM.UnUseProgram();

	//////////////////////
	m_OceanSurfaceMBM.Initialize("Ocean Surface");
	m_OceanSurfaceMBM.CreateModelContext(oceanSurfaceAttributes, m_GridMBM.GetVBOID(), m_GridMBM.GetIBOID(), m_GridMBM.GetAccessType());
}

void Ocean::SetupOceanBottom ( const GlobalConfig& i_Config )
{
	m_OceanBottomSM.Initialize("Ocean Bottom");

	if (m_GridType == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
	{
		m_OceanBottomSM.BuildRenderingProgram("../resources/shaders/OceanScreenGrid.vert.glsl", "../resources/shaders/OceanBottomScreenGrid.geom.glsl", "../resources/shaders/OceanBottom.frag.glsl", i_Config);
	}
	else if (m_GridType == CustomTypes::Ocean::GridType::GT_WORLD_SPACE)
	{
		m_OceanBottomSM.BuildRenderingProgram("../resources/shaders/OceanBottomWorldGrid.vert.glsl", "../resources/shaders/OceanBottom.frag.glsl", i_Config);
	}

	m_OceanBottomSM.UseProgram();

	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> oceanBottomAttributes;
	oceanBottomAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV] = m_OceanBottomSM.GetAttributeLocation("a_uv");

	if (m_GridType == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
	{
		oceanBottomAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_OceanBottomSM.GetAttributeLocation("a_position");
	}

	m_OceanBottomUniforms["u_HDRExposure"] = m_OceanBottomSM.GetUniformLocation("u_HDRExposure");
	m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_HDRExposure")->second, i_Config.Rendering.HDR.Exposure);

	m_OceanBottomUniforms["u_PlaneDistance"] = m_OceanBottomSM.GetUniformLocation("u_PlaneDistance");
	m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_PlaneDistance")->second, m_BottomProjector.GetPlaneDistance());

	m_OceanBottomUniforms["u_ProjectingMatrix"] = m_OceanBottomSM.GetUniformLocation("u_ProjectingMatrix");
	m_OceanBottomUniforms["u_WorldToCameraMatrix"] = m_OceanBottomSM.GetUniformLocation("u_WorldToCameraMatrix");
	m_OceanBottomUniforms["u_WorldToClipMatrix"] = m_OceanBottomSM.GetUniformLocation("u_WorldToClipMatrix");

	if (m_GridType == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
	{
		m_OceanBottomUniforms["u_ClipToCameraMatrix"] = m_OceanBottomSM.GetUniformLocation("u_ClipToCameraMatrix");
		m_OceanBottomUniforms["u_CameraToWorldMatrix"] = m_OceanBottomSM.GetUniformLocation("u_CameraToWorldMatrix");
		m_OceanBottomUniforms["u_CameraPosition"] = m_OceanBottomSM.GetUniformLocation("u_CameraPosition");
	}

	m_OceanBottomUniforms["u_SunDirection"] = m_OceanBottomSM.GetUniformLocation("u_SunDirection");

	m_OceanBottomUniforms["u_SandData.DiffuseMap"] = m_OceanBottomSM.GetUniformLocation("u_SandData.DiffuseMap");
	m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_SandData.DiffuseMap")->second, i_Config.TexUnit.Ocean.Bottom.SandDiffuseMap);
	m_OceanBottomUniforms["u_SandData.Scale"] = m_OceanBottomSM.GetUniformLocation("u_SandData.Scale");
	m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_SandData.Scale")->second, i_Config.Scene.Ocean.Bottom.Sand.Scale);

	m_OceanBottomUniforms["u_PerlinData.DisplacementMap"] = m_OceanBottomSM.GetUniformLocation("u_PerlinData.DisplacementMap");
	m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_PerlinData.DisplacementMap")->second, i_Config.TexUnit.Ocean.Surface.PerlinDisplacementMap);
	m_OceanBottomUniforms["u_PerlinData.Scale"] = m_OceanBottomSM.GetUniformLocation("u_PerlinData.Scale");
	m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_PerlinData.Scale")->second, i_Config.Scene.Ocean.Bottom.PerlinNoise.Scale);
	m_OceanBottomUniforms["u_PerlinData.Amplitude"] = m_OceanBottomSM.GetUniformLocation("u_PerlinData.Amplitude");
	m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_PerlinData.Amplitude")->second, i_Config.Scene.Ocean.Bottom.PerlinNoise.Amplitude);

	m_OceanBottomUniforms["u_PatchSize"] = m_OceanBottomSM.GetUniformLocation("u_PatchSize");
	m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_PatchSize")->second, i_Config.Scene.Ocean.Bottom.PatchSize);

	m_OceanBottomUniforms["u_CausticsData.Map"] = m_OceanBottomSM.GetUniformLocation("u_CausticsData.Map");
	m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_CausticsData.Map")->second, i_Config.TexUnit.Ocean.Bottom.CausticsMap);
	m_OceanBottomUniforms["u_CausticsData.Scale"] = m_OceanBottomSM.GetUniformLocation("u_CausticsData.Scale");
	m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_CausticsData.Scale")->second, i_Config.Scene.Ocean.Bottom.Caustics.Scale);

	if (m_pFFTOceanPatch)
	{
		m_OceanBottomUniforms["u_TileScale"] = m_OceanBottomSM.GetUniformLocation("u_TileScale");
		m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_TileScale")->second, m_pFFTOceanPatch->GetTileScale());
	}

	m_OceanBottomUniforms["u_FogData.Color"] = m_OceanBottomSM.GetUniformLocation("u_FogData.Color");
	m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_FogData.Color")->second, 1, glm::value_ptr(i_Config.Scene.Ocean.Bottom.Fog.Color), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
	m_OceanBottomUniforms["u_FogData.Density"] = m_OceanBottomSM.GetUniformLocation("u_FogData.Density");
	m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_FogData.Density")->second, i_Config.Scene.Ocean.Bottom.Fog.Density);

	m_OceanBottomSM.UnUseProgram();

	///////////////////
	m_OceanBottomMBM.Initialize("Ocean Bottom");
	m_OceanBottomMBM.CreateModelContext(oceanBottomAttributes, m_GridMBM.GetVBOID(), m_GridMBM.GetIBOID(), m_GridMBM.GetAccessType());
}

void Ocean::SetupOceanBottomCaustics ( const GlobalConfig& i_Config )
{
	if (m_EnableBottomCaustics)
	{
		m_OceanCausticsSM.Initialize("Ocean Bottom Caustics");

		if (m_GridType == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
		{
			m_OceanCausticsSM.BuildRenderingProgram("../resources/shaders/OceanScreenGrid.vert.glsl", "../resources/shaders/OceanCausticsScreenGrid.geom.glsl", "../resources/shaders/OceanCaustics.frag.glsl", i_Config);
		}
		else if (m_GridType == CustomTypes::Ocean::GridType::GT_WORLD_SPACE)
		{
			m_OceanCausticsSM.BuildRenderingProgram("../resources/shaders/OceanCausticsWorldGrid.vert.glsl", "../resources/shaders/OceanCaustics.frag.glsl", i_Config);
		}

		m_OceanCausticsSM.UseProgram();

		std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> oceanCausticsAttributes;

		if (m_GridType == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
		{
			oceanCausticsAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_OceanCausticsSM.GetAttributeLocation("a_position");
		}
		else if (m_GridType == CustomTypes::Ocean::GridType::GT_WORLD_SPACE)
		{
			oceanCausticsAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV] = m_OceanCausticsSM.GetAttributeLocation("a_uv");
		}

		m_OceanCausticsUniforms["u_BottomPlaneDistance"] = m_OceanCausticsSM.GetUniformLocation("u_BottomPlaneDistance");
		m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_BottomPlaneDistance")->second, m_BottomProjector.GetPlaneDistance());

		m_OceanCausticsUniforms["u_PlaneDistanceOffset"] = m_OceanCausticsSM.GetUniformLocation("u_PlaneDistanceOffset");
		m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_PlaneDistanceOffset")->second, i_Config.Scene.Ocean.Bottom.Caustics.PlaneDistanceOffset);

		m_OceanCausticsUniforms["u_BottomProjectingMatrix"] = m_OceanCausticsSM.GetUniformLocation("u_BottomProjectingMatrix");

		m_OceanCausticsUniforms["u_WorldToClipMatrix"] = m_OceanCausticsSM.GetUniformLocation("u_WorldToClipMatrix");

		if (m_GridType == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
		{
			m_OceanCausticsUniforms["u_ClipToCameraMatrix"] = m_OceanCausticsSM.GetUniformLocation("u_ClipToCameraMatrix");
			m_OceanCausticsUniforms["u_CameraToWorldMatrix"] = m_OceanCausticsSM.GetUniformLocation("u_CameraToWorldMatrix");
			m_OceanCausticsUniforms["u_CameraPosition"] = m_OceanCausticsSM.GetUniformLocation("u_CameraPosition");
		}

		m_OceanCausticsUniforms["u_SunDirection"] = m_OceanCausticsSM.GetUniformLocation("u_SunDirection");

		if (m_pFFTOceanPatch)
		{
			m_OceanCausticsUniforms["u_FFTWaveDataMap"] = m_OceanCausticsSM.GetUniformLocation("u_FFTWaveDataMap");
			m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_FFTWaveDataMap")->second, m_pFFTOceanPatch->GetFFTWaveDataTexUnitId());

			m_OceanCausticsUniforms["u_PatchSize"] = m_OceanCausticsSM.GetUniformLocation("u_PatchSize");
			m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_PatchSize")->second, static_cast<float>(m_pFFTOceanPatch->GetPatchSize()));

			m_OceanCausticsUniforms["u_ChoppyScale"] = m_OceanCausticsSM.GetUniformLocation("u_ChoppyScale");
			m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_ChoppyScale")->second, m_pFFTOceanPatch->GetChoppyScale());
		}

		m_OceanCausticsUniforms["u_CausticsData.Color"] = m_OceanCausticsSM.GetUniformLocation("u_CausticsData.Color");
		m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_CausticsData.Color")->second, 1, glm::value_ptr(i_Config.Scene.Ocean.Bottom.Caustics.Color), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3),
		m_OceanCausticsUniforms["u_CausticsData.Intensity"] = m_OceanCausticsSM.GetUniformLocation("u_CausticsData.Intensity");
		m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_CausticsData.Intensity")->second, i_Config.Scene.Ocean.Bottom.Caustics.Intensity),

		m_OceanCausticsSM.UnUseProgram();

		m_OceanCausticsMBM.Initialize("Ocean Caustics");
		m_OceanCausticsMBM.CreateModelContext(oceanCausticsAttributes, m_GridMBM.GetVBOID(), m_GridMBM.GetIBOID(), m_GridMBM.GetAccessType());

		m_OceanCausticsFBM.Initialize("Ocean Caustics", i_Config);
		m_OceanCausticsFBM.CreateSimple(1, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, m_CausticsMapSize, m_CausticsMapSize, GL_REPEAT, GL_LINEAR, i_Config.TexUnit.Ocean.Bottom.CausticsMap, 5);
	}
}

void Ocean::SetupOceanBottomGodRays ( const GlobalConfig& i_Config )
{
	if (m_EnableUnderWaterGodRays)
	{
		m_UnderWaterGodRaysData.NumberOfSamples = i_Config.Scene.Ocean.UnderWater.GodRays.NumberOfSamples;
		m_UnderWaterGodRaysData.Exposure = i_Config.Scene.Ocean.UnderWater.GodRays.Exposure;
		m_UnderWaterGodRaysData.Decay = i_Config.Scene.Ocean.UnderWater.GodRays.Decay;
		m_UnderWaterGodRaysData.Density = i_Config.Scene.Ocean.UnderWater.GodRays.Density;
		m_UnderWaterGodRaysData.Weight = i_Config.Scene.Ocean.UnderWater.GodRays.Weight;

		////////////////////////

		// occluder patch grid
		unsigned short occluderSize = i_Config.Scene.Ocean.UnderWater.GodRays.Occluder.Size;
		std::vector<MeshBufferManager::VertexData> occluderVertexData;
		occluderVertexData.resize(occluderSize * occluderSize);

		int index = 0;
		unsigned short step = i_Config.Scene.Ocean.UnderWater.GodRays.Occluder.Step;
		for (unsigned short i = 0; i < occluderSize; i += step)
		{
			for (unsigned short j = 0; j < occluderSize; j += step)
			{
				occluderVertexData[index].position.x = (j - occluderSize / 2.0f);
				occluderVertexData[index].position.y = 0.0f;
				occluderVertexData[index].position.z = (i - occluderSize / 2.0f);

				occluderVertexData[index].uv.x = static_cast<float>(j) / occluderSize;
				occluderVertexData[index].uv.y = static_cast<float>(i) / occluderSize;

				index++;
			}
		}

		std::vector<unsigned int> occluderIndices;
		occluderIndices.resize(2 * occluderSize * (occluderSize - 1));
		m_OccluderIndexCount = occluderIndices.size();

		for (unsigned short i = 0; i < occluderSize - 1; ++i)
		{
			for (unsigned short j = 0; j < occluderSize; ++j)
			{
				index = (j + i * occluderSize) * 2;
				if (i % 2 == 0)
				{
					occluderIndices[index] = j + i * occluderSize;
					occluderIndices[index + 1] = j + (i + 1) * occluderSize;
				}
				else
				{
					occluderIndices[index] = occluderSize - 1 - j + (i + 1) * occluderSize;
					occluderIndices[index + 1] = occluderSize - 1 - j + i * occluderSize;
				}
			}
		}

		////// OCCLUDER SETUP ///////
		m_OceanOccluderSM.Initialize("Ocean Occluder");
		m_OceanOccluderSM.BuildRenderingProgram("../resources/shaders/OceanOccluder.vert.glsl", "../resources/shaders/OceanOccluder.frag.glsl", i_Config);

		m_OceanOccluderSM.UseProgram();

		std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> oceanOccluderAttributes;
		oceanOccluderAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_OceanOccluderSM.GetAttributeLocation("a_position");
		oceanOccluderAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV] = m_OceanOccluderSM.GetAttributeLocation("a_uv");

		m_OceanOccluderUniforms["u_WorldToClipMatrix"] = m_OceanOccluderSM.GetUniformLocation("u_WorldToClipMatrix");

		if (m_pFFTOceanPatch)
		{
			m_OceanOccluderUniforms["u_FFTWaveDataMap"] = m_OceanOccluderSM.GetUniformLocation("u_FFTWaveDataMap");
			m_OceanOccluderSM.SetUniform(m_OceanOccluderUniforms.find("u_FFTWaveDataMap")->second, m_pFFTOceanPatch->GetFFTWaveDataTexUnitId());

			m_OceanOccluderUniforms["u_ChoppyScale"] = m_OceanOccluderSM.GetUniformLocation("u_ChoppyScale");
			m_OceanOccluderSM.SetUniform(m_OceanOccluderUniforms.find("u_ChoppyScale")->second, m_pFFTOceanPatch->GetChoppyScale());
		}

		m_OceanOccluderUniforms["u_SunDirY"] = m_OceanOccluderSM.GetUniformLocation("u_SunDirY");

		m_OceanOccluderSM.UnUseProgram();

		m_OceanOccluderMBM.Initialize("Ocean Occluders");
		m_OceanOccluderMBM.CreateModelContext(occluderVertexData, occluderIndices, oceanOccluderAttributes, MeshBufferManager::ACCESS_TYPE::AT_STATIC);

		////// GOD RAYS SETUP ///////
		m_OceanGodRaysSM.Initialize("Ocean God Rays");
		m_OceanGodRaysSM.BuildRenderingProgram("../resources/shaders/Quad.vert.glsl", "../resources/shaders/OceanGodRays.frag.glsl", i_Config);

		m_OceanGodRaysSM.UseProgram();

		std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> oceanGodRaysAttributes;
		oceanGodRaysAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_OceanGodRaysSM.GetAttributeLocation("a_position");
		oceanGodRaysAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV] = m_OceanGodRaysSM.GetAttributeLocation("a_uv");

		m_OceanGodRaysUniforms["u_GodRaysData.NumberOfSamples"] = m_OceanGodRaysSM.GetUniformLocation("u_GodRaysData.NumberOfSamples");
		m_OceanGodRaysSM.SetUniform(m_OceanGodRaysUniforms.find("u_GodRaysData.NumberOfSamples")->second, m_UnderWaterGodRaysData.NumberOfSamples);

		m_OceanGodRaysUniforms["u_GodRaysData.Exposure"] = m_OceanGodRaysSM.GetUniformLocation("u_GodRaysData.Exposure");
		m_OceanGodRaysSM.SetUniform(m_OceanGodRaysUniforms.find("u_GodRaysData.Exposure")->second, m_UnderWaterGodRaysData.Exposure);
		m_OceanGodRaysUniforms["u_GodRaysData.Decay"] = m_OceanGodRaysSM.GetUniformLocation("u_GodRaysData.Decay");
		m_OceanGodRaysSM.SetUniform(m_OceanGodRaysUniforms.find("u_GodRaysData.Decay")->second, m_UnderWaterGodRaysData.Decay);
		m_OceanGodRaysUniforms["u_GodRaysData.Density"] = m_OceanGodRaysSM.GetUniformLocation("u_GodRaysData.Density");
		m_OceanGodRaysSM.SetUniform(m_OceanGodRaysUniforms.find("u_GodRaysData.Density")->second, m_UnderWaterGodRaysData.Density);
		m_OceanGodRaysUniforms["u_GodRaysData.Weight"] = m_OceanGodRaysSM.GetUniformLocation("u_GodRaysData.Weight");
		m_OceanGodRaysSM.SetUniform(m_OceanGodRaysUniforms.find("u_GodRaysData.Weight")->second, m_UnderWaterGodRaysData.Weight);
		m_OceanGodRaysUniforms["u_GodRaysData.LightDirectionOnScreen"] = m_OceanGodRaysSM.GetUniformLocation("u_GodRaysData.LightDirectionOnScreen");

		m_OceanGodRaysUniforms["u_GodRaysData.OccluderMap"] = m_OceanGodRaysSM.GetUniformLocation("u_GodRaysData.OccluderMap");
		m_OceanGodRaysSM.SetUniform(m_OceanGodRaysUniforms.find("u_GodRaysData.OccluderMap")->second, i_Config.TexUnit.Ocean.UnderWater.GodRaysMap);

		m_OceanCausticsSM.UnUseProgram();

		m_OceanGodRaysFBM.Initialize("Ocean God Rays", i_Config);
		/////////
		glm::ivec4 viewport;
		glGetIntegerv(GL_VIEWPORT, &viewport[0]);

		m_GodRaysMapWidth = viewport.z;
		m_GodRaysMapHeight = viewport.w;

		// NOTE! For some reason the god rays depth works only when the render buffer is used, there are issues when using the texture as depth only or depth-stencil buffer
		m_OceanGodRaysFBM.CreateSimple(oceanGodRaysAttributes, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, m_GodRaysMapWidth, m_GodRaysMapHeight, GL_CLAMP_TO_EDGE, GL_LINEAR, i_Config.TexUnit.Ocean.UnderWater.GodRaysMap, -1, false, FrameBufferManager::DEPTH_BUFFER_TYPE::DBT_RENDER_BUFFER_DEPTH);
		/////////
	}
}

void Ocean::SetupDebugFrustum ( const GlobalConfig& i_Config )
{
	m_FrustumSM.Initialize("Ocean Frustums");
	m_FrustumSM.BuildRenderingProgram("../resources/shaders/OceanFrustum.vert.glsl", "../resources/shaders/OceanFrustum.frag.glsl", i_Config);

	m_FrustumSM.UseProgram();

	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> frustumAttributes;
	frustumAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_FrustumSM.GetAttributeLocation("a_position");

	m_FrustumUniforms["u_ViewClipToWorldMatrix"] = m_FrustumSM.GetUniformLocation("u_ViewClipToWorldMatrix");
	m_FrustumUniforms["u_ProjectorClipToWorldMatrix"] = m_FrustumSM.GetUniformLocation("u_ProjectorClipToWorldMatrix");
	m_FrustumUniforms["u_WorldToClipMatrix"] = m_FrustumSM.GetUniformLocation("u_WorldToClipMatrix");

	m_FrustumUniforms["u_IsViewFrustum"] = m_FrustumSM.GetUniformLocation("u_IsViewFrustum");

	m_FrustumUniforms["u_Color"] = m_FrustumSM.GetUniformLocation("u_Color");

	m_FrustumSM.UnUseProgram();

	/////
	std::vector<MeshBufferManager::VertexData> frustumVertexData;
	frustumVertexData.resize(48);

	//////// CAMERA FRUSTUM ///////////////////
	// near
	// 0 -> 1
	frustumVertexData[0].position = glm::vec3(-1.0f, -1.0f, -1.0f);
	frustumVertexData[1].position = glm::vec3(+1.0f, -1.0f, -1.0f);

	// 2 -> 3
	frustumVertexData[2].position = glm::vec3(-1.0f, +1.0f, -1.0f);
	frustumVertexData[3].position = glm::vec3(+1.0f, +1.0f, -1.0f);

	// 0 -> 2
	frustumVertexData[4].position = glm::vec3(-1.0f, -1.0f, -1.0f);
	frustumVertexData[5].position = glm::vec3(-1.0f, +1.0f, -1.0f);

	// 1 -> 3
	frustumVertexData[6].position = glm::vec3(+1.0f, -1.0f, -1.0f);
	frustumVertexData[7].position = glm::vec3(+1.0f, +1.0f, -1.0f);

	// far
	// 4 -> 5
	frustumVertexData[8].position = glm::vec3(-1.0f, -1.0f, +1.0f);
	frustumVertexData[9].position = glm::vec3(+1.0f, -1.0f, +1.0f);

	// 6 -> 7
	frustumVertexData[10].position = glm::vec3(-1.0f, +1.0f, +1.0f);
	frustumVertexData[11].position = glm::vec3(+1.0f, +1.0f, +1.0f);

	// 4 -> 6
	frustumVertexData[12].position = glm::vec3(-1.0f, -1.0f, +1.0f);
	frustumVertexData[13].position = glm::vec3(-1.0f, +1.0f, +1.0f);

	// 5 -> 7
	frustumVertexData[14].position = glm::vec3(+1.0f, -1.0f, +1.0f);
	frustumVertexData[15].position = glm::vec3(+1.0f, +1.0f, +1.0f);

	// connectors
	// 0 -> 4
	frustumVertexData[16].position = glm::vec3(-1.0f, -1.0f, -1.0f);
	frustumVertexData[17].position = glm::vec3(-1.0f, -1.0f, +1.0f);

	// 1 -> 5
	frustumVertexData[18].position = glm::vec3(+1.0f, -1.0f, -1.0f);
	frustumVertexData[19].position = glm::vec3(+1.0f, -1.0f, +1.0f);

	// 2 -> 6
	frustumVertexData[20].position = glm::vec3(-1.0f, +1.0f, -1.0f);
	frustumVertexData[21].position = glm::vec3(-1.0f, +1.0f, +1.0f);

	// 3 -> 7
	frustumVertexData[22].position = glm::vec3(+1.0f, +1.0f, -1.0f);
	frustumVertexData[23].position = glm::vec3(+1.0f, +1.0f, +1.0f);

	//////////// PROJECTING CAMERA FRUSTUM /////////////////
	// near
	// 0 -> 1
	frustumVertexData[24].position = glm::vec3(-1.0f, -1.0f, -1.0f);
	frustumVertexData[25].position = glm::vec3(+1.0f, -1.0f, -1.0f);

	// 2 -> 3
	frustumVertexData[26].position = glm::vec3(-1.0f, +1.0f, -1.0f);
	frustumVertexData[27].position = glm::vec3(+1.0f, +1.0f, -1.0f);

	// 0 -> 2
	frustumVertexData[28].position = glm::vec3(-1.0f, -1.0f, -1.0f);
	frustumVertexData[29].position = glm::vec3(-1.0f, +1.0f, -1.0f);

	// 1 -> 3
	frustumVertexData[30].position = glm::vec3(+1.0f, -1.0f, -1.0f);
	frustumVertexData[31].position = glm::vec3(+1.0f, +1.0f, -1.0f);

	// far
	// 4 -> 5
	frustumVertexData[32].position = glm::vec3(-1.0f, -1.0f, +1.0f);
	frustumVertexData[33].position = glm::vec3(+1.0f, -1.0f, +1.0f);

	// 6 -> 7
	frustumVertexData[34].position = glm::vec3(-1.0f, +1.0f, +1.0f);
	frustumVertexData[35].position = glm::vec3(+1.0f, +1.0f, +1.0f);

	// 4 -> 6
	frustumVertexData[36].position = glm::vec3(-1.0f, -1.0f, +1.0f);
	frustumVertexData[37].position = glm::vec3(-1.0f, +1.0f, +1.0f);

	// 5 -> 7
	frustumVertexData[38].position = glm::vec3(+1.0f, -1.0f, +1.0f);
	frustumVertexData[39].position = glm::vec3(+1.0f, +1.0f, +1.0f);

	// connectors
	// 0 -> 4
	frustumVertexData[40].position = glm::vec3(-1.0f, -1.0f, -1.0f);
	frustumVertexData[41].position = glm::vec3(-1.0f, -1.0f, +1.0f);

	// 1 -> 5
	frustumVertexData[42].position = glm::vec3(+1.0f, -1.0f, -1.0f);
	frustumVertexData[43].position = glm::vec3(+1.0f, -1.0f, +1.0f);

	// 2 -> 6
	frustumVertexData[44].position = glm::vec3(-1.0f, +1.0f, -1.0f);
	frustumVertexData[45].position = glm::vec3(-1.0f, +1.0f, +1.0f);

	// 3 -> 7
	frustumVertexData[46].position = glm::vec3(+1.0f, +1.0f, -1.0f);
	frustumVertexData[47].position = glm::vec3(+1.0f, +1.0f, +1.0f);

	m_FrustumMBM.Initialize("Ocean Frustums");
	m_FrustumMBM.CreateModelContext(frustumVertexData, frustumAttributes, MeshBufferManager::ACCESS_TYPE::AT_STATIC);
}

void Ocean::SetupTextures ( const GlobalConfig& i_Config )
{
	m_OceanTM.Initialize("Ocean", i_Config);
	// perlin noise is used beginning with middle far to very far away, so we don't need to many levels of mipmaps. 5 levels suffice!
	m_OceanTM.Load2DTexture("../resources/textures/perlin_noise.dds", GL_REPEAT, GL_LINEAR, false, i_Config.TexUnit.Ocean.Surface.PerlinDisplacementMap, 5);// 0);
																																							// foam should be available only near to middle range from camera position, so 7 levels of mipmaps will suffice
	m_OceanTM.Load2DTexture("../resources/textures/foam_highres.dds", GL_REPEAT, GL_LINEAR, true, i_Config.TexUnit.Ocean.Surface.WavesFoamMap, 5);// 0);

	m_OceanTM.Load2DTexture("../resources/textures/sand_d.dds", GL_REPEAT, GL_LINEAR, true, i_Config.TexUnit.Ocean.Bottom.SandDiffuseMap, 3);

	m_OceanTM.Load2DTexture("../resources/textures/boat_foam.dds", GL_CLAMP_TO_EDGE, GL_LINEAR, true, i_Config.TexUnit.Ocean.Surface.BoatFoamMap, 3);

	m_OceanTM.Load2DTexture("../resources/textures/wake_normal_height.dds", GL_CLAMP_TO_EDGE, GL_LINEAR, false, i_Config.TexUnit.Ocean.Surface.KelvinWakeDispNormMap, 3);
	m_OceanTM.Load2DTexture("../resources/textures/wake_foam.dds", GL_CLAMP_TO_EDGE, GL_LINEAR, true, i_Config.TexUnit.Ocean.Surface.KelvinWakeFoamMap, 3);
}

void Ocean::Update ( const Camera& i_Camera, const glm::vec3& i_SunDirection, bool i_IsWireframeMode, bool i_IsFrustumVisible, float i_CrrTime )
{
	m_IsWireframeMode = i_IsWireframeMode;
	m_IsFrustumVisible = i_IsFrustumVisible;

	m_SunDirY = i_SunDirection.y;

	m_pCurrentCamera = &const_cast<Camera&>(i_Camera);


	UpdateOceanSurface(i_Camera, i_SunDirection, i_CrrTime);

	glm::mat4 bottomGridCorners;
	UpdateOceanBottom(i_Camera, i_SunDirection, bottomGridCorners);

	UpdateOceanBottomCaustics(bottomGridCorners, i_SunDirection);

	UpdateOceanBottomGodRays(i_Camera, i_SunDirection);

	UpdateDebugFrustum(i_Camera);
}

void Ocean::UpdateOceanSurface ( const Camera& i_Camera, const glm::vec3& i_SunDirection, float i_CrrTime )
{
	m_WaveProjector.Update(i_Camera);

	if (m_WaveProjector.IsPlaneWithinFrustum() || m_BottomProjector.IsPlaneWithinFrustum())
	{
		m_pFFTOceanPatch->EvaluateWaves(i_CrrTime);

		m_OceanSurfaceSM.UseProgram();

		//// Perlin Noise vars
		glm::vec2 perlinNoiseMovement;

		if (m_pFFTOceanPatch)
		{
			float perlinNoiseSpeed = m_PerlinNoiseSpeed * m_pFFTOceanPatch->GetWindSpeed();
			perlinNoiseMovement = glm::vec2(m_pFFTOceanPatch->GetWindDirectionX(), m_pFFTOceanPatch->GetWindDirectionZ()) * i_CrrTime * perlinNoiseSpeed;
		}

		if (m_SurfaceUseGridCorners)
		{
			glm::mat4 waveGridCorners = m_WaveProjector.ComputeGridCorners();

			m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_ProjectingMatrix")->second, 1, glm::value_ptr(waveGridCorners), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
		}
		else
		{
			m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_ProjectingMatrix")->second, 1, glm::value_ptr(m_WaveProjector.GetProjectingMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
		}

		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_PerlinNoiseData.Movement")->second, 1, glm::value_ptr(perlinNoiseMovement), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_2);
		if (m_pFFTOceanPatch)
		{
			m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_FFTOceanPatchData.WindSpeed")->second, m_pFFTOceanPatch->GetWindSpeed());
		}

		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_IsUnderWater")->second, m_WaveProjector.IsUnderMainPlane());

		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_SunData.Direction")->second, 1, glm::value_ptr(i_SunDirection), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_CrrTime")->second, i_CrrTime);
	}
}

void Ocean::UpdateOceanBottom ( const Camera& i_Camera, const glm::vec3& i_SunDirection, glm::mat4& o_BottomGridCorners )
{
	if (m_WaveProjector.IsUnderMainPlane())
	{
		m_BottomProjector.Update(i_Camera);

		if (m_BottomProjector.IsPlaneWithinFrustum())
		{
			m_OceanBottomSM.UseProgram();

			if (m_BottomUseGridCorners)
			{
				o_BottomGridCorners = m_BottomProjector.ComputeGridCorners();

				m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_ProjectingMatrix")->second, 1, glm::value_ptr(o_BottomGridCorners), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
			}
			else
			{
				m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_ProjectingMatrix")->second, 1, glm::value_ptr(m_BottomProjector.GetProjectingMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
			}
			m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_SunDirection")->second, 1, glm::value_ptr(i_SunDirection), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
		}
	}
}

void Ocean::UpdateOceanBottomCaustics ( const glm::mat4& i_BottomGridCorners, const glm::vec3& i_SunDirection )
{
	if (m_EnableBottomCaustics && m_WaveProjector.IsUnderMainPlane() && m_BottomProjector.IsPlaneWithinFrustum())
	{
		m_OceanCausticsSM.UseProgram();

		if (m_BottomUseGridCorners)
		{
			m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_BottomProjectingMatrix")->second, 1, glm::value_ptr(i_BottomGridCorners), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
		}
		else
		{
			m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_BottomProjectingMatrix")->second, 1, glm::value_ptr(m_BottomProjector.GetProjectingMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
		}

		m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_SunDirection")->second, 1, glm::value_ptr(i_SunDirection), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);			
	}
}

void Ocean::UpdateOceanBottomGodRays ( const Camera& i_Camera, const glm::vec3& i_SunDirection )
{
	if (m_EnableUnderWaterGodRays && m_WaveProjector.IsUnderMainPlane())
	{
		// Update godrays only when we're underwater and sun is not set
		if (m_WaveProjector.IsUnderMainPlane() && i_SunDirection.y > 0.0f)
		{
			// Occluder Update
			m_OceanOccluderSM.UseProgram();
			/////////////
			// translate the occluder to match intersection point on ocean plane along the light dir from viewer position
			glm::vec3 occluderPos;
			glm::vec3 camPos = i_Camera.GetPosition();
			HelperFunctions::PlaneIntersectRay(m_WaveProjector.GetPlane(), camPos, i_SunDirection, occluderPos);

			glm::mat4 PVM = i_Camera.GetProjectionViewMatrix() * glm::translate(glm::mat4(1.0f), occluderPos);

			m_OceanOccluderSM.SetUniform(m_OceanOccluderUniforms.find("u_WorldToClipMatrix")->second, 1, glm::value_ptr(PVM), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);

			m_OceanOccluderSM.SetUniform(m_OceanOccluderUniforms.find("u_SunDirY")->second, i_SunDirection.y);


			//// God Rays Update
			m_OceanGodRaysSM.UseProgram();

			//// calculte light dir in screen space
			int viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);

			glm::vec2 lightDirScreen;

			// http://www.codng.com/2011/02/gluunproject-for-iphoneios.html
			// transformation to clip space
			//glm::vec4 lightDirectionClip = i_Camera.GetProjectionViewMatrix() * glm::vec4(i_SunDirection, 1.0f);
			glm::vec4 lightDirectionClip = i_Camera.GetProjectionViewMatrix() * glm::vec4(occluderPos, 1.0f);

			// transformation to normalized device coordinates
			lightDirectionClip /= lightDirectionClip.w; // perspective division

														// viewport transformation
														// map (x, y) to [0, 1]
														// map (x, y) to viewport
			lightDirScreen.x = (lightDirectionClip.x * 0.5f + 0.5f) * viewport[2] + viewport[0];
			lightDirScreen.y = (lightDirectionClip.y * 0.5f + 0.5f) * viewport[3] + viewport[1];

			// map (x, y) to (u, v)
			lightDirScreen.x /= m_GodRaysMapWidth;
			lightDirScreen.y /= m_GodRaysMapHeight;

			m_OceanGodRaysSM.SetUniform(m_OceanGodRaysUniforms.find("u_GodRaysData.LightDirectionOnScreen")->second, 1, glm::value_ptr(lightDirScreen), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_2);
		}
	}
}

void Ocean::UpdateDebugFrustum ( const Camera& i_Camera )
{
	if (m_IsFrustumVisible)
	{
		m_FrustumSM.UseProgram();
		m_FrustumSM.SetUniform(m_FrustumUniforms.find("u_ViewClipToWorldMatrix")->second, 1, glm::value_ptr(i_Camera.GetInverseProjectionViewMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);

		if (m_WaveProjector.IsUnderMainPlane())
		{
			m_FrustumSM.SetUniform(m_FrustumUniforms.find("u_ProjectorClipToWorldMatrix")->second, 1, glm::value_ptr(m_BottomProjector.GetProjectingCamera().GetInverseProjectionViewMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
		}
		else
		{
			m_FrustumSM.SetUniform(m_FrustumUniforms.find("u_ProjectorClipToWorldMatrix")->second, 1, glm::value_ptr(m_WaveProjector.GetProjectingCamera().GetInverseProjectionViewMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
		}
	}
}

void Ocean::UpdateGrid ( unsigned short i_WindowWidth, unsigned short i_WindowHeight )
{
	if (m_GridType == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
	{
		SetupScreenSpaceGrid(i_WindowWidth, i_WindowHeight);
	}
}

void Ocean::UpdateBoatEffects ( const MotorBoat& i_MotorBoat )
{
	if (m_WaveProjector.IsPlaneWithinFrustum())
	{
		if (m_EnableBoatKelvinWake)
		{
			m_OceanSurfaceSM.UseProgram();

			m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_BoatKelvinWakeData.BoatPosition")->second, 1, glm::value_ptr(i_MotorBoat.GetKelvinWakeData().BoatPosition), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
			m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_BoatKelvinWakeData.WakePosition")->second, 1, glm::value_ptr(i_MotorBoat.GetKelvinWakeData().WakePosition), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
			m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_BoatKelvinWakeData.Amplitude")->second, i_MotorBoat.GetKelvinWakeData().Amplitude);
			m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_BoatKelvinWakeData.FoamAmount")->second, i_MotorBoat.GetKelvinWakeData().FoamAmount);
		}

		if (m_EnableBoatPropellerWash)
		{
			i_MotorBoat.BindPropellerWashTexture();
		}
	}
}

void Ocean::Render ( const Camera& i_CurrentViewingCamera )
{
	// necessary textures for ocean waves displacement, normals and foam
	if (m_pFFTOceanPatch)
	{
		m_pFFTOceanPatch->BindFFTWaveDataTexture();
		m_pFFTOceanPatch->BindNormalFoldingTexture();
	}

	RenderOceanSurface(i_CurrentViewingCamera);

	RenderOceanBottomCaustics(i_CurrentViewingCamera);

	RenderOceanBottom(i_CurrentViewingCamera);

	RenderOceanBottomGodRays();

	RenderDebugFrustum(i_CurrentViewingCamera);
}


void Ocean::RenderOceanSurface ( const Camera& i_CurrentViewingCamera )
{
	if (m_WaveProjector.IsUnderMainPlane() && m_GridType == CustomTypes::Ocean::GridType::GT_WORLD_SPACE)
	{
		glFrontFace(GL_CW);
	}

	if (m_WaveProjector.IsPlaneWithinFrustum())
	{
		m_OceanSurfaceSM.UseProgram();

		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_WorldToCameraMatrix")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetViewMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);

		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_WorldToClipMatrix")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetProjectionViewMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);

		if (m_GridType == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
		{
			m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_ClipToCameraMatrix")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetInverseProjectionMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
			m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_CameraToWorldMatrix")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetInverseViewMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
		}

		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_CameraPosition")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetPosition()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

		m_OceanSurfaceMBM.BindModelContext();

		glDrawElements(m_IsWireframeMode ? GL_LINE_STRIP : GL_TRIANGLE_STRIP, m_GridIndexCount, GL_UNSIGNED_INT, nullptr);

		m_OceanSurfaceMBM.UnBindModelContext();
	}

	if (m_WaveProjector.IsUnderMainPlane() && m_GridType == CustomTypes::Ocean::GridType::GT_WORLD_SPACE)
	{
		glFrontFace(GL_CCW);
	}
}

void Ocean::RenderOceanBottom ( const Camera& i_CurrentViewingCamera )
{
	glm::ivec4 oldViewport, newViewport;

	// save current viewport
	glGetIntegerv(GL_VIEWPORT, &oldViewport[0]);

	if (m_WaveProjector.IsUnderMainPlane() && m_BottomProjector.IsPlaneWithinFrustum())
	{
		if (m_EnableBottomCaustics)
		{
			m_OceanCausticsFBM.BindColorAttachmentByIndex(0, true);
		}

		m_OceanBottomSM.UseProgram();

		m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_WorldToCameraMatrix")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetViewMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);

		m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_WorldToClipMatrix")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetProjectionViewMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);

		if (m_GridType == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
		{
			m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_ClipToCameraMatrix")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetInverseProjectionMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
			m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_CameraToWorldMatrix")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetInverseViewMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
			m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_CameraPosition")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetPosition()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
		}

		m_OceanBottomMBM.BindModelContext();

		glDrawElements(m_IsWireframeMode ? GL_LINE_STRIP : GL_TRIANGLE_STRIP, m_GridIndexCount, GL_UNSIGNED_INT, nullptr);

		m_OceanBottomMBM.UnBindModelContext();
	}
}

void Ocean::RenderOceanBottomCaustics ( const Camera& i_CurrentViewingCamera )
{
	if (m_EnableBottomCaustics && m_WaveProjector.IsUnderMainPlane() && m_BottomProjector.IsPlaneWithinFrustum())
	{
		glm::ivec4 oldViewport, newViewport;

		// save current viewport
		glGetIntegerv(GL_VIEWPORT, &oldViewport[0]);

		newViewport = glm::ivec4(0, 0, m_CausticsMapSize, m_CausticsMapSize);

		// set new viewport
		if (glm::any(glm::notEqual(newViewport, oldViewport)))
		{
			glViewport(newViewport.x, newViewport.y, newViewport.z, newViewport.w);
		}

		m_OceanCausticsFBM.Bind();

		glClear(GL_COLOR_BUFFER_BIT);

		m_OceanCausticsSM.UseProgram();

		m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_WorldToClipMatrix")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetProjectionViewMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);

		if (m_GridType == CustomTypes::Ocean::GridType::GT_SCREEN_SPACE)
		{
			m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_ClipToCameraMatrix")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetInverseProjectionMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
			m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_CameraToWorldMatrix")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetInverseViewMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
			m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_CameraPosition")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetPosition()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
		}

		m_OceanCausticsMBM.BindModelContext();

		glDrawElements(m_IsWireframeMode ? GL_LINE_STRIP : GL_TRIANGLE_STRIP, m_GridIndexCount, GL_UNSIGNED_INT, nullptr);

		m_OceanCausticsMBM.UnBindModelContext();

		m_OceanCausticsFBM.UnBind();

		// restore viewport
		if (glm::any(glm::notEqual(newViewport, oldViewport)))
		{
			glViewport(oldViewport.x, oldViewport.y, oldViewport.z, oldViewport.w);
		}
	}

}

void Ocean::RenderOceanBottomGodRays ( void )
{
	if (m_EnableUnderWaterGodRays && m_WaveProjector.IsUnderMainPlane() && m_SunDirY > 0.0f)
	{
		// Render godrays only when we're underwater and sun is not set

		glm::ivec4 oldViewport, newViewport;

		// save current viewport
		glGetIntegerv(GL_VIEWPORT, &oldViewport[0]);

		newViewport = glm::ivec4(0, 0, m_GodRaysMapWidth, m_GodRaysMapHeight);

		// set new viewport
		if (glm::any(glm::notEqual(newViewport, oldViewport)))
		{
			glViewport(newViewport.x, newViewport.y, newViewport.z, newViewport.w);
		}

		m_OceanGodRaysFBM.Bind();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render the occluders
		m_OceanOccluderSM.UseProgram();

		m_OceanOccluderMBM.BindModelContext();

		glFrontFace(GL_CW);

		glDrawElements(m_IsWireframeMode ? GL_LINE_STRIP : GL_TRIANGLE_STRIP, m_OccluderIndexCount, GL_UNSIGNED_INT, nullptr);

		glFrontFace(GL_CCW);

		m_OceanOccluderMBM.UnBindModelContext();

		m_OceanGodRaysFBM.UnBind();

		/////////////////////
		// Add the light scattering effect

		// bind occluder texture
		m_OceanGodRaysFBM.BindColorAttachmentByIndex(0);

		m_OceanGodRaysSM.UseProgram();

		// blend
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glEnable(GL_BLEND);

		m_OceanGodRaysFBM.RenderToQuad();

		glDisable(GL_BLEND);
	}
}

void Ocean::RenderDebugFrustum ( const Camera& i_CurrentViewingCamera )
{
	if (m_IsFrustumVisible)
	{
		m_FrustumSM.UseProgram();

		m_FrustumSM.SetUniform(m_FrustumUniforms.find("u_WorldToClipMatrix")->second, 1, glm::value_ptr(i_CurrentViewingCamera.GetProjectionViewMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);

		m_FrustumMBM.BindModelContext();

		// Camera frustum - black or white color
		m_FrustumSM.SetUniform(m_FrustumUniforms.find("u_IsViewFrustum")->second, 1);
		if (m_WaveProjector.IsUnderMainPlane())
		{
			m_FrustumSM.SetUniform(m_FrustumUniforms.find("u_Color")->second, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
		}
		else
		{
			m_FrustumSM.SetUniform(m_FrustumUniforms.find("u_Color")->second, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
		}
		glDrawArrays(GL_LINES, 0, 24);

		// Projector frustum - red or green color
		m_FrustumSM.SetUniform(m_FrustumUniforms.find("u_IsViewFrustum")->second, 0);
		if (m_WaveProjector.IsUnderMainPlane())
		{
			m_FrustumSM.SetUniform(m_FrustumUniforms.find("u_Color")->second, 1, glm::value_ptr(glm::vec3(0.0f, 1.0f, 0.0f)), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
		}
		else
		{
			m_FrustumSM.SetUniform(m_FrustumUniforms.find("u_Color")->second, 1, glm::value_ptr(glm::vec3(1.0f, 0.0f, 0.0f)), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
		}
		glDrawArrays(GL_LINES, 24, 24);

		m_FrustumMBM.UnBindModelContext();
	}
}

float Ocean::ComputeWaterHeightAt ( const glm::vec2& i_XZ )
{
	float val = 0.0f;

	if (m_pFFTOceanPatch)
	{
		val = m_pFFTOceanPatch->ComputeWaterHeightAt(i_XZ);
	}

	return val;
}

float Ocean::ComputeAverageWaterHeightAt ( const glm::vec2& i_XZ, const glm::ivec2& i_Zone )
{
	return 0;
}

float Ocean::GetWaveAmplitude ( void ) const
{
	float val = 0.0f;

	if (m_pFFTOceanPatch)
	{
		val = m_pFFTOceanPatch->GetWaveAmplitude();
	}

	return val;
}

unsigned short Ocean::GetPatchSize ( void ) const
{
	unsigned short val = 0;

	if (m_pFFTOceanPatch)
	{
		val = m_pFFTOceanPatch->GetPatchSize();
	}

	return val;
}

float Ocean::GetWindSpeed ( void ) const
{
	float val = 0.0f;

	if (m_pFFTOceanPatch)
	{
		val = m_pFFTOceanPatch->GetWindSpeed();
	}

	return val;
}

float Ocean::GetWindDirectionX ( void ) const
{
	float val = 0.0f;

	if (m_pFFTOceanPatch)
	{
		val = m_pFFTOceanPatch->GetWindDirectionX();
	}

	return val;
}

float Ocean::GetWindDirectionZ ( void ) const
{
	float val = 0.0f;

	if (m_pFFTOceanPatch)
	{
		val = m_pFFTOceanPatch->GetWindDirectionZ();
	}

	return val;
}

glm::vec3 Ocean::GetWindDir(void) const
{
	glm::vec3 vec;

	if (m_pFFTOceanPatch && m_pCurrentCamera)
	{
		glm::vec3 v = m_pFFTOceanPatch->GetWindDir();

		float cameraYaw = m_pCurrentCamera->GetYaw();
		float angle = - cameraYaw;

		vec = glm::rotateY(v, angle);
	}

	return vec;
}

float Ocean::GetOpposingWavesFactor ( void ) const
{
	float val = 0.0f;

	if (m_pFFTOceanPatch)
	{
		val = m_pFFTOceanPatch->GetOpposingWavesFactor();
	}

	return val;
}

float Ocean::GetVerySmallWavesFactor ( void ) const
{
	float val = 0.0f;

	if (m_pFFTOceanPatch)
	{
		val = m_pFFTOceanPatch->GetVerySmallWavesFactor();
	}

	return val;
}

float Ocean::GetChoppyScale ( void ) const
{
	float val = 0.0f;

	if (m_pFFTOceanPatch)
	{
		val = m_pFFTOceanPatch->GetChoppyScale();
	}

	return val;
}

float Ocean::GetTileScale ( void ) const
{
	float val = 0.0f;

	if (m_pFFTOceanPatch)
	{
		val = m_pFFTOceanPatch->GetTileScale();
	}

	return val;
}

unsigned short Ocean::GetGodRaysNumberOfSamples ( void ) const
{
	return m_UnderWaterGodRaysData.NumberOfSamples;
}

float Ocean::GetGodRaysExposure ( void ) const
{
	return m_UnderWaterGodRaysData.Exposure;
}

float Ocean::GetGodRaysDecay ( void ) const
{
	return m_UnderWaterGodRaysData.Decay;
}

float Ocean::GetGodRaysDensity ( void ) const
{
	return m_UnderWaterGodRaysData.Density;
}

float Ocean::GetGodRaysWeight ( void ) const
{
	return m_UnderWaterGodRaysData.Weight;
}

bool Ocean::IsUnderWater ( void ) const
{
	return m_WaveProjector.IsUnderMainPlane();
}

void Ocean::SetWaveAmplitude ( float i_WaveAmplitude )
{
	if (m_pFFTOceanPatch)
	{
		m_pFFTOceanPatch->SetWaveAmplitude(i_WaveAmplitude);

		m_OceanSurfaceSM.UseProgram();
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_FFTOceanPatchData.WaveAmplitude")->second, i_WaveAmplitude);
	}
}

void Ocean::SetPatchSize ( unsigned short i_PatchSize )
{
	if (m_pFFTOceanPatch)
	{
		m_pFFTOceanPatch->SetPatchSize(i_PatchSize);

		m_OceanSurfaceSM.UseProgram();
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_FFTOceanPatchData.PatchSize")->second, static_cast<float>(i_PatchSize));

		m_OceanCausticsSM.UseProgram();
		m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_PatchSize")->second, static_cast<float>(i_PatchSize));
	}
}

void Ocean::SetWindSpeed ( float i_WindSpeed )
{
	if (m_pFFTOceanPatch)
	{
		m_pFFTOceanPatch->SetWindSpeed(i_WindSpeed);

		m_OceanSurfaceSM.UseProgram();
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_FFTOceanPatchData.WindSpeed")->second, static_cast<float>(i_WindSpeed));
	}
}

void Ocean::SetWindDirectionX ( float i_WindDirectionX )
{
	if (m_pFFTOceanPatch)
	{
		m_pFFTOceanPatch->SetWindDirectionX(i_WindDirectionX);
	}
}

void Ocean::SetWindDirectionZ ( float i_WindDirectionZ )
{
	if (m_pFFTOceanPatch)
	{
		m_pFFTOceanPatch->SetWindDirectionZ(i_WindDirectionZ);
	}
}

void Ocean::SetOpposingWavesFactor ( float i_OpposingWavesFactor )
{
	if (m_pFFTOceanPatch)
	{
		m_pFFTOceanPatch->SetOpposingWavesFactor(i_OpposingWavesFactor);
	}
}

void Ocean::SetVerySmallWavesFactor ( float i_VerySmallWavesFactor )
{
	if (m_pFFTOceanPatch)
	{
		m_pFFTOceanPatch->SetVerySmallWavesFactor(i_VerySmallWavesFactor);
	}
}

void Ocean::SetChoppyScale ( float i_ChoppyScale )
{
	if (m_pFFTOceanPatch)
	{
		m_pFFTOceanPatch->SetChoppyScale(i_ChoppyScale);

		m_OceanSurfaceSM.UseProgram();
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_FFTOceanPatchData.ChoppyScale")->second, i_ChoppyScale);

		m_OceanCausticsSM.UseProgram();
		m_OceanCausticsSM.SetUniform(m_OceanCausticsUniforms.find("u_ChoppyScale")->second, static_cast<float>(i_ChoppyScale));
	}
}

void Ocean::SetTileScale ( const float i_TileScale )
{
	if (m_pFFTOceanPatch)
	{
		m_pFFTOceanPatch->SetTileScale(i_TileScale);

		m_OceanSurfaceSM.UseProgram();
		m_OceanSurfaceSM.SetUniform(m_OceanSurfaceUniforms.find("u_FFTOceanPatchData.TileScale")->second, m_pFFTOceanPatch->GetTileScale());

		m_OceanBottomSM.UseProgram();
		m_OceanBottomSM.SetUniform(m_OceanBottomUniforms.find("u_TileScale")->second, m_pFFTOceanPatch->GetTileScale());
	}
}

void Ocean::SetGodRaysNumberOfSamples ( unsigned short i_NumberOfSamples )
{
	m_UnderWaterGodRaysData.NumberOfSamples = i_NumberOfSamples;

	m_OceanGodRaysSM.UseProgram();
	m_OceanGodRaysSM.SetUniform(m_OceanGodRaysUniforms.find("u_GodRaysData.NumberOfSamples")->second, m_UnderWaterGodRaysData.NumberOfSamples);
}

void Ocean::SetGodRaysExposure ( float i_Exposure )
{
	m_UnderWaterGodRaysData.Exposure = i_Exposure;

	m_OceanGodRaysSM.UseProgram();
	m_OceanGodRaysSM.SetUniform(m_OceanGodRaysUniforms.find("u_GodRaysData.Exposure")->second, m_UnderWaterGodRaysData.Exposure);
}

void Ocean::SetGodRaysDecay ( float i_Decay )
{
	m_UnderWaterGodRaysData.Decay = i_Decay;

	m_OceanGodRaysSM.UseProgram();
	m_OceanGodRaysSM.SetUniform(m_OceanGodRaysUniforms.find("u_GodRaysData.Decay")->second, m_UnderWaterGodRaysData.Decay);
}

void Ocean::SetGodRaysDensity ( float i_Density )
{
	m_UnderWaterGodRaysData.Density = i_Density;

	m_OceanGodRaysSM.UseProgram();
	m_OceanGodRaysSM.SetUniform(m_OceanGodRaysUniforms.find("u_GodRaysData.Density")->second, m_UnderWaterGodRaysData.Density);
}

void Ocean::SetGodRaysWeight ( float i_Weight )
{
	m_UnderWaterGodRaysData.Weight = i_Weight;

	m_OceanGodRaysSM.UseProgram();
	m_OceanGodRaysSM.SetUniform(m_OceanGodRaysUniforms.find("u_GodRaysData.Weight")->second, m_UnderWaterGodRaysData.Weight);
}