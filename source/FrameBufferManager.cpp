/* Author: BAIRAC MIHAI */

#include "FrameBufferManager.h"
#include <assert.h>
#include "GL/glew.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "ErrorHandler.h"


int FrameBufferManager::m_MaxColorAttachments = 0;

FrameBufferManager::FrameBufferManager ( void )
	: m_FBOID(0), m_RBOID(0), m_DepthBufferType(DEPTH_BUFFER_TYPE::DBT_NO_DEPTH),
	  m_DepthTexID(0), m_CubeMapTexId(0)
{
	Init();
}

FrameBufferManager::FrameBufferManager ( const std::string& i_Name )
	: m_Name(i_Name), m_FBOID(0), m_RBOID(0), m_DepthBufferType(DEPTH_BUFFER_TYPE::DBT_NO_DEPTH), 
	  m_DepthTexID(0), m_CubeMapTexId(0)
{
	Init();
}

FrameBufferManager::~FrameBufferManager ( void )
{
	Destroy();
}

void FrameBufferManager::Init ( void )
{
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &m_MaxColorAttachments);

	m_TM.Initialize(m_Name);
}

void FrameBufferManager::Initialize ( const std::string& i_Name )
{
	m_Name = i_Name;

	Init();
}

void FrameBufferManager::Destroy ( void )
{
	// NOTE! If we use texture as depth buffer then we don't have to delete it, the texture manager takes care of that!
	if (m_RBOID && (m_DepthBufferType == DEPTH_BUFFER_TYPE::DBT_RENDER_BUFFER_DEPTH || 
		m_DepthBufferType == DEPTH_BUFFER_TYPE::DBT_RENDER_BUFFER_DEPTH_STENCIL))
	{
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glDeleteRenderbuffers(1, &m_RBOID);
	}
	else if (m_DepthBufferType == DEPTH_BUFFER_TYPE::DBT_TEXTURE_DEPTH ||
			 m_DepthBufferType == DEPTH_BUFFER_TYPE::DBT_TEXTURE_DEPTH_STENCIL)
	{
		m_TM.Destroy();
	}

	if (m_FBOID)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &m_FBOID);
		m_FBOID = 0;
	}

	LOG("[" + m_Name + "] FrameBufferManager has been destroyed successfully!");
}

void FrameBufferManager::CreateSimple(unsigned short i_ColorAttachmentCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_StartTexUnitID, short i_MipMapCount, bool i_AnisoFiltering, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType)
{
	SetupSimple(i_ColorAttachmentCount, i_FormatInternal, i_FormatExternal, i_FormatType, i_Width, i_Height, i_WrapType, i_FilterType, i_StartTexUnitID, i_MipMapCount, i_AnisoFiltering, i_DepthBufferType);

	CreateQuadModel();
}

void FrameBufferManager::CreateSimple(const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, unsigned short i_ColorAttachmentCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_StartTexUnitID, short i_MipMapCount, bool i_AnisoFiltering, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType)
{
	SetupSimple(i_ColorAttachmentCount, i_FormatInternal, i_FormatExternal, i_FormatType, i_Width, i_Height, i_WrapType, i_FilterType, i_StartTexUnitID, i_MipMapCount, i_AnisoFiltering, i_DepthBufferType);

	CreateQuadModel(i_ModelVertexAttributes);
}

void FrameBufferManager::CreateLayered(unsigned short i_ColorAttachmentCount, unsigned short i_LayerCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, short i_StartTexUnitID, short i_MipMapCount, bool i_AnisoFiltering, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType)
{
	SetupLayered(i_ColorAttachmentCount, i_LayerCount, i_FormatInternal, i_FormatExternal, i_FormatType, i_Width, i_Height, i_WrapType, i_FilterType, i_StartTexUnitID, i_MipMapCount, i_AnisoFiltering, i_DepthBufferType);

	CreateQuadModel();
}

