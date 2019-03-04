/* Author: BAIRAC MIHAI */

#ifndef FRAME_BUFFER_MANAGER_H
#define FRAME_BUFFER_MANAGER_H

#include <string>
#include <vector>
#include "MeshBufferManager.h"
#include "TextureManager.h"

/*
 Manager for custom framebuffers
 Color and depth types are both supported!
*/

class FrameBufferManager
{
public:
	enum class DEPTH_BUFFER_TYPE
	{
		DBT_RENDER_BUFFER_DEPTH = 0,
		DBT_RENDER_BUFFER_DEPTH_STENCIL,
		DBT_TEXTURE_DEPTH,
		DBT_TEXTURE_DEPTH_STENCIL,
		DBT_NO_DEPTH,
		DBT_COUNT
	};

	FrameBufferManager(void);
	FrameBufferManager(const std::string& i_Name);
	~FrameBufferManager(void);

	void Initialize(const std::string& i_Name);

	// i_MipMapCount can have the values: 
	// -1 - no mipmaps
	// 0 - use mipmaps (full range)
	// n - use mipmaps down to range [size .. n]
	void CreateSimple(unsigned short i_ColorAttachmentCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_StartTexUnitID, short i_MipMapCount = -1, bool i_AnisoFiltering = true, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType = FrameBufferManager::DEPTH_BUFFER_TYPE::DBT_NO_DEPTH);
	void CreateSimple(const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, unsigned short i_ColorAttachmentCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_StartTexUnitID, short i_MipMapCount = -1, bool i_AnisoFiltering = true, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType = FrameBufferManager::DEPTH_BUFFER_TYPE::DBT_NO_DEPTH);

	void CreateLayered(unsigned short i_ColorAttachmentCount, unsigned short i_LayerCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, short i_StartTexUnitID, short i_MipMapCount = -1, bool i_AnisoFiltering = true, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType = FrameBufferManager::DEPTH_BUFFER_TYPE::DBT_NO_DEPTH);
	void CreateLayered(const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, unsigned short i_ColorAttachmentCount, unsigned short i_LayerCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, short i_StartTexUnitID, short i_MipMapCount = -1, bool i_AnisoFiltering = true, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType = FrameBufferManager::DEPTH_BUFFER_TYPE::DBT_NO_DEPTH);

	void CreateLayered(unsigned int i_ColorAttachmentTexId, unsigned short i_ColorAttachmentLayerCount);
	void CreateLayered(const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, unsigned int i_ColorAttachmentTexId, unsigned short i_ColorAttachmentLayerCount);

	void CreateCubeMaped(unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_StartTexUnitID, short i_MipMapCount = -1, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType = FrameBufferManager::DEPTH_BUFFER_TYPE::DBT_NO_DEPTH);
	void CreateCubeMaped(const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_StartTexUnitID, short i_MipMapCount = -1, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType = FrameBufferManager::DEPTH_BUFFER_TYPE::DBT_NO_DEPTH);

	void SetupDrawBuffers(unsigned short i_ColorAttachmentCount, unsigned short i_Index = 0);

	void AttachCubeMapFace(unsigned short i_FaceIndex);

	void CheckCompletenessStatus(void);

	void CheckBufferBinding(void);

	void RenderToQuad(void) const;

	void Bind(void) const;
	void UnBind(void) const;

	void BindColorAttachmentByTexId(unsigned short i_TexId, bool i_GenerateMipMaps = false) const;
	void BindColorAttachmentByIndex(unsigned short i_Index, bool i_GenerateMipMaps = false, short i_TexUnitId = -1) const;

	void UpdateColorAttachmentSize(unsigned short i_Index, unsigned short i_Width, unsigned short i_Height);
	void UpdateDepthBufferSize(unsigned short i_Width, unsigned short i_Height);

	unsigned int GetQuadVBOID(void) const;
	MeshBufferManager::ACCESS_TYPE GetQuadAccessType(void) const;
	unsigned int GetColorAttachmentTexId(unsigned short i_Index) const;
	unsigned short GetColorAttachmentTexUnitId(unsigned short i_Index) const;

private:
	//// Methods ////
	void Init(void);
	void Destroy(void);

	void CreateQuadModel(void);
	void CreateQuadModel(const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes);

	void SetupSimple(unsigned short i_ColorAttachmentCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_StartTexUnitID, short i_MipMapCount, bool i_AnisoFiltering, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType);
	void SetupLayered(unsigned short i_ColorAttachmentCount, unsigned short i_LayerCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_StartTexUnitID, short i_MipMapCount, bool i_AnisoFiltering, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType);
	void SetupLayered(unsigned int i_ColorAttachmentTexId, unsigned short i_ColorAttachmentLayerCount);
	void SetupCubeMaped(unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_StartTexUnitID, short i_MipMapCount, FrameBufferManager::DEPTH_BUFFER_TYPE i_DepthBufferType);

	void GenAndBindFramebuffer();

	void SetupDepthBuffer(DEPTH_BUFFER_TYPE i_DepthBufferType, unsigned short i_Width, unsigned short i_Height);

	//// Variables ////
	std::string m_Name;

	static int m_MaxColorAttachments;

	MeshBufferManager m_QuadMBM;
	TextureManager m_TM;

	unsigned int m_FBOID;
	unsigned int m_RBOID;
	unsigned int m_DepthTexID;

	DEPTH_BUFFER_TYPE m_DepthBufferType;

	std::vector<unsigned int> m_DrawBuffers;

	unsigned int m_CubeMapTexId;
};

#endif /* FRAME_BUFFER_MANAGER_H */
