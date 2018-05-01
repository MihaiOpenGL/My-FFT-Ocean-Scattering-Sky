/* Author: BAIRAC MIHAI */

#include "MotorBoat.h"

#include "GL/glew.h"
#include "GL/glfw3.h"

#include "Camera.h"

#include "Common.h"
//#define ENABLE_ERROR_CHECK
#include "ErrorHandler.h"

#include "glm/gtc/matrix_transform.hpp" //translate()
#include "glm/gtx/rotate_vector.hpp" //rotateY()
#include "glm/gtc/type_ptr.hpp" //value_ptr()

#include "GlobalConfig.h"


const short MotorBoat::m_kBoatTrailVertexCount = 24 * 2;

MotorBoat::MotorBoat ( void ) 
	: m_BoatCurrentPosition(0.0f), m_BoatVelocity(0.0f), m_PropellerWashWidth(0.0f),
	  m_BoatTurnAngle(0.0f), m_BoatAxis(0.0f, 0.0f, -1.0f),
	  m_KelvinWakeOffset(0.0f), m_PropellerWashOffset(0.0f),
	  m_AccelerationFactor(0.0f), m_TurnAngleFactor(0.0f),
	  m_KelvinWakeDisplacementFactor(0.0f), m_FoamAmountFactor(0.0f),
	  m_CrrPropellerPosIdx(0), m_EnableBoatKelvinWake(false), m_EnableBoatPropellerWash(false),
	  m_IsWireframeMode(false), m_BoatArea(0.0f), m_BoatVolume(0.0f), m_BoatMass(0.0f),
	  m_BoatDensity(0.0f), m_BoatDragCoefficient(0.0f), m_BoatYAccelerationFactor(0.0f)
{
}

MotorBoat::MotorBoat (const GlobalConfig& i_Config )
	: m_BoatCurrentPosition(0.0f), m_BoatVelocity(0.0f), m_PropellerWashWidth(0.0f),
	  m_BoatTurnAngle(0.0f), m_BoatAxis(0.0f, 0.0f, -1.0f),
	  m_KelvinWakeOffset(0.0f), m_PropellerWashOffset(0.0f),
	  m_AccelerationFactor(0.0f), m_TurnAngleFactor(0.0f),
	  m_KelvinWakeDisplacementFactor(0.0f), m_FoamAmountFactor(0.0f),
	  m_CrrPropellerPosIdx(0), m_EnableBoatKelvinWake(false), m_EnableBoatPropellerWash(false),
	  m_IsWireframeMode(false), m_BoatArea(0.0f), m_BoatVolume(0.0f), m_BoatMass(0.0f),
	  m_BoatDensity(0.0f), m_BoatDragCoefficient(0.0f), m_BoatYAccelerationFactor(0.0f)
{
	Initialize(i_Config);
}

MotorBoat::~MotorBoat ( void )
{
	Destroy();
}

void MotorBoat::Destroy ( void )
{
	LOG("MotorBoat has been destroyed successfully!");
}

