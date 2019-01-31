/* Author: BAIRAC MIHAI */

#include "CubeMapSkyModel.h"
#include "GL/glew.h"
#include "Camera.h"
#include "ErrorHandler.h"
#include "glm/gtc/matrix_transform.hpp" //translate()
#include "glm/gtc/type_ptr.hpp" //value_ptr()
#include "glm/gtc/constants.hpp" //pi()
#include "GlobalConfig.h"


CubeMapSkyModel::CubeMapSkyModel ( void )
{}

CubeMapSkyModel::CubeMapSkyModel ( const GlobalConfig& i_Config )
{
	Initialize(i_Config);
}

CubeMapSkyModel::~CubeMapSkyModel ( void )
{
	Destroy();
}

void CubeMapSkyModel::Destroy ( void )
{
	LOG("CubeMapSkyModel has been destroyed successfully!");
}

void CubeMapSkyModel::Initialize ( const GlobalConfig& i_Config )
{
	BaseSkyModel::Initialize(i_Config);

	m_SunData.AllowChangeDirWithMouse = i_Config.Scene.Sky.Model.Cubemap.Sun.AllowChangeDirWithMouse;
	m_SunData.IsDynamic = i_Config.Scene.Sky.Model.Cubemap.Sun.IsDynamic;
	m_SunData.MoveFactor = i_Config.Scene.Sky.Model.Cubemap.Sun.MoveFactor;

	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> attributes;
	SetupShaders(i_Config, attributes);
	SetupGeometry(attributes);
	SetupTextures(i_Config);

	LOG("CubeMapSkyModel has been created successfully!");
}