void FrameBufferManager::CreateLayered(const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, unsigned short i_ColorAttachmentCount, unsigned short i_LayerCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, short i_StartTexUnitID, short i_MipMapCount, bool i_AnisoFiltering, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType)
{
	SetupLayered(i_ColorAttachmentCount, i_LayerCount, i_FormatInternal, i_FormatExternal, i_FormatType, i_Width, i_Height, i_WrapType, i_FilterType, i_StartTexUnitID, i_MipMapCount, i_AnisoFiltering, i_DepthBufferType);

	CreateQuadModel(i_ModelVertexAttributes);
}

void FrameBufferManager::CreateLayered(unsigned int i_ColorAttachmentTexId, unsigned short i_ColorAttachmentLayerCount)
{
	SetupLayered(i_ColorAttachmentTexId, i_ColorAttachmentLayerCount);

	CreateQuadModel();
}

void FrameBufferManager::CreateLayered(const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, unsigned int i_ColorAttachmentTexId, unsigned short i_ColorAttachmentLayerCount)
{
	SetupLayered(i_ColorAttachmentTexId, i_ColorAttachmentLayerCount);

	CreateQuadModel(i_ModelVertexAttributes);
}

void FrameBufferManager::CreateCubeMaped(unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_StartTexUnitID, short i_MipMapCount, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType)
{
	SetupCubeMaped(i_FormatInternal, i_FormatExternal, i_FormatType, i_Width, i_Height, i_WrapType, i_FilterType, i_StartTexUnitID, i_MipMapCount, i_DepthBufferType);

	CreateQuadModel();
}

void FrameBufferManager::CreateCubeMaped(const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_StartTexUnitID, short i_MipMapCount, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType)
{
	SetupCubeMaped(i_FormatInternal, i_FormatExternal, i_FormatType, i_Width, i_Height, i_WrapType, i_FilterType, i_StartTexUnitID, i_MipMapCount, i_DepthBufferType);

	CreateQuadModel(i_ModelVertexAttributes);
}

void FrameBufferManager::CreateQuadModel ( void )
{
	std::vector<MeshBufferManager::VertexData> quadData;
	quadData.resize(4);

	quadData[0].position = glm::vec3(-1.0f, -1.0f, 0.0f); //0
	quadData[0].uv = glm::vec2(0.0f, 0.0f);
	quadData[1].position = glm::vec3(1.0f, -1.0f, 0.0f); //1
	quadData[1].uv = glm::vec2(1.0f, 0.0f);
	quadData[2].position = glm::vec3(-1.0f, 1.0f, 0.0f); //3
	quadData[2].uv = glm::vec2(0.0f, 1.0f);
	quadData[3].position = glm::vec3(1.0f, 1.0f, 0.0f); //2
	quadData[3].uv = glm::vec2(1.0f, 1.0f);

	m_QuadMBM.Initialize(m_Name);
	m_QuadMBM.CreateModel(quadData, MeshBufferManager::ACCESS_TYPE::AT_STATIC);
}

void FrameBufferManager::CreateQuadModel ( const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes )
{
	std::vector<MeshBufferManager::VertexData> quadData;
	quadData.resize(4);

	quadData[0].position = glm::vec3(-1.0f, -1.0f, 0.0f); //0
	quadData[0].uv = glm::vec2(0.0f, 0.0f);
	quadData[1].position = glm::vec3(1.0f, -1.0f, 0.0f); //1
	quadData[1].uv = glm::vec2(1.0f, 0.0f);
	quadData[2].position = glm::vec3(-1.0f, 1.0f, 0.0f); //3
	quadData[2].uv = glm::vec2(0.0f, 1.0f);
	quadData[3].position = glm::vec3(1.0f, 1.0f, 0.0f); //2
	quadData[3].uv = glm::vec2(1.0f, 1.0f);

	m_QuadMBM.Initialize(m_Name);
	m_QuadMBM.CreateModelContext(quadData, i_ModelVertexAttributes, MeshBufferManager::ACCESS_TYPE::AT_STATIC);
}

