/* Author: BAIRAC MIHAI */

#include "ScatteringSkyModel.h"
#include "CommonHeaders.h"
#include "GLConfig.h"
// glm::mat4 comes from the header
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat3x3.hpp"
#include "glm/exponential.hpp" //pow()
#include "glm/trigonometric.hpp" //sin(), cos()
#include "glm/gtc/matrix_transform.hpp" //perspective(), ortho(), lookAt()
#include "glm/gtc/type_ptr.hpp" //value_ptr()
#include "glm/gtc/constants.hpp" //pi(), two_pi()
#include "PhysicsConstants.h"
#include "GlobalConfig.h"


ScatteringSkyModel::ScatteringSkyModel ( void )
	: m_AtmosphereInnerRadius(0.0f), m_AreCloudsEnabled(false)
{
	LOG("ScatteringSkyModel successfully created!");
}

ScatteringSkyModel::ScatteringSkyModel ( const GlobalConfig& i_Config )
	: m_AtmosphereInnerRadius(0.0f), m_AreCloudsEnabled(false)
{
	Initialize(i_Config);
}

ScatteringSkyModel::~ScatteringSkyModel ( void )
{
	Destroy();
}

void ScatteringSkyModel::Destroy ( void )
{
	LOG("ScatteringSkyModel successfully destroyed!");
}

void ScatteringSkyModel::Initialize ( const GlobalConfig& i_Config )
{
	BaseSkyModel::Initialize(i_Config);

	m_AtmosphereInnerRadius = i_Config.Scene.Sky.Model.Scattering.Atmosphere.InnerRadius;

	m_SunData.AllowChangeDirWithMouse = i_Config.Scene.Sky.Model.Scattering.Sun.AllowChangeDirWithMouse;
	m_SunData.IsDynamic = i_Config.Scene.Sky.Model.Scattering.Sun.IsDynamic;
	m_SunData.MoveFactor = i_Config.Scene.Sky.Model.Scattering.Sun.MoveFactor;

	m_AreCloudsEnabled = i_Config.Scene.Sky.Model.Scattering.Clouds.Enabled;

	m_CloudsData.Octaves = i_Config.Scene.Sky.Model.Scattering.Clouds.Octaves;
	m_CloudsData.Lacunarity = i_Config.Scene.Sky.Model.Scattering.Clouds.Lacunarity;
	m_CloudsData.Gain = i_Config.Scene.Sky.Model.Scattering.Clouds.Gain;
	m_CloudsData.ScaleFactor = i_Config.Scene.Sky.Model.Scattering.Clouds.ScaleFactor;

	//// Setup Sky
	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> skyAttributes;
	SetupSkyShaders(i_Config, skyAttributes);
	SetupSkyGeometry(i_Config, skyAttributes);

	//// Setup Clouds
	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> cloudsAttributes;
	SetupCloudsShaders(i_Config, cloudsAttributes);
	SetupCloudsGeometry(i_Config, cloudsAttributes);

	LOG("ScatteringSkyModel successfully created!");
}

