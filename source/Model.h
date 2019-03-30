/* Author: BAIRAC MIHAI */

#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"
#include "Material.h"
#include "MeshBufferManager.h"
#include "TextureManager.h"
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
	bool ParseMTLFile(const std::string& i_MTLFileName, const std::string& i_Directory, std::map<std::string, Material>& o_MaterialMap);

	Material::TextureData ProcessTexture(const std::string& i_TexPath, const Material::TEXTURE_MAP_TYPE& i_TexType, const std::vector<Model::LoadedTextureData>& i_LoadedTextureVector);

	//// Variables ////
	std::string m_Name;

	std::vector<Mesh*> m_Meshes;
	TextureManager m_TM;
};

#endif /* MODEL_H */