void FrameBufferManager::SetupSimple ( unsigned short i_ColorAttachmentCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_StartTexUnitID, short i_MipMapCount, bool i_AnisoFiltering, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType )
{
	if (i_ColorAttachmentCount == 0 || i_ColorAttachmentCount >= m_MaxColorAttachments)
	{
		ERR("Number of color attachments exceeds the supported level!");
		return;
	}

	GenAndBindFramebuffer();

	// setup draw buffers
	m_DrawBuffers.resize(i_ColorAttachmentCount);

	unsigned short texUnitID = i_StartTexUnitID;
	for (unsigned short i = 0; i < i_ColorAttachmentCount; ++i)
	{
		m_DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;

		unsigned int texId = m_TM.Create2DTexture(i_FormatInternal, i_FormatExternal, i_FormatType, i_Width, i_Height, i_WrapType, i_FilterType, nullptr, texUnitID, i_MipMapCount, i_AnisoFiltering);

		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, m_DrawBuffers[i], texId, 0);

		++texUnitID;
	}

	glDrawBuffer(m_DrawBuffers[0]);
	//

	SetupDepthBuffer(i_DepthBufferType, i_Width, i_Height);

	CheckCompletenessStatus();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferManager::SetupLayered ( unsigned short i_ColorAttachmentCount, unsigned short i_LayerCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_StartTexUnitID, short i_MipMapCount, bool i_AnisoFiltering, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType )
{
	if (i_ColorAttachmentCount == 0 || i_ColorAttachmentCount >= m_MaxColorAttachments)
	{
		ERR("Number of color attachments exceeds the supported level!");
		return;
	}

	if (!TextureManager::CheckLayerCount(i_LayerCount))
	{
		ERR("Number of layers exceeds the supported level!");
		return;
	}

	GenAndBindFramebuffer();

	// setup draw buffers
	m_DrawBuffers.resize(i_ColorAttachmentCount * i_LayerCount);

	unsigned short texUnitID = i_StartTexUnitID;
	for (unsigned short i = 0; i < i_ColorAttachmentCount; ++i)
	{
		unsigned int texId = m_TM.Create2DArrayTexture(i_LayerCount, i_FormatInternal, i_FormatExternal, i_FormatType, i_Width, i_Height, i_WrapType, i_FilterType, nullptr, texUnitID, i_MipMapCount, i_AnisoFiltering);

		for (unsigned short j = 0; j < i_LayerCount; ++j)
		{
			unsigned int idx = i * i_LayerCount + j;

			m_DrawBuffers[idx] = GL_COLOR_ATTACHMENT0 + idx;

			glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, m_DrawBuffers[idx], texId, 0, j);
		}

		++texUnitID;
	}

	glDrawBuffer(m_DrawBuffers[0]);
	//

	SetupDepthBuffer(i_DepthBufferType, i_Width, i_Height);

	CheckCompletenessStatus();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferManager::SetupLayered ( unsigned int i_ColorAttachmentTexId, unsigned short i_ColorAttachmentLayerCount )
{
	if (!TextureManager::CheckLayerCount(i_ColorAttachmentLayerCount))
	{
		ERR("Number of layers exceeds the supported level!");
		return;
	}

	GenAndBindFramebuffer();

	// setup draw buffers
	m_DrawBuffers.resize(i_ColorAttachmentLayerCount);

	for (unsigned short i = 0; i < i_ColorAttachmentLayerCount; ++i)
	{
		m_DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;

		glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, m_DrawBuffers[i], i_ColorAttachmentTexId, 0, i);
	}

	glDrawBuffer(m_DrawBuffers[0]);
	//

	CheckCompletenessStatus();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferManager::SetupCubeMaped ( unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_StartTexUnitID, short i_MipMapCount, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType )
{
	GenAndBindFramebuffer();

	// setup draw buffers
	m_DrawBuffers.resize(1);
	m_DrawBuffers[0] = GL_COLOR_ATTACHMENT0;

	// http://stackoverflow.com/questions/462721/rendering-to-cube-map
	// http://www.gamedev.net/topic/610644-rendering-to-cubemap-for-environment-mapping/
	// http://gamedev.stackexchange.com/questions/19461/opengl-glsl-render-to-cube-map

	m_CubeMapTexId = m_TM.CreateCubeMapTexture(i_FormatInternal, i_FormatExternal, i_FormatType, i_Width, i_Height, i_WrapType, i_FilterType, nullptr, i_StartTexUnitID, i_MipMapCount);

/*
	This function can be used to generate a cube map, to achive this 2 appraoches can be followed:
	1) we attach each face of the cubemap to the fbo, we still need to:
	   setup the required draw buffer, rotate the camera and render for each of the cubemap face
	   so, we render to the quad 6 times per frame

	2) we need an additional geometry shader to redirect the image to the required face of the cubemap
	   using the geometry shader we need to render to the quad only once per frame
*/
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, m_DrawBuffers[0], m_CubeMapTexId, 0);

	glDrawBuffer(m_DrawBuffers[0]);
	//

	SetupDepthBuffer(i_DepthBufferType, i_Width, i_Height);

	CheckCompletenessStatus();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferManager::GenAndBindFramebuffer()
{
	glGenFramebuffers(1, &m_FBOID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBOID);
}

