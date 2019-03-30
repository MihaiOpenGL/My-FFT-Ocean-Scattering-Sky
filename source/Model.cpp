/* 
 Author: BAIRAC MIHAI
*/

#include "Model.h"
#include "CommonHeaders.h"
#include "Material.h"
#include "GLConfig.h"
#include "GlobalConfig.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include <limits>
#include <sstream>
#include <cstdio>
#include <cstring>


Model::Model ( void )
	: m_Name("Default")
{
	LOG("Model [%s] successfully created!", m_Name.c_str());
}

Model::Model ( const std::string& i_Name, const std::string& i_Path, bool i_UseMaterial, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, bool i_UseFlattenedModel, const GlobalConfig& i_Config)
	: m_Name(i_Name)
{
	Initialize(i_Name, i_Path, i_UseMaterial, i_ModelVertexAttributes, i_UseFlattenedModel, i_Config);
}

Model::~Model()
{
	Destroy();
}

void Model::Destroy ( void )
{
	for (unsigned int i = 0; i < m_Meshes.size(); ++ i)
	{
		if (m_Meshes[i])
		{
			delete m_Meshes[i];
		}
	}

	LOG("Model [%s] successfully destroyed!", m_Name.c_str());
}

bool Model::Initialize ( const std::string& i_Name, const std::string& i_ObjPath, bool i_UseMaterial, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, bool i_UseFlattenedModel, const GlobalConfig& i_Config)
{
	m_Name = i_Name;
	
	if (i_UseMaterial)
	{
		m_TM.Initialize(i_Name, i_Config);
	}

	ParseObjFile(i_ObjPath, i_UseMaterial, i_ModelVertexAttributes, i_UseFlattenedModel);

	// compute additional data like: 
	// model minX, maxX, minY, maxY, minZ, maxZ
	// model width, height and width
	m_Limits.MinX = std::numeric_limits<float>::max(); m_Limits.MaxX = 0.0f;
	m_Limits.MinY = std::numeric_limits<float>::max(), m_Limits.MaxY = 0.0f;
	m_Limits.MinZ = std::numeric_limits<float>::max(), m_Limits.MaxZ = 0.0f;
	for (unsigned short i = 0; i < m_Meshes.size(); ++i)
	{
		if (m_Meshes[i])
		{
			Mesh::Limits meshLimits = m_Meshes[i]->GetLimits();

			if (meshLimits.MinX < m_Limits.MinX) m_Limits.MinX = meshLimits.MinX;
			if (meshLimits.MaxX >= m_Limits.MaxX) m_Limits.MaxX = meshLimits.MaxX;

			if (meshLimits.MinY < m_Limits.MinY) m_Limits.MinY = meshLimits.MinY;
			if (meshLimits.MaxY >= m_Limits.MaxY) m_Limits.MaxY = meshLimits.MaxY;

			if (meshLimits.MinZ < m_Limits.MinZ) m_Limits.MinZ = meshLimits.MinZ;
			if (meshLimits.MaxZ >= m_Limits.MaxZ) m_Limits.MaxZ = meshLimits.MaxZ;
		}
	}

	m_Dimensions.Width = m_Limits.MaxX - m_Limits.MinX;
	m_Dimensions.Height = m_Limits.MaxY - m_Limits.MinY;
	m_Dimensions.Depth = m_Limits.MaxZ - m_Limits.MinZ;

	LOG("Model [%s] successfully created!", m_Name.c_str());

	return true;
}

