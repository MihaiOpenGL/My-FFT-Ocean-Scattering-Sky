/* 
 Author: BAIRAC MIHAI
*/

#include "Model.h"
#include "Material.h"

#include <assert.h>
#include <sstream>

#include "GL/glew.h"
#include "glm/glm.hpp"

#include "ErrorHandler.h"


Model::Model ( void )
	: m_Name("")
{
}

Model::Model ( const std::string& i_Name, const std::string& i_Path, bool i_UseMaterial, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, bool i_UseFlattenedModel )
	: m_Name(i_Name)
{
	m_TM.Initialize(i_Name);

	LoadModel(i_Name, i_Path, i_UseMaterial, i_ModelVertexAttributes, i_UseFlattenedModel);
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
			delete m_Meshes[i];
	}

	LOG("[" + m_Name + "] Model has been destroyed successfully!");
}

bool Model::LoadModel ( const std::string& i_Name, const std::string& i_Path, bool i_UseMaterial, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, bool i_UseFlattenedModel )
{
	m_Name = i_Name;

	std::vector<glm::vec3> positions, normals;
	std::vector<glm::vec2> uvs;

	std::vector<unsigned int> positionIndices, normalIndices, uvIndices;

	std::vector<unsigned int> meshVertexCount;
	unsigned short meshCrrIndex = 0;
	unsigned int prevSize = 0;


	FILE* pObjFile = nullptr;
	// i_Path - path to the obj file
	pObjFile = fopen(i_Path.c_str(), "r");
	if (pObjFile == nullptr)
	{
		ERR("Failed to open the obj file: " + i_Path + "!\n");
		return false;
	}

	std::string directory = i_Path.substr(0, i_Path.find_last_of('/'));


	std::map<std::string, Material::MaterialDetails> materialMap;
	// mesh id, material name
	std::map<unsigned short, std::string> meshMaterialMap;


	while (true)
	{
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(pObjFile, "%s", lineHeader);
		if (res == EOF)
		{
			// save the last data also
			meshVertexCount.push_back(positionIndices.size() - prevSize);

			++meshCrrIndex;

			break;
		}

		if (strcmp(lineHeader, "mtllib") == 0) // material file
		{
			if (i_UseMaterial)
			{
				char mtlFilePath[128];

				fscanf(pObjFile, "%s\n", &mtlFilePath);

				std::string mtlFN = directory + "/" + std::string(mtlFilePath);
				FILE* pMtlFile = fopen(mtlFN.c_str(), "r");
				if (pMtlFile == nullptr)
				{
					ERR("Failed to open the mtl file: " + mtlFN + "!\n");
					return false;
				}


				Material::MaterialDetails materialDetails;
				materialDetails.Id = 0;
				unsigned short materialCrrIndex = 0;

				while (true)
				{
					char lineHeader[128];

					int res = fscanf(pMtlFile, "%s", lineHeader);
					if (res == EOF)
					{
						// gather previous material data
						materialMap[materialDetails.Name] = materialDetails;

						++materialCrrIndex;

						break;
					}

					if (strcmp(lineHeader, "newmtl") == 0) // material name
					{
						char mtlName[128];

						fscanf(pMtlFile, "%s\n", &mtlName);

						// gather previous material data
						if (materialCrrIndex > 0)
						{
							materialMap[materialDetails.Name] = materialDetails;
						}

						materialDetails.Id = materialCrrIndex;
						materialDetails.Name = mtlName;

						materialDetails.TextureArray.clear(); //reset texture array

						++materialCrrIndex;
					}
					else if (strcmp(lineHeader, "map_Ka") == 0) // material ambient texture map
					{
						char map_Ka[128];

						fscanf(pMtlFile, "%s\n", &map_Ka);

						std::string texPath(directory + "/" + std::string(map_Ka));

						unsigned int texId = m_TM.Load2DTexture(texPath.c_str(), GL_REPEAT, GL_LINEAR, true);

						Material::TextureData texData;
						texData.Id = texId;
						texData.Type = Material::TEXTURE_MAP_TYPE::TMT_AMBIENT;

						materialDetails.TextureArray.push_back(texData);
					}
					else if (strcmp(lineHeader, "map_Kd") == 0) // material diffuse texture map
					{
						char map_Kd[128];

						fscanf(pMtlFile, "%s\n", &map_Kd);

						std::string texPath(directory + "/" + std::string(map_Kd));

						unsigned int texId = m_TM.Load2DTexture(texPath.c_str(), GL_REPEAT, GL_LINEAR, true);

						Material::TextureData texData;
						texData.Id = texId;
						texData.Type = Material::TEXTURE_MAP_TYPE::TMT_DIFFUSE;

						materialDetails.TextureArray.push_back(texData);
					}
					else if (strcmp(lineHeader, "map_Ks") == 0) // material specular texture map
					{
						char map_Ks[128];

						fscanf(pMtlFile, "%s\n", &map_Ks);

						std::string texPath(directory + "/" + std::string(map_Ks));

						unsigned int texId = m_TM.Load2DTexture(texPath.c_str(), GL_REPEAT, GL_LINEAR, true);

						Material::TextureData texData;
						texData.Id = texId;
						texData.Type = Material::TEXTURE_MAP_TYPE::TMT_SPECULAR;

						materialDetails.TextureArray.push_back(texData);
					}
					else if (strcmp(lineHeader, "map_Bump") == 0) // material normal texture map
					{
						char map_Bump[128];

						fscanf(pMtlFile, "%s\n", &map_Bump);

						std::string texPath(directory + "/" + std::string(map_Bump));

						unsigned int texId = m_TM.Load2DTexture(texPath.c_str(), GL_REPEAT, GL_LINEAR, false);

						Material::TextureData texData;
						texData.Id = texId;
						texData.Type = Material::TEXTURE_MAP_TYPE::TMT_NORMAL;

						materialDetails.TextureArray.push_back(texData);
					}
				}

				fclose(pMtlFile);
			}
		}
		else if (strcmp(lineHeader, "v") == 0) // vertex position
		{
			glm::vec3 position;
			fscanf(pObjFile, "%f %f %f\n", &position.x, &position.y, &position.z);
			positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vn") == 0) // vertex normal
		{
			glm::vec3 normal;
			fscanf(pObjFile, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "vt") == 0) // vertex texture coordinates
		{
			glm::vec2 uv;
			fscanf(pObjFile, "%f %f\n", &uv.x, &uv.y);
			uvs.push_back(uv);
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

			positionIndices.push_back(positionIndex[0]);
			positionIndices.push_back(positionIndex[1]);
			positionIndices.push_back(positionIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else if (strcmp(lineHeader, "g") == 0) // group
		{

		}
		else if (strcmp(lineHeader, "o") == 0) // object - aka mesh
		{
			if (meshCrrIndex > 0)
			{
				// gather previous mesh data
				meshVertexCount.push_back(positionIndices.size() - prevSize);
				prevSize = positionIndices.size();
			}

			++ meshCrrIndex;
		}
		else if (strcmp(lineHeader, "usemtl") == 0) // use material from material file
		{
			if (i_UseMaterial)
			{
				char mtlName[128];

				fscanf(pObjFile, "%s\n", &mtlName);

				std::map<std::string, Material::MaterialDetails>::iterator it = materialMap.find(std::string(mtlName));
				if (it != materialMap.end() && meshCrrIndex > 0)
				{
					meshMaterialMap[meshCrrIndex - 1] = it->second.Name;
				}
			}
		}
	}

	fclose(pObjFile);

	// create each mesh data
	unsigned int crrIndex = 0;
	for (unsigned int i = 0; i < meshVertexCount.size(); ++i)
	{
		std::vector<MeshBufferManager::VertexData> vertices;
		vertices.resize(meshVertexCount[i]);

		// add primary vertex attributes
		for (unsigned int j = 0, k = crrIndex; k < crrIndex + meshVertexCount[i]; ++j, ++k)
		{
			MeshBufferManager::VertexData vert;

			vert.position = positions[positionIndices[k] - 1]; //obj indexes start from 1, c++ array indexes from 0
			vert.normal = normals[normalIndices[k] - 1];
			vert.uv = uvs[uvIndices[k] - 1];

			vertices[j] = vert;
		}
		crrIndex += meshVertexCount[i];

		std::stringstream ss;
		ss << m_Name << " [Mesh " << i << "]";

		m_Meshes.push_back(new Mesh(ss.str(), vertices, i_ModelVertexAttributes, materialMap[meshMaterialMap[i]].TextureArray, i_UseMaterial, i_UseFlattenedModel));
	}

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

	return true;
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