void FrameBufferManager::SetupDepthBuffer ( DEPTH_BUFFER_TYPE i_DepthBufferType, unsigned short i_Width, unsigned short i_Height )
{
	m_DepthBufferType = i_DepthBufferType;

	switch (i_DepthBufferType)
	{
		case DEPTH_BUFFER_TYPE::DBT_RENDER_BUFFER_DEPTH:
		{
			glGenRenderbuffers(1, &m_RBOID);
			glBindRenderbuffer(GL_RENDERBUFFER, m_RBOID);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, i_Width, i_Height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBOID);
		}
		break;
		case DEPTH_BUFFER_TYPE::DBT_RENDER_BUFFER_DEPTH_STENCIL:
		{
			glGenRenderbuffers(1, &m_RBOID);
			glBindRenderbuffer(GL_RENDERBUFFER, m_RBOID);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, i_Width, i_Height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RBOID);
		}
		break;
	case DEPTH_BUFFER_TYPE::DBT_TEXTURE_DEPTH:
	{
		// we don't need to sample from it, so we pass -1 as the tex unit id!
		m_DepthTexID = m_TM.Create2DTexture(GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, i_Width, i_Height, GL_REPEAT, GL_NEAREST, nullptr, -1, -1, false);
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthTexID, 0);
	}
	break;
	case DEPTH_BUFFER_TYPE::DBT_TEXTURE_DEPTH_STENCIL:
	{
		// we don't need to sample from it, so we pass -1 as the tex unit id!
		m_DepthTexID = m_TM.Create2DTexture(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, i_Width, i_Height, GL_REPEAT, GL_NEAREST, nullptr, -1, -1, false);
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, m_DepthTexID, 0);
	}
	break;
	default:  // DEPTH_BUFFER_TYPE::DBT_NO_DEPTH
		break;
	}
}

void FrameBufferManager::SetupDrawBuffers (unsigned short i_ColorAttachmentCount, unsigned short i_Index )
{

	if (i_ColorAttachmentCount == 0 || i_ColorAttachmentCount >= m_MaxColorAttachments)
	{
		ERR("Number of color attachments exceeds the supported level!");
		return;
	}

	// NOTE! FBO must be bound before calling this function !
	CheckBufferBinding();

	assert((unsigned short)(i_Index + i_ColorAttachmentCount) <= m_DrawBuffers.size());

	glDrawBuffers(i_ColorAttachmentCount, &m_DrawBuffers[i_Index]);
}