bool Model::ParseObjFile(const std::string& i_ObjPath, bool i_UseMaterial, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, bool i_UseFlattenedModel)
{
	std::vector<Model::MeshData*> meshDataVector;
	std::map<std::string, Material> materialMap;
	int crrMeshIndex = -1;

	std::string directory = i_ObjPath.substr(0, i_ObjPath.find_last_of('/'));

	FILE* pObjFile = nullptr;
	// i_Path - path to the obj file
	pObjFile = fopen(i_ObjPath.c_str(), "r");
	if (pObjFile == nullptr)
	{
		ERR("Failed to open the obj file: %s!", i_ObjPath.c_str());
		return false;
	}

	while (true)
	{
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(pObjFile, "%s", lineHeader);
		if (res == EOF)
		{
			fclose(pObjFile);
			LOG("File %s has been successfully parsed!", i_ObjPath.c_str());
			break;
		}

		if (strcmp(lineHeader, "mtllib") == 0) // material file
		{
			if (i_UseMaterial)
			{
				char mtlFileName[128];
				fscanf(pObjFile, "%s\n", mtlFileName);

				ParseMTLFile(mtlFileName, directory, materialMap);
			}
		}
		else if (strcmp(lineHeader, "o") == 0) // object - aka mesh
		{
			char meshName[128];
			fscanf(pObjFile, "%s", meshName);

			meshDataVector.push_back(new MeshData(meshName));

			crrMeshIndex++;
		}
		else if (strcmp(lineHeader, "v") == 0) // vertex position
		{
			glm::vec3 position;
			fscanf(pObjFile, "%f %f %f\n", &position.x, &position.y, &position.z);
			
			assert(crrMeshIndex >= 0 && crrMeshIndex < meshDataVector.size());

			if (meshDataVector[crrMeshIndex])
			{
				meshDataVector[crrMeshIndex]->VertexPositions.push_back(position);
			}
		}
		else if (strcmp(lineHeader, "vn") == 0) // vertex normal
		{
			glm::vec3 normal;
			fscanf(pObjFile, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			
			assert(crrMeshIndex >= 0 && crrMeshIndex < meshDataVector.size());

			if (meshDataVector[crrMeshIndex])
			{
				meshDataVector[crrMeshIndex]->VertexNormals.push_back(normal);
			}
		}
		else if (strcmp(lineHeader, "vt") == 0) // vertex texture coordinates
		{
			glm::vec2 uv;
			fscanf(pObjFile, "%f %f\n", &uv.x, &uv.y);
			
			assert(crrMeshIndex >= 0 && crrMeshIndex < meshDataVector.size());

			if (meshDataVector[crrMeshIndex])
			{
				meshDataVector[crrMeshIndex]->VertexUVs.push_back(uv);
			}
		}
		else if (strcmp(lineHeader, "f") == 0) // face
		{
			unsigned int positionIndex[3], uvIndex[3], normalIndex[3], matches;
			fpos_t fpos;

			// f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
			fgetpos(pObjFile, &fpos);
			matches = fscanf(pObjFile, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &positionIndex[0], &uvIndex[0], &normalIndex[0], &positionIndex[1], &uvIndex[1], &normalIndex[1], &positionIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9)
			{
				fsetpos(pObjFile, &fpos);

				// f v1//vn1 v2//vn2 v3//vn3
				matches = fscanf(pObjFile, "%d//%d %d//%d %d//%d\n", &positionIndex[0], &normalIndex[0], &positionIndex[1], &normalIndex[1], &positionIndex[2], &normalIndex[2]);
				if (matches != 6)
				{
					fsetpos(pObjFile, &fpos);

					// f v1 v2 v3
					matches = fscanf(pObjFile, "%d %d %d\n", &positionIndex[0], &positionIndex[1], &positionIndex[2]);
					if (matches != 3)
					{
						ERR("File can't be read by our simple parser : ( Try exporting with other options\n");
						return false;
					}
				}
			}

			assert(crrMeshIndex >= 0 && crrMeshIndex < meshDataVector.size());

			if (meshDataVector[crrMeshIndex])
			{
				meshDataVector[crrMeshIndex]->PositionIndices.push_back(positionIndex[0]);
				meshDataVector[crrMeshIndex]->PositionIndices.push_back(positionIndex[1]);
				meshDataVector[crrMeshIndex]->PositionIndices.push_back(positionIndex[2]);
				meshDataVector[crrMeshIndex]->UVsIndices.push_back(uvIndex[0]);
				meshDataVector[crrMeshIndex]->UVsIndices.push_back(uvIndex[1]);
				meshDataVector[crrMeshIndex]->UVsIndices.push_back(uvIndex[2]);
				meshDataVector[crrMeshIndex]->NormalsIndices.push_back(normalIndex[0]);
				meshDataVector[crrMeshIndex]->NormalsIndices.push_back(normalIndex[1]);
				meshDataVector[crrMeshIndex]->NormalsIndices.push_back(normalIndex[2]);
			}
		}
		else if (strcmp(lineHeader, "usemtl") == 0) // use material from material file
		{
			if (i_UseMaterial)
			{
				char materialName[128];
				fscanf(pObjFile, "%s\n", materialName);

				assert(crrMeshIndex >= 0 && crrMeshIndex < meshDataVector.size());

				if (meshDataVector[crrMeshIndex])
				{
					meshDataVector[crrMeshIndex]->MaterialName = materialName;
				}
			}
		}
	}


	// process mesh & material data
	unsigned int crrPosIndexOffset = 0, crrNormalIndexOffset = 0, crrUVIndexOffset = 0;
	for (unsigned int i = 0; i < meshDataVector.size(); ++i)
	{
		const Model::MeshData* meshData = meshDataVector[i];

		if (meshData)
		{
			// indices size check
			if (meshData->PositionIndices.size() != meshData->NormalsIndices.size() &&
				meshData->PositionIndices.size() != meshData->UVsIndices.size())
			{
				ERR("Invalid indices for mesh %s", meshData->Name.c_str());
				return false;
			}

			std::vector<MeshBufferManager::VertexData> vertices;
			vertices.resize(meshData->PositionIndices.size());
			for (unsigned int j = 0; j < vertices.size(); ++j)
			{
				// obj indices start from 1, but in c++ indices - from 0
				// the indices value doesn't restart for each mesh, it just goes on increasing
				unsigned int idx = meshData->PositionIndices[j] - crrPosIndexOffset - 1;
				vertices[j].position = meshData->VertexPositions[idx];

				idx = meshData->NormalsIndices[j] - crrNormalIndexOffset - 1;
				vertices[j].normal = meshData->VertexNormals[idx];

				idx = meshData->UVsIndices[j] - crrUVIndexOffset - 1;
				vertices[j].uv = meshData->VertexUVs[idx];
			}

			crrPosIndexOffset += meshData->VertexPositions.size();
			crrNormalIndexOffset += meshData->VertexNormals.size();
			crrUVIndexOffset += meshData->VertexUVs.size();

			// create a mesh
			m_Meshes.push_back(new Mesh(meshData->Name, vertices, i_ModelVertexAttributes, materialMap[meshData->MaterialName].TextureArray, i_UseMaterial, i_UseFlattenedModel));
		}
	}

	// cleanup mesh data
	for (unsigned int i = 0; i < meshDataVector.size(); ++i)
	{
		SAFE_DELETE(meshDataVector[i]);
	}

	return true;
}

bool Model::ParseMTLFile(const std::string& i_MTLFileName, const std::string& i_Directory, std::map<std::string, Material>& o_MaterialMap)
{
	std::string mtlFilePath = i_Directory + "/" + i_MTLFileName;

	FILE* pMtlFile = fopen(mtlFilePath.c_str(), "r");
	if (pMtlFile == nullptr)
	{
		ERR("Failed to open the mtl file: %s!", mtlFilePath.c_str());
		return false;
	}

	std::vector<Model::LoadedTextureData> loadedTextureVector;

	std::string crrMaterialName;

	while (true)
	{
		char lineHeader[128];

		int res = fscanf(pMtlFile, "%s", lineHeader);
		if (res == EOF)
		{
			fclose(pMtlFile);
			LOG("File %s has been successfully parsed!", mtlFilePath.c_str());
			break;
		}

		if (strcmp(lineHeader, "newmtl") == 0) // material name
		{
			char mtlName[128];
			fscanf(pMtlFile, "%s\n", mtlName);

			crrMaterialName = mtlName;

			Material material(crrMaterialName);
			o_MaterialMap[crrMaterialName] = material;
		}
		else if (strcmp(lineHeader, "map_Ka") == 0) // material ambient texture map
		{
			char map_Ka[128];
			fscanf(pMtlFile, "%s\n", map_Ka);

			std::string texPath(i_Directory + "/" + std::string(map_Ka));

			Material::TextureData texData = ProcessTexture(texPath, Material::TEXTURE_MAP_TYPE::TMT_AMBIENT, loadedTextureVector);

			o_MaterialMap[crrMaterialName].TextureArray.push_back(texData);
		}
		else if (strcmp(lineHeader, "map_Kd") == 0) // material diffuse texture map
		{
			char map_Kd[128];
			fscanf(pMtlFile, "%s\n", map_Kd);

			std::string texPath(i_Directory + "/" + std::string(map_Kd));

			Material::TextureData texData = ProcessTexture(texPath, Material::TEXTURE_MAP_TYPE::TMT_DIFFUSE, loadedTextureVector);

			o_MaterialMap[crrMaterialName].TextureArray.push_back(texData);
		}
		else if (strcmp(lineHeader, "map_Ks") == 0) // material specular texture map
		{
			char map_Ks[128];
			fscanf(pMtlFile, "%s\n", map_Ks);

			std::string texPath(i_Directory + "/" + std::string(map_Ks));

			Material::TextureData texData = ProcessTexture(texPath, Material::TEXTURE_MAP_TYPE::TMT_SPECULAR, loadedTextureVector);

			o_MaterialMap[crrMaterialName].TextureArray.push_back(texData);
		}
		else if (strcmp(lineHeader, "map_Bump") == 0) // material normal texture map
		{
			char map_Bump[128];
			fscanf(pMtlFile, "%s\n", map_Bump);

			std::string texPath(i_Directory + "/" + std::string(map_Bump));

			Material::TextureData texData = ProcessTexture(texPath, Material::TEXTURE_MAP_TYPE::TMT_NORMAL, loadedTextureVector);

			o_MaterialMap[crrMaterialName].TextureArray.push_back(texData);
		}
	}

	return true;
}

Material::TextureData Model::ProcessTexture(const std::string& i_TexPath, const Material::TEXTURE_MAP_TYPE& i_TexType, const std::vector<Model::LoadedTextureData>& i_LoadedTextureVector)
{
	Material::TextureData texData;

	bool skip = false;
	for (unsigned int i = 0; i < i_LoadedTextureVector.size(); ++i)
	{
		if (i_TexPath == i_LoadedTextureVector[i].Path)
		{
			skip = true;

			texData.Id = i_LoadedTextureVector[i].Id;
			texData.Type = i_LoadedTextureVector[i].Type;

			break;
		}
	}

	if (!skip)
	{
		unsigned int texId = m_TM.Load2DTexture(i_TexPath.c_str(), GL_REPEAT, GL_LINEAR, true);

		texData.Id = texId;
		texData.Type = i_TexType;
	}

	return texData;
}

void Model::Render ( const ShaderManager& i_SM, unsigned short i_StartTexUnitId, bool i_IsWireframeMode)
{
	for (unsigned int i = 0; i < m_Meshes.size(); ++i)
	{
		if (m_Meshes[i])
		{
			m_Meshes[i]->Render(i_SM, m_TM, i_StartTexUnitId, i_IsWireframeMode);
		}
	}
}

void Model::Render ( bool i_IsWireframeMode )
{
	for (unsigned int i = 0; i < m_Meshes.size(); ++i)
	{
		if (m_Meshes[i])
		{
			m_Meshes[i]->Render(i_IsWireframeMode);
		}
	}
}

void Model::RenderFlattened ( void )
{
	for (unsigned int i = 0; i < m_Meshes.size(); ++i)
	{
		if (m_Meshes[i])
		{
			m_Meshes[i]->RenderFlattened();
		}
	}
}

const Model::Limits& Model::GetLimits ( void ) const
{
	return m_Limits;
}

const Model::Dimensions& Model::GetDimensions ( void ) const
{
	return m_Dimensions;
}

float Model::GetWidth ( void ) const
{
	return m_Dimensions.Width;
}

float Model::GetHeight ( void ) const
{
	return m_Dimensions.Height;
}

float Model::GetDepth ( void ) const
{
	return m_Dimensions.Depth;
}