void MotorBoat::Initialize ( const GlobalConfig& i_Config )
{
	m_KelvinWakeOffset = i_Config.Scene.Boat.KelvinWakeOffset;
	m_PropellerWashOffset = i_Config.Scene.Boat.PropellerWashOffset;
	m_PropellerWashWidth = i_Config.Scene.Boat.PropellerWashWidth;
	m_AccelerationFactor = i_Config.Scene.Boat.AccelerationFactor;
	m_TurnAngleFactor = i_Config.Scene.Boat.TurnAngleFactor;
	m_KelvinWakeDisplacementFactor = i_Config.Scene.Boat.KelvinWakeDisplacementFactor;
	m_FoamAmountFactor = i_Config.Scene.Boat.FoamAmountFactor;
	assert(m_PropellerWashWidth > 0.0f);
	assert(m_AccelerationFactor > 0.0f);
	assert(m_TurnAngleFactor > 0.0f);
	assert(m_KelvinWakeDisplacementFactor > 0.0f);
	assert(m_FoamAmountFactor > 0.0f);

	m_BoatCurrentPosition = i_Config.Scene.Boat.Position;
	m_EnableBoatKelvinWake = i_Config.Scene.Ocean.Surface.BoatEffects.KelvinWake.Enabled;
	m_EnableBoatPropellerWash = i_Config.Scene.Ocean.Surface.BoatEffects.PropellerWash.Enabled;

	m_BoatDensity = i_Config.Scene.Boat.Density;
	m_BoatDragCoefficient = i_Config.Scene.Boat.Density;
	m_BoatYAccelerationFactor = i_Config.Scene.Boat.YAccelerationFactor;

	////////// SETUP BOAT /////////////////

	/////////////////////
	m_SM.Initialize("Motor Boat");
	m_SM.BuildRenderingProgram("../resources/shaders/MotorBoat.vert.glsl", "../resources/shaders/MotorBoat.frag.glsl", i_Config);

	m_SM.UseProgram();

	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> attributes;
	attributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_SM.GetAttributeLocation("a_position");
	attributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_NORMAL] = m_SM.GetAttributeLocation("a_normal");
	attributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV] = m_SM.GetAttributeLocation("a_uv");

	m_Uniforms["u_HDRExposure"] = m_SM.GetUniformLocation("u_HDRExposure");
	m_SM.SetUniform(m_Uniforms.find("u_HDRExposure")->second, i_Config.Rendering.HDR.Exposure);
	m_Uniforms["u_ApplyHDR"] = m_SM.GetUniformLocation("u_ApplyHDR");
	m_SM.SetUniform(m_Uniforms.find("u_ApplyHDR")->second, true);

	m_Uniforms["u_WorldToClipMatrix"] = m_SM.GetUniformLocation("u_WorldToClipMatrix");
	m_Uniforms["u_ObjectToWorldMatrix"] = m_SM.GetUniformLocation("u_ObjectToWorldMatrix");

	m_Uniforms["u_BoatDiffMap"] = m_SM.GetUniformLocation("u_BoatDiffMap");
	m_SM.SetUniform(m_Uniforms.find("u_BoatDiffMap")->second, i_Config.TexUnit.MotorBoat.BoatDiffMap);
	m_Uniforms["u_BoatNormalMap"] = m_SM.GetUniformLocation("u_BoatNormalMap");
	m_SM.SetUniform(m_Uniforms.find("u_BoatNormalMap")->second, i_Config.TexUnit.MotorBoat.BoatNormalMap);

	m_Uniforms["u_SunDirection"] = m_SM.GetUniformLocation("u_SunDirection");

	// clip plane for reflection
	m_Uniforms["u_ReflClipPlane"] = m_SM.GetUniformLocation("u_ReflClipPlane");
	m_SM.SetUniform(m_Uniforms.find("u_ReflClipPlane")->second, 1, glm::value_ptr(glm::vec4(0.0f, -1.0f, 0.0f, 0.0f)), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_4);

	// clip plane for refraction
	m_Uniforms["u_RefrClipPlane"] = m_SM.GetUniformLocation("u_RefrClipPlane");
	m_SM.SetUniform(m_Uniforms.find("u_RefrClipPlane")->second, 1, glm::value_ptr(glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_4);

	m_SM.UnUseProgram();

	/////
	m_TM.Initialize("Motor Boat");
	m_TM.Load2DTexture("../resources/models/motor_boat/boat_d.dds", GL_REPEAT, GL_LINEAR, true, i_Config.TexUnit.MotorBoat.BoatDiffMap, 3);
	m_TM.Load2DTexture("../resources/models/motor_boat/boat_n.dds", GL_REPEAT, GL_LINEAR, false, i_Config.TexUnit.MotorBoat.BoatNormalMap, 3);
	/////////


	// NOTE! The model has CCW winding - default
	m_M.LoadModel("Motor Boat", "../resources/models/motor_boat/boat.obj", false, attributes, i_Config.Scene.Boat.UseFlattenedModel);

	//////////////////////////
	m_BoatArea = m_M.GetWidth() * m_M.GetDepth() * 0.7f;
	m_BoatVolume = m_BoatArea * m_M.GetDepth() * 0.8f;
	m_BoatMass = m_BoatVolume * m_BoatDensity;

	//////////////
	if (m_EnableBoatKelvinWake)
	{
		m_BoatKelvinWakeData.BoatPosition = m_BoatCurrentPosition;
		m_BoatKelvinWakeData.WakePosition = m_BoatCurrentPosition + m_BoatAxis * m_KelvinWakeOffset;
		m_BoatKelvinWakeData.Amplitude = 0.0f; // zero amplitutde when the boat is still
		m_BoatKelvinWakeData.FoamAmount = 0.0f; // zero foam when the boat is still
	}

	if (m_EnableBoatPropellerWash)
	{
		m_BoatProperllerWashData.DistortFactor = i_Config.Scene.Ocean.Surface.BoatEffects.PropellerWash.DistortFactor;

		///////////// SETUP BOAT TRAil (Propeller Wash)
		m_TrailSM.Initialize("Motor Boat Trail");
		m_TrailSM.BuildRenderingProgram("../resources/shaders/MotorBoatTrail.vert.glsl", "../resources/shaders/MotorBoatTrail.frag.glsl", i_Config);

		m_TrailSM.UseProgram();

		std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int> TrailAttributes;
		TrailAttributes[MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION] = m_TrailSM.GetAttributeLocation("a_position");


		m_TrailUniforms["u_WorldToClipMatrix"] = m_TrailSM.GetUniformLocation("u_WorldToClipMatrix");
		m_TrailUniforms["u_ObjectToWorldMatrix"] = m_TrailSM.GetUniformLocation("u_ObjectToWorldMatrix");

		m_TrailSM.UnUseProgram();

		/////
		m_TrailVertexData.resize(m_kBoatTrailVertexCount);

		m_CrrPropellerPosIdx = 0;

		// NOTE! The boat is aligned with OZ axis, so the OX axis is its right direction !!!
		glm::vec3 propellerPos = m_BoatCurrentPosition + m_BoatAxis * m_PropellerWashOffset;
		propellerPos.y = 1.0f; // for the wash to be visible we set its Y coord slightly above 0.0f 

		glm::vec3 leftOffsetPropellerPos = propellerPos + glm::vec3(- m_PropellerWashWidth, 0.0f, 0.0f);
		glm::vec3 rightOffsetPropellerPos = propellerPos + glm::vec3(+ m_PropellerWashWidth, 0.0f, 0.0f);

		for (unsigned short i = 0; i < m_TrailVertexData.size(); i += 2)
		{
			m_TrailVertexData[i].position = rightOffsetPropellerPos;
			m_TrailVertexData[i + 1].position = leftOffsetPropellerPos;
		}

		m_PrevPropellerPos = propellerPos;

		m_TrailMBM.Initialize("Motor Boat Trail");
		m_TrailMBM.CreateModelContext(m_TrailVertexData, TrailAttributes, MeshBufferManager::ACCESS_TYPE::AT_DYNAMIC);

		/////
		m_TrailFBM.Initialize("Motor Boat Trail");

		glm::ivec4 viewport;
		glGetIntegerv(GL_VIEWPORT, &viewport[0]);

		unsigned short trailMapWidth = viewport.z;
		unsigned short trailMapHeight = viewport.w;

		m_TrailFBM.CreateSimple(1, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, trailMapWidth, trailMapHeight, GL_CLAMP_TO_EDGE, GL_LINEAR, i_Config.TexUnit.Ocean.Surface.PropellerWashMap, 5, false, FrameBufferManager::DEPTH_BUFFER_TYPE::DBT_NO_DEPTH);
	}

	LOG("MotorBoat has been created successfully!");
}