void FrameBufferManager::AttachCubeMapFace ( unsigned short i_FaceIndex )
{
	if (i_FaceIndex > 5)
	{
		ERR("Invalid face index! Index must not exceed the value of 5 !");
		return;
	}

	// NOTE! FBO must be bound before calling this function !
	CheckBufferBinding();

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i_FaceIndex, m_CubeMapTexId, 0);

	CheckCompletenessStatus();
}

void FrameBufferManager::CheckCompletenessStatus(void)
{
	int error = 0;
	if ((error = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &m_FBOID);
		m_FBOID = 0;
		ERR("The Auxiliary Framebuffer is incomplete! " + error);
		return;
	}
}

void FrameBufferManager::CheckBufferBinding(void)
{
	int drawFboId = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
	if (drawFboId != m_FBOID)
	{
		ERR("The default framebuffer is bound! A FBO must be bound to use this function!");
		return;
	}
}

void FrameBufferManager::RenderToQuad ( void ) const
{
	m_QuadMBM.BindModelContext();

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	m_QuadMBM.UnBindModelContext();
}

void FrameBufferManager::Bind ( void ) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBOID);
}

void FrameBufferManager::UnBind ( void ) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void FrameBufferManager::BindColorAttachmentByTexId ( unsigned short i_TexId, bool i_GenerateMipMaps ) const
{
	m_TM.BindTexture(i_TexId, i_GenerateMipMaps);
}

void FrameBufferManager::BindColorAttachmentByIndex ( unsigned short i_Index, bool i_GenerateMipMaps, short i_TexUnitId ) const
{
	m_TM.BindTexture(m_TM.GetTextureId(i_Index), i_GenerateMipMaps, i_TexUnitId);
}

void FrameBufferManager::UpdateColorAttachmentSize(unsigned short i_Index, unsigned short i_Width, unsigned short i_Height)
{
	m_TM.Update2DTextureSize(m_TM.GetTextureId(i_Index), i_Width, i_Height);
}

void FrameBufferManager::UpdateDepthBufferSize(unsigned short i_Width, unsigned short i_Height)
{
	m_TM.Update2DTextureSize(m_TM.GetDepthTextureId(), i_Width, i_Height);

	switch (m_DepthBufferType)
	{
	case DEPTH_BUFFER_TYPE::DBT_RENDER_BUFFER_DEPTH:
	{
		glBindRenderbuffer(GL_RENDERBUFFER, m_RBOID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, i_Width, i_Height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	break;
	case DEPTH_BUFFER_TYPE::DBT_RENDER_BUFFER_DEPTH_STENCIL:
	{
		glBindRenderbuffer(GL_RENDERBUFFER, m_RBOID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, i_Width, i_Height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	break;
	case DEPTH_BUFFER_TYPE::DBT_TEXTURE_DEPTH:
	{
		m_TM.Update2DTextureSize(m_DepthTexID, i_Width, i_Height);
	}
	break;
	case DEPTH_BUFFER_TYPE::DBT_TEXTURE_DEPTH_STENCIL:
	{
		m_TM.Update2DTextureSize(m_DepthTexID, i_Width, i_Height);
	}
	break;
	default:  // DEPTH_BUFFER_TYPE::DBT_NO_DEPTH
		break;
	}
}

unsigned int FrameBufferManager::GetQuadVBOID ( void ) const
{
	return m_QuadMBM.GetVBOID();
}

MeshBufferManager::ACCESS_TYPE FrameBufferManager::GetQuadAccessType ( void ) const
{
	return m_QuadMBM.GetAccessType();
}

unsigned int FrameBufferManager::GetColorAttachmentTexId ( unsigned short i_Index ) const
{
	return m_TM.GetTextureId(i_Index);
}

unsigned short FrameBufferManager::GetColorAttachmentTexUnitId ( unsigned short i_Index ) const
{
	return m_TM.GetTextureUnitId(i_Index);
}