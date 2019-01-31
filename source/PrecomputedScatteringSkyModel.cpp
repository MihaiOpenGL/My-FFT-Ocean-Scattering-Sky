/* Author: BAIRAC MIHAI */

#include "PrecomputedScatteringSkyModel.h"
#include "GL/glew.h"
//#define ENABLE_ERROR_CHECK
#include "ErrorHandler.h"
#include "glm/gtc/matrix_transform.hpp" //translate()
#include "glm/gtc/type_ptr.hpp" //value_ptr()
#include "glm/gtc/constants.hpp" //pi()
#include "glm/gtx/rotate_vector.hpp" //just for reference
#include "PhysicsConstants.h"
#include "GlobalConfig.h"


PrecomputedScatteringSkyModel::PrecomputedScatteringSkyModel ( void )
	: m_AreCloudsEnabled(false)
{}

PrecomputedScatteringSkyModel::PrecomputedScatteringSkyModel ( const GlobalConfig& i_Config )
	: m_AreCloudsEnabled(false)
{
	Initialize(i_Config);
}

PrecomputedScatteringSkyModel::~PrecomputedScatteringSkyModel ( void )
{
	Destroy();
}

void PrecomputedScatteringSkyModel::Destroy ( void )
{
	LOG("PrecomputedScatteringSkyModel has been destroyed successfully!");
}

void PrecomputedScatteringSkyModel::Initialize ( const GlobalConfig& i_Config )
{
	BaseSkyModel::Initialize(i_Config);

	m_SunData.AllowChangeDirWithMouse = i_Config.Scene.Sky.Model.PrecomputedScattering.Sun.AllowChangeDirWithMouse;
	m_SunData.IsDynamic = i_Config.Scene.Sky.Model.PrecomputedScattering.Sun.IsDynamic;
	m_SunData.MoveFactor = i_Config.Scene.Sky.Model.PrecomputedScattering.Sun.MoveFactor;

	m_AreCloudsEnabled = i_Config.Scene.Sky.Model.PrecomputedScattering.Clouds.Enabled;

	m_CloudsData.Octaves = i_Config.Scene.Sky.Model.PrecomputedScattering.Clouds.Octaves;
	m_CloudsData.Lacunarity = i_Config.Scene.Sky.Model.PrecomputedScattering.Clouds.Lacunarity;
	m_CloudsData.Gain = i_Config.Scene.Sky.Model.PrecomputedScattering.Clouds.Gain;
	m_CloudsData.Norm = i_Config.Scene.Sky.Model.PrecomputedScattering.Clouds.Norm;

	//// Setup Sky
	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> skyAttributes;
	SetupSkyShaders(i_Config, skyAttributes);
	SetupSkyGeometry(skyAttributes);

	//// Setup Clouds
	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> cloudsAttributes;
	SetupCloudsShaders(i_Config, cloudsAttributes);
	SetupCloudsGeometry(i_Config, cloudsAttributes);


	SetupTextures(i_Config);

	LOG("PrecomputedScatteringSkyModel has been created successfully!");
}