void CubeMapSkyModel::SetupShaders ( const GlobalConfig& i_Config, std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& o_Attributes )
{
	m_SM.Initialize("CubeMapSkyModel");
	m_SM.BuildRenderingProgram("../resources/shaders/CubeMapSkyModel.vert.glsl", "../resources/shaders/CubeMapSkyModel.frag.glsl", i_Config);

	m_SM.UseProgram();

	o_Attributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_SM.GetAttributeLocation("a_position");

	m_Uniforms["u_HDRExposure"] = m_SM.GetUniformLocation("u_HDRExposure");
	m_SM.SetUniform(m_Uniforms.find("u_HDRExposure")->second, i_Config.Rendering.HDR.Exposure);
	m_Uniforms["u_ApplyHDR"] = m_SM.GetUniformLocation("u_ApplyHDR");
	m_SM.SetUniform(m_Uniforms.find("u_ApplyHDR")->second, 1);

	m_Uniforms["u_WorldToCameraMatrix"] = m_SM.GetUniformLocation("u_WorldToCameraMatrix");
	m_Uniforms["u_WorldToClipMatrix"] = m_SM.GetUniformLocation("u_WorldToClipMatrix");
	m_Uniforms["u_ObjectToWorldMatrix"] = m_SM.GetUniformLocation("u_ObjectToWorldMatrix");

	m_Uniforms["u_CubeMap"] = m_SM.GetUniformLocation("u_CubeMap");
	m_SM.SetUniform(m_Uniforms.find("u_CubeMap")->second, i_Config.TexUnit.Sky.CubeMapSkyModel.CubeMap);

	m_Uniforms["u_IsUnderWater"] = m_SM.GetUniformLocation("u_IsUnderWater");
	m_SM.SetUniform(m_Uniforms.find("u_IsUnderWater")->second, false);

	m_Uniforms["u_SunData.Shininess"] = m_SM.GetUniformLocation("u_SunData.Shininess");
	m_SM.SetUniform(m_Uniforms.find("u_SunData.Shininess")->second, i_Config.Scene.Sky.Model.Cubemap.Sun.Shininess);
	m_Uniforms["u_SunData.Strength"] = m_SM.GetUniformLocation("u_SunData.Strength");
	m_SM.SetUniform(m_Uniforms.find("u_SunData.Strength")->second, i_Config.Scene.Sky.Model.Cubemap.Sun.Strength);
	m_Uniforms["u_SunData.SunFactor"] = m_SM.GetUniformLocation("u_SunData.SunFactor");
	m_SM.SetUniform(m_Uniforms.find("u_SunData.SunFactor")->second, i_Config.Scene.Sky.Model.Cubemap.Sun.SunFactor);

	m_Uniforms["u_SunData.Direction"] = m_SM.GetUniformLocation("u_SunData.Direction");
	SetSunDirection(i_Config.Scene.Sky.Model.Cubemap.Sun.InitialPhi, i_Config.Scene.Sky.Model.Cubemap.Sun.InitialTheta);

	m_Uniforms["u_UnderWaterColor"] = m_SM.GetUniformLocation("u_UnderWaterColor");
	m_SM.SetUniform(m_Uniforms.find("u_UnderWaterColor")->second, 1, glm::value_ptr(i_Config.Scene.Sky.UnderWaterColor), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

	m_Uniforms["u_UnderWaterFogColor"] = m_SM.GetUniformLocation("u_UnderWaterFogColor");
	m_SM.SetUniform(m_Uniforms.find("u_UnderWaterFogColor")->second, 1, glm::value_ptr(i_Config.Scene.Ocean.UnderWater.Fog.Color), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

	m_SM.UnUseProgram();
}

void CubeMapSkyModel::SetupGeometry ( const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_Attributes )
{
	/////////////// SKY cube geometry setup
	float cube_vertices[] = {
		-1.0,  1.0,  1.0, //0
		-1.0, -1.0,  1.0, //1
		1.0, -1.0,  1.0, //2
		1.0,  1.0,  1.0, //3
		-1.0,  1.0, -1.0, //4
		-1.0, -1.0, -1.0, //5
		1.0, -1.0, -1.0, //6
		1.0,  1.0, -1.0, //7
	};

	std::vector<MeshBufferManager::VertexData> vertexData;
	vertexData.resize(8);

	for (size_t i = 0, j = 0; i < 8; ++i, j += 3)
	{
		vertexData[i].position.x = cube_vertices[j];
		vertexData[i].position.y = cube_vertices[j + 1];
		vertexData[i].position.z = cube_vertices[j + 2];
	}

	// CCW winding
	//unsigned int cube_indices[] = {
	//	0, 1, 3, 1, 2, 3,
	//	4, 7, 6, 4, 6, 5,
	//	0, 4, 5, 0, 5, 1,
	//	2, 7, 3, 2, 6, 7,
	//	0, 3, 4, 3, 7, 4,
	//	1, 5, 2, 2, 5, 6
	//};

	// CW winding
	unsigned int cube_indices[] = {
		0, 3, 1, 1, 3, 2,
		4, 6, 7, 4, 5, 6,
		0, 5, 4, 0, 1, 5,
		2, 3, 7, 2, 7, 6,
		0, 4, 3, 3, 4, 7,
		1, 2, 5, 2, 6, 5
	};

	std::vector<unsigned int> indices;
	indices.resize(36);

	for (size_t i = 0; i < 36; ++i)
	{
		indices[i] = cube_indices[i];
	}

	m_IndexCount = 36;

	/////////
	m_MBM.Initialize("CubeMapSkyModel");
	m_MBM.CreateModelContext(vertexData, indices, i_Attributes, MeshBufferManager::ACCESS_TYPE::AT_STATIC);
}

void CubeMapSkyModel::SetupTextures ( const GlobalConfig& i_Config )
{
	m_TM.Initialize("CubeMapSkyModel");

	std::vector<std::string> cubeMapFaces;
	cubeMapFaces.resize(6);

	cubeMapFaces[0] = "../resources/textures/xneg.dds";
	cubeMapFaces[1] = "../resources/textures/xpos.dds";
	cubeMapFaces[2] = "../resources/textures/ypos.dds";
	cubeMapFaces[3] = "../resources/textures/yneg.dds";
	cubeMapFaces[4] = "../resources/textures/zpos.dds";
	cubeMapFaces[5] = "../resources/textures/zneg.dds";

	m_TM.LoadCubeMapTexture(cubeMapFaces, GL_CLAMP_TO_EDGE, GL_LINEAR, true, i_Config.TexUnit.Sky.CubeMapSkyModel.CubeMap);
}

void CubeMapSkyModel::Update ( const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	UpdateInternal(glm::mat4(1.0f), true, i_Camera, i_IsUnderWater, i_IsWireframeMode, i_CrrTime);
}

void CubeMapSkyModel::UpdateReflected ( const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	UpdateInternal(i_ScaleMatrix, false, i_Camera, false, i_IsWireframeMode, i_CrrTime);
}

void CubeMapSkyModel::UpdateRefracted ( const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	UpdateInternal(i_ScaleMatrix, false, i_Camera, false, i_IsWireframeMode, i_CrrTime);
}

void CubeMapSkyModel::UpdateInternal ( const glm::mat4& i_ModelMatrix, bool i_ApplyHDR, const Camera& i_Camera, bool i_IsUnderWater, bool i_IsWireframeMode, float i_CrrTime )
{
	m_IsWireframeMode = i_IsWireframeMode;

	BaseSkyModel::UpdateSunPosition(i_CrrTime);

	/////////////////////////////
	m_SM.UseProgram();

	m_SM.SetUniform(m_Uniforms.find("u_ApplyHDR")->second, i_ApplyHDR);

	// we remove the translation vector from the view matrix !
	// this allows the skybox to be still even if the camera moves !
	// as explained in here:  https://learnopengl.com/#!Advanced-OpenGL/Cubemaps
	glm::mat4 PV = i_Camera.GetProjectionMatrix() * glm::mat4(glm::mat3(i_Camera.GetViewMatrix()));

	m_SM.SetUniform(m_Uniforms.find("u_WorldToClipMatrix")->second, 1, glm::value_ptr(PV), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
	m_SM.SetUniform(m_Uniforms.find("u_ObjectToWorldMatrix")->second, 1, glm::value_ptr(i_ModelMatrix), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);

	m_SM.SetUniform(m_Uniforms.find("u_IsUnderWater")->second, i_IsUnderWater);
}

void CubeMapSkyModel::Render ( void )
{
	RenderInternal();
}

void CubeMapSkyModel::RenderReflected ( void )
{
	// revert winding for correct mirror like reflection!
	glFrontFace(GL_CW); 

	RenderInternal();

	// restore winding
	glFrontFace(GL_CCW);
}

void CubeMapSkyModel::RenderRefracted ( void )
{
	RenderInternal();
}

void CubeMapSkyModel::RenderInternal ( void )
{
	// we set a different depth function as explined here: https://learnopengl.com/#!Advanced-OpenGL/Cubemaps
	glDepthFunc(GL_LEQUAL);

	//////////// RENDER SKY /////////////
	m_SM.UseProgram();

	m_MBM.BindModelContext();
	glDrawElements(m_IsWireframeMode ? GL_LINES : GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
	m_MBM.UnBindModelContext();

	glDepthFunc(GL_LESS);
}

void CubeMapSkyModel::SetSunDirection ( float i_Phi, float i_Theta )
{
	//////
	BaseSkyModel::SetSunDirection(i_Phi, i_Theta);

	m_SM.UseProgram();
	m_SM.SetUniform(m_Uniforms.find("u_SunData.Direction")->second, 1, glm::value_ptr(m_SunData.Direction), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);
}