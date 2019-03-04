/* Author: BAIRAC MIHAI */

#include "PostProcessingManager.h"
#include "GL/glew.h"
#include "ErrorHandler.h"
#include "GlobalConfig.h"
#include "glm/vec4.hpp"
#include "glm/exponential.hpp" //sqrt()
#include "glm/gtc/type_ptr.hpp" //value_ptr()


PostProcessingManager::PostProcessingManager ( const GlobalConfig& i_Config )
	: m_EffectType(CustomTypes::PostProcessing::EffectType::PPET_COUNT)
{
	Initialize(i_Config);
}

PostProcessingManager::~PostProcessingManager ( void )
{
	Destroy();
}

void PostProcessingManager::Destroy ( void )
{
	LOG("PostProcessingManager has been destroyed successfully!");
}

void PostProcessingManager::Initialize ( const GlobalConfig& i_Config )
{
	m_EffectType = i_Config.VisualEffects.PostProcessing.EffectType;

	m_SM.Initialize("PPE");

	std::string fragmentShaderSource;

	switch(m_EffectType)
	{
		case CustomTypes::PostProcessing::EffectType::PPET_Invert:
			fragmentShaderSource = "../resources/shaders/PPE_Invert.frag.glsl";
		break;
		case CustomTypes::PostProcessing::EffectType::PPET_Grey:
			fragmentShaderSource = "../resources/shaders/PPE_Grey.frag.glsl";
			break;
		case CustomTypes::PostProcessing::EffectType::PPET_BlackWhite:
			fragmentShaderSource = "../resources/shaders/PPE_BlackWhite.frag.glsl";
			break;
		case CustomTypes::PostProcessing::EffectType::PPET_Sepia:
			fragmentShaderSource = "../resources/shaders/PPE_Sepia.frag.glsl";
			break;
		case CustomTypes::PostProcessing::EffectType::PPET_Wavy:
			fragmentShaderSource = "../resources/shaders/PPE_Wavy.frag.glsl";
			break;
		case CustomTypes::PostProcessing::EffectType::PPET_Blur:
			fragmentShaderSource = "../resources/shaders/PPE_Blur.frag.glsl";
			break;
		case CustomTypes::PostProcessing::EffectType::PPET_EdgeDetection:
			fragmentShaderSource = "../resources/shaders/PPE_EdgeDetection.frag.glsl";
			break;
		case CustomTypes::PostProcessing::EffectType::PPET_NoEffect:
			fragmentShaderSource = "../resources/shaders/PPE_NoEffect.frag.glsl";
			break;
		default:
			ERR("Invalid Post Processing Effect !");
			break;
	}

	m_SM.BuildRenderingProgram("../resources/shaders/Quad.vert.glsl", fragmentShaderSource, i_Config);

	m_SM.UseProgram();

	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> attributes;
	attributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_SM.GetAttributeLocation("a_position");
	attributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV] = m_SM.GetAttributeLocation("a_uv");

	glm::ivec4 viewport;
	glGetIntegerv(GL_VIEWPORT, &viewport[0]);

	unsigned short windowWidth = viewport.z;
	unsigned short windowHeight = viewport.w;

	m_FBM.Initialize("PPE");
	m_FBM.CreateSimple(attributes, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, windowWidth, windowHeight, GL_REPEAT, GL_LINEAR, i_Config.TexUnit.Global.PostProcessingMap, -1, false, FrameBufferManager::DEPTH_BUFFER_TYPE::DBT_RENDER_BUFFER_DEPTH_STENCIL);
	m_FBM.Bind();
	m_FBM.SetupDrawBuffers(1, 0);
	m_FBM.UnBind();

	//////
	m_Uniforms["u_ColorMap"] = m_SM.GetUniformLocation("u_ColorMap");
	const unsigned short& kColorMapTexUnitId = m_FBM.GetColorAttachmentTexUnitId(0);
	m_SM.SetUniform(m_Uniforms.find("u_ColorMap")->second, kColorMapTexUnitId);

	if (i_Config.VisualEffects.PostProcessing.EffectType == CustomTypes::PostProcessing::EffectType::PPET_Blur)
	{
		m_Uniforms["u_Step"] = m_SM.GetUniformLocation("u_Step");

		glm::vec4 step;
		step.x = 1.0f / windowWidth;
		step.y = 1.0f / windowHeight;
		step.z = glm::sqrt(2.0f) / 2.0f * step.x;
		step.w = glm::sqrt(2.0f) / 2.0f * step.y;

		m_SM.SetUniform(m_Uniforms.find("u_Step")->second, 1, glm::value_ptr(step), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_4);
	}
	else if (i_Config.VisualEffects.PostProcessing.EffectType == CustomTypes::PostProcessing::EffectType::PPET_Wavy)
	{
		m_Uniforms["u_CrrTime"] = m_SM.GetUniformLocation("u_CrrTime");
	}

	m_SM.UnUseProgram();
}

void PostProcessingManager::UpdateSize(unsigned short i_Width, unsigned short i_Height)
{
	m_FBM.UpdateColorAttachmentSize(0, i_Width, i_Height);
	m_FBM.UpdateDepthBufferSize(i_Width, i_Height);
}

void PostProcessingManager::Update ( float i_CrrTime, float i_DeltaTime )
{
	m_SM.UseProgram();

	if (m_EffectType == CustomTypes::PostProcessing::EffectType::PPET_Wavy)
	{
		m_SM.SetUniform(m_Uniforms.find("u_CrrTime")->second, i_CrrTime);
	}
}

void PostProcessingManager::Render ( void )
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_FBM.BindColorAttachmentByIndex(0, false, false);

	m_SM.UseProgram();

	m_FBM.RenderToQuad();
}

void PostProcessingManager::BindFB ( void )
{
	m_FBM.Bind();
}

void PostProcessingManager::UnBindFB ( void )
{
	m_FBM.UnBind();
}