void MotorBoat::Update ( const Camera& i_Camera, const glm::vec3& i_SunDirection, bool i_IsWireframeMode, float i_CrrTime )
{
	/////////////////////
	// UPDATE BOAT AXIS

	float radAngle = glm::radians(m_BoatTurnAngle);
	m_BoatAxis.x = - glm::sin(radAngle);
	m_BoatAxis.y = 0.0f;
	m_BoatAxis.z = - glm::cos(radAngle);

	////// UPDATE BOAT POSITION
	glm::vec3 deltaPos = m_BoatAxis * m_BoatVelocity * i_CrrTime;

	m_BoatCurrentPosition += deltaPos;

	//////
	if (m_EnableBoatKelvinWake)
	{
		m_BoatKelvinWakeData.BoatPosition = m_BoatCurrentPosition;
		m_BoatKelvinWakeData.WakePosition = m_BoatCurrentPosition + m_BoatAxis * m_KelvinWakeOffset;
	}

	/////////////////////

	if (m_EnableBoatPropellerWash && m_BoatVelocity > 0.0f)
	{
		glm::vec3 propellerPos, leftOffsetPropellerPos, rightOffsetPropellerPos;

		if (m_CrrPropellerPosIdx == m_kBoatTrailVertexCount)
		{
			m_CrrPropellerPosIdx = 0;
			// reseting the counter, the first 2 positions are the last 2, to have continuty across the triangle strip !!!
			m_TrailVertexData[m_CrrPropellerPosIdx++].position = m_TrailVertexData[m_kBoatTrailVertexCount - 2].position;
			m_TrailVertexData[m_CrrPropellerPosIdx++].position = m_TrailVertexData[m_kBoatTrailVertexCount - 1].position;
		}

		propellerPos = m_BoatCurrentPosition + m_BoatAxis * m_PropellerWashOffset;
		propellerPos.y = 1.0f;

		glm::vec3 fowardDir = glm::normalize(propellerPos - m_PrevPropellerPos);
		glm::vec3 upDir = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 rightDir = glm::normalize(glm::cross(upDir, fowardDir));

		leftOffsetPropellerPos = propellerPos + rightDir * (- m_PropellerWashWidth);
		rightOffsetPropellerPos = propellerPos + rightDir * (+ m_PropellerWashWidth);
		
		// save last propeller pos
		m_PrevPropellerPos = propellerPos;

		/////////// Update mesh buffer with vertex data
		m_TrailVertexData[m_CrrPropellerPosIdx ++].position = rightOffsetPropellerPos;
		m_TrailVertexData[m_CrrPropellerPosIdx ++].position = leftOffsetPropellerPos;
		m_TrailMBM.UpdateModelVertexData(m_TrailVertexData);
	}

	////////////////////

	UpdateInternal(glm::mat4(1.0f), false, true, i_Camera, i_SunDirection, i_IsWireframeMode, i_CrrTime);
}