void PrecomputedScatteringSkyModel::SetupSkyShaders ( const GlobalConfig& i_Config, std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& o_Attributes )
{
	m_SM.Initialize("PrecomputedScatteringSkyModel Sky");
	m_SM.BuildRenderingProgram("../resources/shaders/PrecomputedScatteringSkyModel.vert.glsl", "../resources/shaders/PrecomputedScatteringSkyModel.frag.glsl", i_Config);

	m_SM.UseProgram();

	o_Attributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_SM.GetAttributeLocation("a_position");

	m_Uniforms["u_PrecomputedScatteringData.TransmittanceMap"] = m_SM.GetUniformLocation("u_PrecomputedScatteringData.TransmittanceMap");
	m_SM.SetUniform(m_Uniforms.find("u_PrecomputedScatteringData.TransmittanceMap")->second, i_Config.TexUnit.Sky.PrecomputedScatteringSkyModel.TransmittanceMap);
	m_Uniforms["u_PrecomputedScatteringData.IrradianceMap"] = m_SM.GetUniformLocation("u_PrecomputedScatteringData.IrradianceMap");
	m_SM.SetUniform(m_Uniforms.find("u_PrecomputedScatteringData.IrradianceMap")->second, i_Config.TexUnit.Sky.PrecomputedScatteringSkyModel.IrradianceMap);
	m_Uniforms["u_PrecomputedScatteringData.InscatterMap"] = m_SM.GetUniformLocation("u_PrecomputedScatteringData.InscatterMap");
	m_SM.SetUniform(m_Uniforms.find("u_PrecomputedScatteringData.InscatterMap")->second, i_Config.TexUnit.Sky.PrecomputedScatteringSkyModel.InscatterMap);
	m_Uniforms["u_PrecomputedScatteringData.EarthRadius"] = m_SM.GetUniformLocation("u_PrecomputedScatteringData.EarthRadius");
	m_SM.SetUniform(m_Uniforms.find("u_PrecomputedScatteringData.EarthRadius")->second, PhysicsConstants::kEarthRadius);
	m_Uniforms["u_PrecomputedScatteringData.SunIntensity"] = m_SM.GetUniformLocation("u_PrecomputedScatteringData.SunIntensity");
	m_SM.SetUniform(m_Uniforms.find("u_PrecomputedScatteringData.SunIntensity")->second, i_Config.Scene.Sky.Model.PrecomputedScattering.Atmosphere.SunIntensity);
	m_Uniforms["u_PrecomputedScatteringData.MieG"] = m_SM.GetUniformLocation("u_PrecomputedScatteringData.MieG");
	m_SM.SetUniform(m_Uniforms.find("u_PrecomputedScatteringData.MieG")->second, i_Config.Scene.Sky.Model.PrecomputedScattering.Atmosphere.MieScattering);
	m_Uniforms["u_PrecomputedScatteringData.PI"] = m_SM.GetUniformLocation("u_PrecomputedScatteringData.PI");
	m_SM.SetUniform(m_Uniforms.find("u_PrecomputedScatteringData.PI")->second, glm::pi<float>());
	m_Uniforms["u_PrecomputedScatteringData.Rgtl"] = m_SM.GetUniformLocation("u_PrecomputedScatteringData.Rgtl");
	m_SM.SetUniform(m_Uniforms.find("u_PrecomputedScatteringData.Rgtl")->second, 1, glm::value_ptr(i_Config.Scene.Sky.Model.PrecomputedScattering.Atmosphere.Rgtl), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
	m_Uniforms["u_PrecomputedScatteringData.BetaR"] = m_SM.GetUniformLocation("u_PrecomputedScatteringData.BetaR");
	m_SM.SetUniform(m_Uniforms.find("u_PrecomputedScatteringData.BetaR")->second, 1, glm::value_ptr(i_Config.Scene.Sky.Model.PrecomputedScattering.Atmosphere.BetaR), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);


	m_Uniforms["u_HDRExposure"] = m_SM.GetUniformLocation("u_HDRExposure");
	m_SM.SetUniform(m_Uniforms.find("u_HDRExposure")->second, i_Config.Rendering.HDR.Exposure * 0.2f);

	m_Uniforms["u_ClipToCameraMatrix"] = m_SM.GetUniformLocation("u_ClipToCameraMatrix");
	m_Uniforms["u_CameraToWorldMatrix"] = m_SM.GetUniformLocation("u_CameraToWorldMatrix");

	m_Uniforms["u_CameraPosition"] = m_SM.GetUniformLocation("u_CameraPosition");
	// the camera position is hardcoded for the sky
	m_SM.SetUniform(m_Uniforms.find("u_CameraPosition")->second, 1, glm::value_ptr(glm::vec3(0.0f)), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

	m_Uniforms["u_SunDirection"] = m_SM.GetUniformLocation("u_SunDirection");
	m_SM.SetUniform(m_Uniforms.find("u_SunDirection")->second, 1, glm::value_ptr(m_SunData.Direction), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
	SetSunDirection(i_Config.Scene.Sky.Model.PrecomputedScattering.Sun.InitialPhi, i_Config.Scene.Sky.Model.PrecomputedScattering.Sun.InitialTheta);

	m_Uniforms["u_IsReflMode"] = m_SM.GetUniformLocation("u_IsReflMode");
	m_SM.SetUniform(m_Uniforms.find("u_IsReflMode")->second, false);
	m_Uniforms["u_IsUnderWater"] = m_SM.GetUniformLocation("u_IsUnderWater");
	m_SM.SetUniform(m_Uniforms.find("u_IsUnderWater")->second, false);

	m_SM.UnUseProgram();
}

void PrecomputedScatteringSkyModel::SetupCloudsShaders ( const GlobalConfig& i_Config, std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& o_Attributes )
{
	m_CloudsSM.Initialize("PrecomputedScatteringSkyModel Clouds");
	m_CloudsSM.BuildRenderingProgram("../resources/shaders/PrecomputedScatteringSkyModelClouds.vert.glsl", "../resources/shaders/PrecomputedScatteringSkyModelClouds.frag.glsl", i_Config);

	m_CloudsSM.UseProgram();

	o_Attributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_CloudsSM.GetAttributeLocation("a_position");

	m_CloudsUniforms["u_HDRExposure"] = m_CloudsSM.GetUniformLocation("u_HDRExposure");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_HDRExposure")->second, i_Config.Rendering.HDR.Exposure * 0.2f);

	m_CloudsUniforms["u_WorldToClipMatrix"] = m_CloudsSM.GetUniformLocation("u_WorldToClipMatrix");

	m_CloudsUniforms["u_CameraPosition"] = m_CloudsSM.GetUniformLocation("u_CameraPosition");
	// the camera position is hardcoded for the sky
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CameraPosition")->second, 1, glm::value_ptr(glm::vec3(0.0f)), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

	m_CloudsUniforms["u_SunDirection"] = m_CloudsSM.GetUniformLocation("u_SunDirection");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_SunDirection")->second, 1, glm::value_ptr(m_SunData.Direction), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

	m_CloudsUniforms["u_PrecomputedScatteringData.TransmittanceMap"] = m_CloudsSM.GetUniformLocation("u_PrecomputedScatteringData.TransmittanceMap");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_PrecomputedScatteringData.TransmittanceMap")->second, i_Config.TexUnit.Sky.PrecomputedScatteringSkyModel.TransmittanceMap);
	m_CloudsUniforms["u_PrecomputedScatteringData.IrradianceMap"] = m_CloudsSM.GetUniformLocation("u_PrecomputedScatteringData.IrradianceMap");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_PrecomputedScatteringData.IrradianceMap")->second, i_Config.TexUnit.Sky.PrecomputedScatteringSkyModel.IrradianceMap);
	m_CloudsUniforms["u_PrecomputedScatteringData.InscatterMap"] = m_CloudsSM.GetUniformLocation("u_PrecomputedScatteringData.InscatterMap");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_PrecomputedScatteringData.InscatterMap")->second, i_Config.TexUnit.Sky.PrecomputedScatteringSkyModel.InscatterMap);
	m_CloudsUniforms["u_PrecomputedScatteringData.EarthRadius"] = m_CloudsSM.GetUniformLocation("u_PrecomputedScatteringData.EarthRadius");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_PrecomputedScatteringData.EarthRadius")->second, PhysicsConstants::kEarthRadius);
	m_CloudsUniforms["u_PrecomputedScatteringData.SunIntensity"] = m_CloudsSM.GetUniformLocation("u_PrecomputedScatteringData.SunIntensity");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_PrecomputedScatteringData.SunIntensity")->second, i_Config.Scene.Sky.Model.PrecomputedScattering.Atmosphere.SunIntensity);
	m_CloudsUniforms["u_PrecomputedScatteringData.MieG"] = m_CloudsSM.GetUniformLocation("u_PrecomputedScatteringData.MieG");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_PrecomputedScatteringData.MieG")->second, i_Config.Scene.Sky.Model.PrecomputedScattering.Atmosphere.MieScattering);
	m_CloudsUniforms["u_PrecomputedScatteringData.PI"] = m_CloudsSM.GetUniformLocation("u_PrecomputedScatteringData.PI");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_PrecomputedScatteringData.PI")->second, glm::pi<float>());
	m_CloudsUniforms["u_PrecomputedScatteringData.Rgtl"] = m_CloudsSM.GetUniformLocation("u_PrecomputedScatteringData.Rgtl");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_PrecomputedScatteringData.Rgtl")->second, 1, glm::value_ptr(i_Config.Scene.Sky.Model.PrecomputedScattering.Atmosphere.Rgtl), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
	m_CloudsUniforms["u_PrecomputedScatteringData.BetaR"] = m_CloudsSM.GetUniformLocation("u_PrecomputedScatteringData.BetaR");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_PrecomputedScatteringData.BetaR")->second, 1, glm::value_ptr(i_Config.Scene.Sky.Model.PrecomputedScattering.Atmosphere.BetaR), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

	m_CloudsUniforms["u_CloudsData.NoiseMap"] = m_CloudsSM.GetUniformLocation("u_CloudsData.NoiseMap");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.NoiseMap")->second, i_Config.TexUnit.Sky.PrecomputedScatteringSkyModel.NoiseMap);

	m_CloudsUniforms["u_CloudsData.Octaves"] = m_CloudsSM.GetUniformLocation("u_CloudsData.Octaves");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Octaves")->second, m_CloudsData.Octaves);
	m_CloudsUniforms["u_CloudsData.Lacunarity"] = m_CloudsSM.GetUniformLocation("u_CloudsData.Lacunarity");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Lacunarity")->second, m_CloudsData.Lacunarity);
	m_CloudsUniforms["u_CloudsData.Gain"] = m_CloudsSM.GetUniformLocation("u_CloudsData.Gain");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Gain")->second, m_CloudsData.Gain);
	m_CloudsUniforms["u_CloudsData.Norm"] = m_CloudsSM.GetUniformLocation("u_CloudsData.Norm");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Norm")->second, m_CloudsData.Norm);
	m_CloudsUniforms["u_CloudsData.Clamp1"] = m_CloudsSM.GetUniformLocation("u_CloudsData.Clamp1");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Clamp1")->second, i_Config.Scene.Sky.Model.PrecomputedScattering.Clouds.Clamp1);
	m_CloudsUniforms["u_CloudsData.Clamp2"] = m_CloudsSM.GetUniformLocation("u_CloudsData.Clamp2");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Clamp2")->second, i_Config.Scene.Sky.Model.PrecomputedScattering.Clouds.Clamp2);
	m_CloudsUniforms["u_CloudsData.Color"] = m_CloudsSM.GetUniformLocation("u_CloudsData.Color");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Color")->second, 1, glm::value_ptr(i_Config.Scene.Sky.Model.PrecomputedScattering.Clouds.Color), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
	m_CloudsUniforms["u_CloudsData.Height"] = m_CloudsSM.GetUniformLocation("u_CloudsData.Height");
	m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Height")->second, i_Config.Scene.Sky.Model.PrecomputedScattering.Clouds.AltitudeOffset);

	m_CloudsSM.UnUseProgram();
}

void PrecomputedScatteringSkyModel::SetupSkyGeometry ( const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_Attributes )
{
	std::vector<MeshBufferManager::VertexData> skyData;
	skyData.resize(4);

	// arranged in CCW winding
	// NOTE! is not influenced by winding because is defiend in clip space directly!
	skyData[0].position = glm::vec3(-1.0f, -1.0f, 0.0f); //0
	skyData[1].position = glm::vec3(1.0f, -1.0f, 0.0f); //1
	skyData[2].position = glm::vec3(-1.0f, 1.0f, 0.0f); //3
	skyData[3].position = glm::vec3(1.0f, 1.0f, 0.0f); //2

	m_MBM.Initialize("PrecomputedScatteringSkyModel");
	m_MBM.CreateModelContext(skyData, i_Attributes, MeshBufferManager::ACCESS_TYPE::AT_STATIC);
}
void PrecomputedScatteringSkyModel::SetupCloudsGeometry ( const GlobalConfig& i_Config, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_Attributes )
{
	float offset = i_Config.Scene.Sky.Model.PrecomputedScattering.Clouds.Offset;
	float altitude = i_Config.Scene.Sky.Model.PrecomputedScattering.Clouds.Altitude;

	std::vector<MeshBufferManager::VertexData> cloudVertexData;
	cloudVertexData.resize(4);

	// CW winding
	cloudVertexData[0].position = glm::vec3(-offset, -offset, altitude); //0
	cloudVertexData[1].position = glm::vec3(-offset, offset, altitude); //3
	cloudVertexData[2].position = glm::vec3(offset, -offset, altitude); //1
	cloudVertexData[3].position = glm::vec3(offset, offset, altitude); //2

	m_CloudsMBM.Initialize("PrecomputedScatteringSkyModel Clouds");
	m_CloudsMBM.CreateModelContext(cloudVertexData, i_Attributes, MeshBufferManager::ACCESS_TYPE::AT_STATIC);
}

void PrecomputedScatteringSkyModel::SetupTextures ( const GlobalConfig& i_Config )
{
	m_TM.Initialize("PrecomputedScatteringSkyModel");

	m_TM.Create2DRawTexture("../resources/textures/irradiance.raw", GL_RGB16F, GL_RGB, GL_FLOAT, 64, 16, GL_CLAMP_TO_EDGE, GL_LINEAR, i_Config.TexUnit.Sky.PrecomputedScatteringSkyModel.IrradianceMap);
	m_TM.Create3DRawTexture("../resources/textures/inscatter.raw", GL_RGBA16F, GL_RGBA, GL_FLOAT, 256, 128, 32, GL_CLAMP_TO_EDGE, GL_LINEAR, i_Config.TexUnit.Sky.PrecomputedScatteringSkyModel.InscatterMap);
	m_TM.Create2DRawTexture("../resources/textures/transmittance.raw", GL_RGB16F, GL_RGB, GL_FLOAT, 256, 64, GL_CLAMP_TO_EDGE, GL_LINEAR, i_Config.TexUnit.Sky.PrecomputedScatteringSkyModel.TransmittanceMap);

	m_TM.Create2DRawTexture("../resources/textures/noise.pgm", GL_RGB, GL_RED, GL_UNSIGNED_BYTE, 512, 512, GL_REPEAT, GL_LINEAR, i_Config.TexUnit.Sky.PrecomputedScatteringSkyModel.NoiseMap, 0, true, 38);
}

void PrecomputedScatteringSkyModel::Update ( const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	UpdateInternal(- glm::half_pi<float>(), false, false, i_Camera, false, i_IsUnderWater, i_IsWireframeMode, i_CrrTime);
}

void PrecomputedScatteringSkyModel::UpdateReflected ( const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	UpdateInternal( glm::half_pi<float>(), true, true, i_Camera, true, false, i_IsWireframeMode, i_CrrTime);
}

void PrecomputedScatteringSkyModel::UpdateRefracted ( const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	UpdateInternal(- glm::half_pi<float>(), false, false, i_Camera, false, false, i_IsWireframeMode, i_CrrTime);
}

void PrecomputedScatteringSkyModel::UpdateInternal ( float i_RotateAngle, bool i_ApplyCloudsCorrection, bool i_ApplySunDirCorrection, const Camera& i_Camera, bool i_IsReflMode, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	m_IsWireframeMode = i_IsWireframeMode;

	UpdateSunPosition(i_CrrTime);

	//////////////////////////////
	// correct transform order :
	// v' = RZ * RX * RY * v

	// v' = translate * rotate * scale * v

	// The Eric Bruneton's implementation is a special one!
	// We do not use the usual model matrix, but do a rotation !!!

	// because the sky is built in a special way, it appears to be rotated with pi / 2 radions on OX axis
	// we must rotate it back with - pi / 2 radians
	// we must provide a special matrix to correctly position it realtive to the other objects of the world!
	// we also remove the translation matrix from the view matrix to not imfluence the sky if the camera moves!
	glm::mat4 rotateMatrix = glm::rotate(glm::mat4(1.0f), i_RotateAngle, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 correctedViewMatrix = glm::mat4(glm::mat3(i_Camera.GetViewMatrix())) * rotateMatrix;

	/////// Update Sky Shader //////////
	m_SM.UseProgram();

	m_SM.SetUniform(m_Uniforms.find("u_ClipToCameraMatrix")->second, 1, glm::value_ptr(i_Camera.GetInverseProjectionMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
	m_SM.SetUniform(m_Uniforms.find("u_CameraToWorldMatrix")->second, 1, glm::value_ptr(glm::inverse(correctedViewMatrix)), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);

	m_SM.SetUniform(m_Uniforms.find("u_CameraPosition")->second, 1, glm::value_ptr(glm::vec3(0.0f, i_Camera.GetAltitude(), 0.0f)), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

	m_SM.SetUniform(m_Uniforms.find("u_IsReflMode")->second, i_IsReflMode);

	m_SM.SetUniform(m_Uniforms.find("u_IsUnderWater")->second, i_IsUnderWater);

	////// Update Clouds Shader /////////
	glm::mat4 T(1.0f);
	if (i_ApplyCloudsCorrection)
	{
		T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3000.0f, 0.0f));
	}
	else
	{
		T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3000.0f, 0.0f));
	}

	glm::mat4 RX(glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

	glm::mat4 crrViewMatrix = glm::mat4(glm::mat3(i_Camera.GetViewMatrix())) * T * RX;

	if (m_AreCloudsEnabled)
	{
		m_CloudsSM.UseProgram();
		m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_WorldToClipMatrix")->second, 1, glm::value_ptr(i_Camera.GetProjectionMatrix() * crrViewMatrix), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
		m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CameraPosition")->second, 1, glm::value_ptr(i_Camera.GetPosition()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
	}

	// correction needed only for reflection - sky map
	if (i_ApplySunDirCorrection)
	{
		m_SM.UseProgram();

		glm::vec3 correctedSunDir = m_SunData.Direction;
		correctedSunDir.y *= -1.0f;
		m_SM.SetUniform(m_Uniforms.find("u_SunDirection")->second, 1, glm::value_ptr(correctedSunDir), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
	}
	else
	{
		m_SM.UseProgram();
		m_SM.SetUniform(m_Uniforms.find("u_SunDirection")->second, 1, glm::value_ptr(m_SunData.Direction), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

		if (m_AreCloudsEnabled)
		{
			m_CloudsSM.UseProgram();
			m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_SunDirection")->second, 1, glm::value_ptr(m_SunData.Direction), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
		}
	}
}

void PrecomputedScatteringSkyModel::UpdateSunPosition ( float i_CrrTime )
{
	if (m_SunData.IsDynamic)
	{
		float angle = m_SunData.MoveFactor * i_CrrTime * glm::two_pi<float>();
		SetSunDirection(- glm::half_pi<float>(), angle);
	}
}

void PrecomputedScatteringSkyModel::Render ( void )
{
	RenderInternal();
}

void PrecomputedScatteringSkyModel::RenderReflected ( void )
{
	// in this sky model there is no need to change the winding, because the geometry is a just a quad already in clip space!

	RenderInternal(true);
}

void PrecomputedScatteringSkyModel::RenderRefracted ( void )
{
	RenderInternal();
}

void PrecomputedScatteringSkyModel::RenderInternal ( bool i_RevertWinding )
{
	/////// RENDER SKY ////////
	m_SM.UseProgram();

	m_MBM.BindModelContext();
	glDrawArrays(m_IsWireframeMode ? GL_LINE_STRIP : GL_TRIANGLE_STRIP, 0, 4);
	m_MBM.UnBindModelContext();

	////////// RENDER CLOUDS /////////
	if (m_AreCloudsEnabled)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		m_CloudsSM.UseProgram();

		if (i_RevertWinding) glFrontFace(GL_CW);

		m_CloudsMBM.BindModelContext();
		glDrawArrays(m_IsWireframeMode ? GL_LINE_STRIP : GL_TRIANGLE_STRIP, 0, 4);
		m_CloudsMBM.UnBindModelContext();

		if (i_RevertWinding) glFrontFace(GL_CCW);

		glDisable(GL_BLEND);
	}
}

void PrecomputedScatteringSkyModel::SetSunDirection ( float i_Phi, float i_Theta )
{
	BaseSkyModel::SetSunDirection(i_Phi, i_Theta);
}

void PrecomputedScatteringSkyModel::SetEnabledClouds ( bool i_Value )
{
	m_AreCloudsEnabled = i_Value;
}

bool PrecomputedScatteringSkyModel::GetEnabledClouds ( void ) const
{
	return m_AreCloudsEnabled;
}

void PrecomputedScatteringSkyModel::SetCloudsOctaves ( unsigned short i_Octaves )
{
	if (m_AreCloudsEnabled)
	{
		m_CloudsData.Octaves = i_Octaves;

		m_CloudsSM.UseProgram();
		m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Octaves")->second, m_CloudsData.Octaves);
	}
}

unsigned short PrecomputedScatteringSkyModel::GetCloudsOctaves ( void ) const
{
	return m_AreCloudsEnabled ? m_CloudsData.Octaves : 0;
}

void PrecomputedScatteringSkyModel::SetCloudsLacunarity ( float i_Lacunarity )
{
	if (m_AreCloudsEnabled)
	{
		m_CloudsData.Lacunarity = i_Lacunarity;

		m_CloudsSM.UseProgram();
		m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Lacunarity")->second, m_CloudsData.Lacunarity);
	}
}

float PrecomputedScatteringSkyModel::GetCloudsLacunarity ( void ) const
{
	return m_AreCloudsEnabled ? m_CloudsData.Lacunarity : 0.0f;
}

void PrecomputedScatteringSkyModel::SetCloudsGain ( float i_Gain )
{
	if (m_AreCloudsEnabled)
	{
		m_CloudsData.Gain = i_Gain;

		m_CloudsSM.UseProgram();
		m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Gain")->second, m_CloudsData.Gain);
	}
}

float PrecomputedScatteringSkyModel::GetCloudsGain ( void ) const
{
	return m_AreCloudsEnabled ? m_CloudsData.Gain : 0.0f;
}

void PrecomputedScatteringSkyModel::SetCloudsNorm ( float i_Norm )
{
	if (m_AreCloudsEnabled)
	{
		m_CloudsData.Norm = i_Norm;

		m_CloudsSM.UseProgram();
		m_CloudsSM.SetUniform(m_CloudsUniforms.find("u_CloudsData.Norm")->second, m_CloudsData.Norm);
	}
}

float PrecomputedScatteringSkyModel::GetCloudsNorm ( void ) const
{
	return m_AreCloudsEnabled ? m_CloudsData.Norm : 0.0f;
}