void ScatteringSkyModel::SetupSkyShaders ( const GlobalConfig& i_Config, std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& o_Attributes )
{
	float Kr4PI = 4.0f * glm::pi<float>() * i_Config.Scene.Sky.Model.Scattering.Atmosphere.RayleighScatteringConstant;
	float Km4PI = 4.0f * glm::pi<float>() * i_Config.Scene.Sky.Model.Scattering.Atmosphere.MieScatteringConstant;
	float KrESun = i_Config.Scene.Sky.Model.Scattering.Atmosphere.RayleighScatteringConstant * i_Config.Scene.Sky.Model.Scattering.Atmosphere.SunBrightnessConstant;
	float KmESun = i_Config.Scene.Sky.Model.Scattering.Atmosphere.MieScatteringConstant * i_Config.Scene.Sky.Model.Scattering.Atmosphere.SunBrightnessConstant;

	float g = - 0.1f * PhysicsConstants::kG;

	float scaleDepth = i_Config.Scene.Sky.Model.Scattering.Atmosphere.OuterRadius - i_Config.Scene.Sky.Model.Scattering.Atmosphere.InnerRadius;
	float scale = 1.0f / scaleDepth;
	float scaleOverScaleDepth = scale / scaleDepth;

	glm::vec3 wavelength4;
	wavelength4.x = glm::pow(i_Config.Scene.Sky.Model.Scattering.Atmosphere.WaveLength.x, 4.0f);
	wavelength4.y = glm::pow(i_Config.Scene.Sky.Model.Scattering.Atmosphere.WaveLength.y, 4.0f);
	wavelength4.z = glm::pow(i_Config.Scene.Sky.Model.Scattering.Atmosphere.WaveLength.z, 4.0f);

	m_SM.Initialize("ScatteringSkyModel");
	m_SM.BuildRenderingProgram("resources/shaders/ScatteringSkyModel.vert.glsl", "resources/shaders/ScatteringSkyModel.frag.glsl", i_Config);

	m_SM.UseProgram();

	o_Attributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_SM.GetAttributeLocation("a_position");

	m_Uniforms["u_HDRExposure"] = m_SM.GetUniformLocation("u_HDRExposure");
	m_SM.SetUniform(m_Uniforms.find("u_HDRExposure")->second, i_Config.Rendering.HDR.Exposure);
	m_Uniforms["u_ApplyHDR"] = m_SM.GetUniformLocation("u_ApplyHDR");
	m_SM.SetUniform(m_Uniforms.find("u_ApplyHDR")->second, true);

	m_Uniforms["u_IsReflMode"] = m_SM.GetUniformLocation("u_IsReflMode");
	m_SM.SetUniform(m_Uniforms.find("u_IsReflMode")->second, false);
	m_Uniforms["u_IsUnderWater"] = m_SM.GetUniformLocation("u_IsUnderWater");
	m_SM.SetUniform(m_Uniforms.find("u_IsUnderWater")->second, false);

	m_Uniforms["u_UnderWaterColor"] = m_SM.GetUniformLocation("u_UnderWaterColor");
	m_SM.SetUniform(m_Uniforms.find("u_UnderWaterColor")->second, 1, glm::value_ptr(i_Config.Scene.Sky.UnderWaterColor), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

	m_Uniforms["u_UnderWaterFogColor"] = m_SM.GetUniformLocation("u_UnderWaterFogColor");
	m_SM.SetUniform(m_Uniforms.find("u_UnderWaterFogColor")->second, 1, glm::value_ptr(i_Config.Scene.Ocean.UnderWater.Fog.Color), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

	m_Uniforms["u_WorldToClipMatrix"] = m_SM.GetUniformLocation("u_WorldToClipMatrix");
	m_Uniforms["u_ObjectToWorldMatrix"] = m_SM.GetUniformLocation("u_ObjectToWorldMatrix");

	m_Uniforms["u_CameraPosition"] = m_SM.GetUniformLocation("u_CameraPosition");
	m_Uniforms["u_SunDirection"] = m_SM.GetUniformLocation("u_SunDirection");
	SetSunDirection(i_Config.Scene.Sky.Model.Scattering.Sun.InitialPhi, i_Config.Scene.Sky.Model.Scattering.Sun.InitialTheta);

	m_Uniforms["u_ScatteringData.SampleCount"] = m_SM.GetUniformLocation("u_ScatteringData.SampleCount");
	m_SM.SetUniform(m_Uniforms.find("u_ScatteringData.SampleCount")->second, i_Config.Scene.Sky.Model.Scattering.Atmosphere.SampleCount);
	m_Uniforms["u_ScatteringData.EarthRadius"] = m_SM.GetUniformLocation("u_ScatteringData.EarthRadius");
	m_SM.SetUniform(m_Uniforms.find("u_ScatteringData.EarthRadius")->second, PhysicsConstants::kEarthRadius);
	m_Uniforms["u_ScatteringData.InvWavelength"] = m_SM.GetUniformLocation("u_ScatteringData.InvWavelength");
	m_SM.SetUniform(m_Uniforms.find("u_ScatteringData.InvWavelength")->second, 1, glm::value_ptr(1.0f / wavelength4), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
	m_Uniforms["u_ScatteringData.InnerRadius"] = m_SM.GetUniformLocation("u_ScatteringData.InnerRadius");
	m_SM.SetUniform(m_Uniforms.find("u_ScatteringData.InnerRadius")->second, i_Config.Scene.Sky.Model.Scattering.Atmosphere.InnerRadius);
	m_Uniforms["u_ScatteringData.KrESun"] = m_SM.GetUniformLocation("u_ScatteringData.KrESun");
	m_SM.SetUniform(m_Uniforms.find("u_ScatteringData.KrESun")->second, KrESun);
	m_Uniforms["u_ScatteringData.KmESun"] = m_SM.GetUniformLocation("u_ScatteringData.KmESun");
	m_SM.SetUniform(m_Uniforms.find("u_ScatteringData.KmESun")->second, KmESun);
	m_Uniforms["u_ScatteringData.Kr4PI"] = m_SM.GetUniformLocation("u_ScatteringData.Kr4PI");
	m_SM.SetUniform(m_Uniforms.find("u_ScatteringData.Kr4PI")->second, Kr4PI);
	m_Uniforms["u_ScatteringData.Km4PI"] = m_SM.GetUniformLocation("u_ScatteringData.Km4PI");
	m_SM.SetUniform(m_Uniforms.find("u_ScatteringData.Km4PI")->second, Km4PI);
	m_Uniforms["u_ScatteringData.Scale"] = m_SM.GetUniformLocation("u_ScatteringData.Scale");
	m_SM.SetUniform(m_Uniforms.find("u_ScatteringData.Scale")->second, scale);
	m_Uniforms["u_ScatteringData.ScaleDepth"] = m_SM.GetUniformLocation("u_ScatteringData.ScaleDepth");
	m_SM.SetUniform(m_Uniforms.find("u_ScatteringData.ScaleDepth")->second, scaleDepth);
	m_Uniforms["u_ScatteringData.ScaleOverScaleDepth"] = m_SM.GetUniformLocation("u_ScatteringData.ScaleOverScaleDepth");
	m_SM.SetUniform(m_Uniforms.find("u_ScatteringData.ScaleOverScaleDepth")->second, scaleOverScaleDepth);

	m_Uniforms["u_g"] = m_SM.GetUniformLocation("u_g");
	m_SM.SetUniform(m_Uniforms.find("u_g")->second, g);
	m_Uniforms["u_yOffset"] = m_SM.GetUniformLocation("u_yOffset");
	m_SM.SetUniform(m_Uniforms.find("u_yOffset")->second, i_Config.Scene.Sky.Model.Scattering.Atmosphere.AltitudeOffset);

	m_SM.UnUseProgram();
}

void ScatteringSkyModel::SetupCloudsShaders ( const GlobalConfig& i_Config, std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& o_Attributes )
{
	m_CloudsSM.Initialize("ScatteringSkyModel Clouds");
	m_CloudsSM.BuildRenderingProgram("resources/shaders/ScatteringSkyModelClouds.vert.glsl", "resources/shaders/ScatteringSkyModelClouds.frag.glsl", i_Config);

	m_CloudsSM.UseProgram();

	o_Attributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_CloudsSM.GetAttributeLocation("a_position");
	o_Attributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV] = m_CloudsSM.GetAttributeLocation("a_uv");

	m_CloudsUniforms["u_HDRExposure"] = m_CloudsSM.GetUniformLocation("u_HDRExposure");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_HDRExposure")->second, i_Config.Rendering.HDR.Exposure);
	m_CloudsUniforms["u_ApplyHDR"] = m_CloudsSM.GetUniformLocation("u_ApplyHDR");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_ApplyHDR")->second, true);

	m_CloudsUniforms["u_WorldToClipMatrix"] = m_CloudsSM.GetUniformLocation("u_WorldToClipMatrix");
	m_CloudsUniforms["u_ObjectToWorldMatrix"] = m_CloudsSM.GetUniformLocation("u_ObjectToWorldMatrix");

	m_CloudsUniforms["u_SunDirY"] = m_CloudsSM.GetUniformLocation("u_SunDirY");

	m_CloudsUniforms["u_CloudsData.Octaves"] = m_CloudsSM.GetUniformLocation("u_CloudsData.Octaves");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Octaves")->second, m_CloudsData.Octaves);
	m_CloudsUniforms["u_CloudsData.Lacunarity"] = m_CloudsSM.GetUniformLocation("u_CloudsData.Lacunarity");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Lacunarity")->second, m_CloudsData.Lacunarity);
	m_CloudsUniforms["u_CloudsData.Gain"] = m_CloudsSM.GetUniformLocation("u_CloudsData.Gain");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Gain")->second, m_CloudsData.Gain);
	m_CloudsUniforms["u_CloudsData.ScaleFactor"] = m_CloudsSM.GetUniformLocation("u_CloudsData.ScaleFactor");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.ScaleFactor")->second, m_CloudsData.ScaleFactor);

	m_CloudsSM.UnUseProgram();
}

void ScatteringSkyModel::SetupSkyGeometry ( const GlobalConfig& i_Config, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_Attributes )
{
	unsigned int geometrySliceCount = i_Config.Scene.Sky.Model.Scattering.Atmosphere.GeometrySliceCount;

	unsigned int numberParallels = geometrySliceCount / 2;

	unsigned int numberVertices = (numberParallels + 1) * (geometrySliceCount + 1);
	unsigned int numberIndices = numberParallels * geometrySliceCount * 6;

	float angleStep = (glm::two_pi<float>()) / ((float)geometrySliceCount);

	////
	std::vector<MeshBufferManager::VertexData> vertexData;
	vertexData.resize(numberVertices);

	for (size_t i = 0; i < numberParallels + 1; i++)
	{
		for (size_t j = 0; j < geometrySliceCount + 1; j++)
		{
			unsigned int index = i * (geometrySliceCount + 1) + j;

			vertexData[index].position.x = PhysicsConstants::kEarthRadius * glm::sin(angleStep * (float)i) * glm::sin(angleStep * (float)j);
			vertexData[index].position.y = PhysicsConstants::kEarthRadius * glm::cos(angleStep * (float)i) + i_Config.Scene.Sky.Model.Scattering.Atmosphere.AltitudeOffset;
			vertexData[index].position.z = PhysicsConstants::kEarthRadius * glm::sin(angleStep * (float)i) * glm::cos(angleStep * (float)j);
		}
	}

	////
	std::vector<unsigned int> indices;
	indices.resize(numberIndices);

	unsigned int indexIndices = 0;
	for (size_t i = 0; i < numberParallels; i++)
	{
		for (size_t j = 0; j < geometrySliceCount; j++)
		{
			// CW winding
			indices[indexIndices++] = i * (geometrySliceCount + 1) + j;
			indices[indexIndices++] = (i + 1) * (geometrySliceCount + 1) + (j + 1);
			indices[indexIndices++] = (i + 1) * (geometrySliceCount + 1) + j;

			indices[indexIndices++] = i * (geometrySliceCount + 1) + j;
			indices[indexIndices++] = i * (geometrySliceCount + 1) + (j + 1);
			indices[indexIndices++] = (i + 1) * (geometrySliceCount + 1) + (j + 1);
		}
	}

	m_IndexCount = indices.size();

	m_MBM.Initialize("ScatteringSkyModel Sky");
	m_MBM.CreateModelContext(vertexData, indices, i_Attributes, MeshBufferManager::ACCESS_TYPE::AT_STATIC);
}

void ScatteringSkyModel::SetupCloudsGeometry ( const GlobalConfig& i_Config, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_Attributes )
{
	float offset = i_Config.Scene.Sky.Model.Scattering.Clouds.Offset;
	float altitude = i_Config.Scene.Sky.Model.Scattering.Clouds.Altitude;

	/////////// CLOUDS ////////
	//// geometry setup
	std::vector<MeshBufferManager::VertexData> cloudVertexData;
	cloudVertexData.resize(4);

	// CW winding
	cloudVertexData[0].position = glm::vec3(-offset, altitude, -offset); //0
	cloudVertexData[0].uv = glm::vec2(0.0f, 0.0f);
	cloudVertexData[1].position = glm::vec3(offset, altitude, -offset); //1
	cloudVertexData[1].uv = glm::vec2(1.0f, 0.0f);
	cloudVertexData[2].position = glm::vec3(-offset, altitude, offset); //3
	cloudVertexData[2].uv = glm::vec2(0.0f, 1.0f);
	cloudVertexData[3].position = glm::vec3(offset, altitude, offset); //2
	cloudVertexData[3].uv = glm::vec2(1.0f, 1.0f);

	m_CloudsMBM.Initialize("ScatteringSkyModel Clouds");
	m_CloudsMBM.CreateModelContext(cloudVertexData, i_Attributes, MeshBufferManager::ACCESS_TYPE::AT_STATIC);
}

void ScatteringSkyModel::Update ( const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	UpdateInternal(glm::mat4(1.0f), true, i_Camera, false, i_IsUnderWater, i_IsWireframeMode, i_CrrTime);
}

void ScatteringSkyModel::UpdateReflected ( const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	UpdateInternal(i_ScaleMatrix, false, i_Camera, true, false, i_IsWireframeMode, i_CrrTime);
}

void ScatteringSkyModel::UpdateRefracted ( const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	UpdateInternal(i_ScaleMatrix, false, i_Camera, false, false, i_IsWireframeMode, i_CrrTime);
}

void ScatteringSkyModel::UpdateInternal ( const glm::mat4& i_ModelMatrix, bool i_ApplyHDR, const Camera& i_Camera, bool i_IsReflMode, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	m_IsWireframeMode = i_IsWireframeMode;

	BaseSkyModel::UpdateSunPosition(i_CrrTime);

	////////////////////////////
	// remove the translation matrix from the view matrix
	glm::mat4 PV = i_Camera.GetProjectionMatrix() * glm::mat4(glm::mat3(i_Camera.GetViewMatrix()));

	glm::vec3 vecCamera = i_Camera.GetPosition();
	vecCamera /= PhysicsConstants::kEarthRadius;
	vecCamera.y += m_AtmosphereInnerRadius;
	vecCamera.x = 0;
	vecCamera.z = 0;

	////////// SKY ///////
	m_SM.UseProgram();
	m_SM.SetUniform(m_Uniforms.find("u_ApplyHDR")->second, i_ApplyHDR);

	m_SM.SetUniform(m_Uniforms.find("u_CameraPosition")->second, 1, glm::value_ptr(vecCamera), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

	m_SM.SetUniform(m_Uniforms.find("u_IsReflMode")->second, i_IsReflMode);
	m_SM.SetUniform(m_Uniforms.find("u_IsUnderWater")->second, i_IsUnderWater);

	m_SM.SetUniform(m_Uniforms.find("u_WorldToClipMatrix")->second, 1, glm::value_ptr(PV), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
	m_SM.SetUniform(m_Uniforms.find("u_ObjectToWorldMatrix")->second, 1, glm::value_ptr(i_ModelMatrix), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);

	///////// CLOUDS /////////
	if (m_AreCloudsEnabled)
	{
		m_CloudsSM.UseProgram();
		m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_ApplyHDR")->second, i_ApplyHDR);
		m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_SunDirY")->second, m_SunData.Direction.y);

		m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_WorldToClipMatrix")->second, 1, glm::value_ptr(PV), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
		m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_ObjectToWorldMatrix")->second, 1, glm::value_ptr(i_ModelMatrix), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
	}
}

void ScatteringSkyModel::Render ( void )
{
	RenderInternal();
}

void ScatteringSkyModel::RenderReflected ( void )
{
	// revert winding for correct mirror like reflection!
	glFrontFace(GL_CW);

	RenderInternal();

	// restore winding
	glFrontFace(GL_CCW);
}

void ScatteringSkyModel::RenderRefracted ( void )
{
	RenderInternal();
}

void ScatteringSkyModel::RenderInternal ( void )
{
	glDepthFunc(GL_LEQUAL);

	//////////// RENDER SKY /////////
	m_SM.UseProgram();
	
	m_MBM.BindModelContext();
	glDrawElements(m_IsWireframeMode ? GL_LINES : GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
	m_MBM.UnBindModelContext();

	glDepthFunc(GL_LESS);

	/////////// RENDER CLOUDS //////////
	if (m_AreCloudsEnabled)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		m_CloudsSM.UseProgram();

		m_CloudsMBM.BindModelContext();
		glDrawArrays(m_IsWireframeMode ? GL_LINE_STRIP : GL_TRIANGLE_STRIP, 0, 4);
		m_CloudsMBM.UnBindModelContext();

		glDisable(GL_BLEND);
	}
}

void ScatteringSkyModel::SetSunDirection ( float i_Phi, float i_Theta )
{
	//////
	BaseSkyModel::SetSunDirection(i_Phi, i_Theta);

	m_SM.UseProgram();
	m_SM.SetUniform(m_Uniforms.find("u_SunDirection")->second, 1, glm::value_ptr(m_SunData.Direction), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
}

void ScatteringSkyModel::SetEnabledClouds ( bool i_Value )
{
	m_AreCloudsEnabled = i_Value;
}

bool ScatteringSkyModel::GetEnabledClouds ( void ) const
{
	return m_AreCloudsEnabled;
}

void ScatteringSkyModel::SetCloudsOctaves ( unsigned short i_Octaves )
{
	if (m_AreCloudsEnabled)
	{
		m_CloudsData.Octaves = i_Octaves;

		m_CloudsSM.UseProgram();
		m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Octaves")->second, m_CloudsData.Octaves);
	}
}

unsigned short ScatteringSkyModel::GetCloudsOctaves ( void ) const
{
	return m_AreCloudsEnabled ? m_CloudsData.Octaves : 0;
}

void ScatteringSkyModel::SetCloudsLacunarity ( float i_Lacunarity )
{
	if (m_AreCloudsEnabled)
	{
		m_CloudsData.Lacunarity = i_Lacunarity;

		m_CloudsSM.UseProgram();
		m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Lacunarity")->second, m_CloudsData.Lacunarity);
	}
}

float ScatteringSkyModel::GetCloudsLacunarity ( void ) const
{
	return m_AreCloudsEnabled ? m_CloudsData.Lacunarity : 0.0f;
}

void ScatteringSkyModel::SetCloudsGain ( float i_Gain )
{
	if (m_AreCloudsEnabled)
	{
		m_CloudsData.Gain = i_Gain;

		m_CloudsSM.UseProgram();
		m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Gain")->second, m_CloudsData.Gain);
	}
}

float ScatteringSkyModel::GetCloudsGain ( void ) const
{
	return m_AreCloudsEnabled ? m_CloudsData.Gain : 0.0f;
}

void ScatteringSkyModel::SetCloudsScaleFactor ( float i_ScaleFactor )
{
	if (m_AreCloudsEnabled)
	{
		m_CloudsData.ScaleFactor = i_ScaleFactor;

		m_CloudsSM.UseProgram();
		m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.ScaleFactor")->second, m_CloudsData.ScaleFactor);
	}
}

float ScatteringSkyModel::GetCloudsScaleFactor ( void ) const
{
	return m_AreCloudsEnabled ? m_CloudsData.ScaleFactor : 0.0f;
}