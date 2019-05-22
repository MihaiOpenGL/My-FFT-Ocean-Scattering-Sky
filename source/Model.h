/* Author: BAIRAC MIHAI */

#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"
#include "Material.h"
#include "MeshBufferManager.h"
#include "TextureManager.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include <string>
#include <vector>

class GlobalConfig;

/*
 A simple Wavefront OBJ model class
*/

class Model
{
public:
	struct Limits
	{
		float MinX;
		float MaxX;
		float MinY;
		float MaxY;
		float MinZ;
		float MaxZ;
	} m_Limits;

	struct Dimensions
	{
		float Width;
		float Height;
		float Depth;
	} m_Dimensions;

	Model(void);
	Model(const std::string& i_Name, const std::string& i_Path, bool i_UseMaterial, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, bool i_UseFlattenedModel, const GlobalConfig& i_Config);
	~Model(void);

	bool Initialize(const std::string& i_Name, const std::string& i_Path, bool i_UseMaterial, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, bool i_UseFlattenedModel, const GlobalConfig& i_Config);

	void Render(const ShaderManager& i_SM, unsigned short i_StartTexUnitId, bool i_IsWireframeMode);
	void Render(bool i_IsWireframeMode);
	void RenderFlattened(void);

	const Model::Limits& GetLimits(void) const;
	const Model::Dimensions& GetDimensions(void) const;
	float GetWidth(void) const;
	float GetHeight(void) const;
	float GetDepth(void) const;

private:
	struct MeshData
	{
		MeshData(const std::string& name)
			: Name(name)
		{}

		std::string Name;
		std::vector<glm::vec3> VertexPositions;
		std::vector<glm::vec3> VertexNormals;
		std::vector<glm::vec2> VertexUVs;

		std::vector<unsigned int> PositionIndices;
		std::vector<unsigned int> NormalsIndices;
		std::vector<unsigned int> UVsIndices;

		std::string MaterialName;
	};

	struct LoadedTextureData
	{
		unsigned int Id;
		Material::TEXTURE_MAP_TYPE Type;
		std::string Path;
	};

	///// Methods ////
	void Destroy(void);

	bool ParseObjFile(const std::string& i_ObjPath, bool i_UseMaterial, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, bool i_UseFlattenedModel);
	void ParseOBJLine(const std::string& i_Line, std::vector<Model::MeshData*>& io_MeshDataVector, int& io_CrrMeshIndex,
		bool i_UseMaterial, std::map<std::string, Material>& o_MaterialMap, const std::string& i_DirectoryName);

	void ParseVertexToken(size_t& io_LineFoundSepPos, size_t& io_LineCrrSearchPos, const std::string& i_TokenSep,
		const std::string& i_Line, size_t& io_Idx, glm::vec3& o_Out, size_t i_CustomSize);
	void ParseFaceToken(size_t& io_LineFoundSepPos, size_t& io_LineCrrSearchPos, const std::string& i_TokenSep,
		const std::string& i_Line, size_t& io_FaceIdx, glm::ivec3& o_VertexIdx, glm::ivec3& o_UVIdx, glm::ivec3& o_NormalIdx);
	void ParseFaceSubToken(size_t& io_TokenFoundSepPos, size_t& io_TokenCrrSearchPos, const std::string& i_TokenSep, const std::string& i_Token,
		size_t i_FaceIdx, size_t& io_TypeIdx, glm::ivec3& o_VertexIdx, glm::ivec3& o_UVIdx, glm::ivec3& o_NormalIdx);

	bool ParseMTLFile(const std::string& i_MTLFileName, const std::string& i_Directory, std::map<std::string, Material>& o_MaterialMap);
	void ParseMTLLine(const std::string& i_Line, std::map<std::string, Material>& o_MaterialMap, std::string& io_CrrMaterialName,
		std::vector<Model::LoadedTextureData>& io_LoadedTextureVector, const std::string& i_TexBaseName);

	Material::TextureData ProcessTexture(const std::string& i_TexPath, const Material::TEXTURE_MAP_TYPE& i_TexType, std::vector<Model::LoadedTextureData>& io_LoadedTextureVector);

	//// Variables ////
	std::string m_Name;

	std::vector<Mesh*> m_Meshes;
	TextureManager m_TM;
};

#endif /* MODEL_H */