void MotorBoat::UpdateReflected ( const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, const glm::vec3& i_SunDirection, bool i_IsWireframeMode, float i_CrrTime )
{
	UpdateInternal(i_ScaleMatrix, true, false, i_Camera, i_SunDirection, i_IsWireframeMode, i_CrrTime);
}

void MotorBoat::UpdateRefracted ( const glm::mat4& i_ScaleMatrix, const Camera& i_Camera, const glm::vec3& i_SunDirection, bool i_IsWireframeMode, float i_CrrTime )
{
	UpdateInternal(i_ScaleMatrix, false, true, i_Camera, i_SunDirection, i_IsWireframeMode, i_CrrTime);
}

void MotorBoat::UpdateInternal(const glm::mat4& i_ScaleMatrix, bool i_ApplyBoatPositionCorrection, bool i_ApplyHDR, const Camera& i_Camera, const glm::vec3& i_SunDirection, bool i_IsWireframeMode, float i_CrrTime)
{
	m_IsWireframeMode = i_IsWireframeMode;

	//////////////////////////////////////

	// correct rotation transform order:
	// R = RZ * RX * RY

	// correct transform order:
	// Model = translate * rotate * scale

	// we also rotate the boat model to 90 degrees to orient its front with the -Z axis
	glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(m_BoatTurnAngle + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::vec3 tempPos = m_BoatCurrentPosition;
	// this correction is needed for reflections!
	if (i_ApplyBoatPositionCorrection)
	{
		tempPos.y *= -1.0f;
	}

	glm::mat4 T = glm::translate(glm::mat4(1.0f), tempPos);

	glm::mat4 modelMatrix = T * R * i_ScaleMatrix;


	m_SM.UseProgram();
	m_SM.SetUniform(m_Uniforms.find("u_ApplyHDR")->second, i_ApplyHDR);

	m_SM.SetUniform(m_Uniforms.find("u_WorldToClipMatrix")->second, 1, glm::value_ptr(i_Camera.GetProjectionViewMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
	m_SM.SetUniform(m_Uniforms.find("u_ObjectToWorldMatrix")->second, 1, glm::value_ptr(modelMatrix), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);

	m_SM.SetUniform(m_Uniforms.find("u_SunDirection")->second, 1, glm::value_ptr(i_SunDirection), ShaderManager::UNIFORM_TYPE::UT_FLOAT_VEC_3);

	////////////////////
	if (m_EnableBoatPropellerWash)
	{
		glm::mat4 TrailModelMatrix = i_ScaleMatrix;

		m_TrailSM.UseProgram();
		m_TrailSM.SetUniform(m_TrailUniforms.find("u_WorldToClipMatrix")->second, 1, glm::value_ptr(i_Camera.GetProjectionViewMatrix()), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
		m_TrailSM.SetUniform(m_TrailUniforms.find("u_ObjectToWorldMatrix")->second, 1, glm::value_ptr(TrailModelMatrix), ShaderManager::UNIFORM_TYPE::UT_FLOAT_MAT_4);
	}
}

void MotorBoat::Render ( void )
{
	// Render the Motor Boat
	RenderInternal();

	// Render the Boat Trail
	if (m_EnableBoatPropellerWash)
	{
		m_TrailFBM.Bind();

		glClear(GL_COLOR_BUFFER_BIT);

		RenderTrail();

		m_TrailFBM.UnBind();
	}
}

void MotorBoat::RenderReflected ( void )
{
	// revert winding for correct mirror like reflection!
	glFrontFace(GL_CW);

	RenderInternal();

	// restore winding
	glFrontFace(GL_CCW);
}

void MotorBoat::RenderRefracted ( void )
{
	RenderInternal();
}

void MotorBoat::RenderInternal ( void )
{
	m_SM.UseProgram();

	m_M.Render(m_IsWireframeMode);
}

void MotorBoat::RenderTrail ( void )
{
	if (m_EnableBoatPropellerWash && m_BoatVelocity > 0.0f)
	{
		m_TrailSM.UseProgram();
		m_TrailMBM.BindModelContext();

		glDrawArrays(GL_TRIANGLE_STRIP, 0, m_TrailVertexData.size());

		m_TrailMBM.UnBindModelContext();
	}
}

void MotorBoat::RenderFlattened ( void )
{
	m_SM.UseProgram();

	m_M.RenderFlattened();
}

void MotorBoat::Accelerate ( float i_DeltaTime )
{
	m_BoatVelocity += m_AccelerationFactor * i_DeltaTime;

	m_BoatVelocity = glm::min(m_BoatVelocity, 1.0f);

	if (m_EnableBoatKelvinWake)
	{
		m_BoatKelvinWakeData.Amplitude = glm::min(m_BoatVelocity * m_KelvinWakeDisplacementFactor, 20.0f);
		m_BoatKelvinWakeData.FoamAmount = glm::min(m_BoatVelocity * m_FoamAmountFactor, 1.0f);
	}
}

void MotorBoat::Decelerate ( float i_DeltaTime )
{
	m_BoatVelocity -= m_AccelerationFactor * i_DeltaTime;

	m_BoatVelocity = glm::max(m_BoatVelocity, 0.0f);

	if (m_EnableBoatKelvinWake)
	{
		m_BoatKelvinWakeData.Amplitude = glm::min(m_BoatVelocity * m_KelvinWakeDisplacementFactor, 20.0f);
		m_BoatKelvinWakeData.FoamAmount = glm::min(m_BoatVelocity * m_FoamAmountFactor, 1.0f);
	}
}

void MotorBoat::TurnRight ( float i_DeltaTime )
{
	if (m_BoatVelocity == 0.0f) return;

	m_BoatTurnAngle -= m_TurnAngleFactor * i_DeltaTime;
}

void MotorBoat::TurnLeft ( float i_DeltaTime )
{
	if (m_BoatVelocity == 0.0f) return;

	m_BoatTurnAngle += m_TurnAngleFactor * i_DeltaTime;
}

void MotorBoat::BindPropellerWashTexture ( void ) const
{
	if (m_EnableBoatPropellerWash)
	{
		m_TrailFBM.BindColorAttachmentByIndex(0, true);
	}
}

const Ocean::BoatPropellerWashData& MotorBoat::GetPropellerWashData ( void ) const
{
	return m_BoatProperllerWashData;
}

const Ocean::BoatKelvinWakeData& MotorBoat::GetKelvinWakeData ( void ) const
{
	return m_BoatKelvinWakeData;
}

const Model::Dimensions& MotorBoat::GetBoatDimensions ( void ) const
{
	return m_M.GetDimensions();
}

float MotorBoat::GetBoatWidth ( void ) const
{
	return m_M.GetWidth();
}

float MotorBoat::GetBoatHeight ( void ) const
{
	return m_M.GetHeight();
}

float MotorBoat::GetBoatDepth ( void ) const
{
	return m_M.GetDepth();
}

float MotorBoat::GetBoatDensity ( void ) const
{
	return m_BoatDensity;
}


float MotorBoat::GetBoatArea ( void ) const
{
	return m_BoatArea;
}


float MotorBoat::GetBoatVolume ( void ) const
{
	return m_BoatVolume;
}

float MotorBoat::GetBoatMass ( void ) const
{
	return m_BoatMass;
}

float MotorBoat::GetBoatDragCoefficient ( void ) const
{
	return m_BoatDragCoefficient;
}

float MotorBoat::GetBoatYAccelerationFactor ( void ) const
{
	return m_BoatYAccelerationFactor;
}

const glm::vec3& MotorBoat::GetBoatPosition ( void ) const
{
	return m_BoatCurrentPosition;
}

float MotorBoat::GetBoatYPos ( void ) const
{
	return m_BoatCurrentPosition.y;
}

float MotorBoat::GetBoatVelocity ( void ) const
{
	return m_BoatVelocity;
}


void MotorBoat::SetBoatYPos ( float i_BoatYPos )
{
	m_BoatCurrentPosition.y = i_BoatYPos;
}