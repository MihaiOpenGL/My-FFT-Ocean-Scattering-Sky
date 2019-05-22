/* 
 Author: BAIRAC MIHAI
*/

#include "Model.h"
#include "CommonHeaders.h"
#include "Material.h"
#include "GLConfig.h"
// glm::vec2, glm::vec2 come from the header
#include "GlobalConfig.h"
#include "SDL/SDL_rwops.h"
#include "SDL/SDL_filesystem.h"
#include <limits>
#include <sstream>
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
	int crrMeshIndex = -1;
	std::vector<Model::MeshData*> meshDataVector;
	std::map<std::string, Material> materialMap;

	std::string directoryName = i_ObjPath.substr(0, i_ObjPath.find_last_of('/'));

	char* pBasePath = SDL_GetBasePath();
	std::string fileName(pBasePath ? pBasePath : "");
	fileName += i_ObjPath;
	SDL_RWops* pObjFile = SDL_RWFromFile(fileName.c_str(), "rb");
	if (pObjFile == nullptr)
	{
		ERR("Failed to open the obj file: %s!", i_ObjPath.c_str());
		return false;
	}

	size_t fileSize = (size_t)SDL_RWsize(pObjFile);

	char* pFileContent = new char[fileSize + 1];

	size_t nbRead = (size_t)SDL_RWread(pObjFile, pFileContent, sizeof(char), fileSize);
	if (nbRead != fileSize)
	{
		ERR("Mismatch of bytes read vs file size in bytes!");
		return false;
	}

	pFileContent[fileSize] = '\0';

	std::string objContent(pFileContent);
	SAFE_ARRAY_DELETE(pFileContent);

	// process OBJ file
	size_t fileCrrSearchPos = 0, fileFoundSepPos = 0;
	std::string lineSep("\r\n");

	std::string line;
	while (std::string::npos != (fileFoundSepPos = objContent.find_first_of(lineSep, fileCrrSearchPos)))
	{
		size_t lineLength = fileFoundSepPos - fileCrrSearchPos;

		line = objContent.substr(fileCrrSearchPos, lineLength);

		if (!line.empty())
		{
			ParseOBJLine(line, meshDataVector, crrMeshIndex, i_UseMaterial, materialMap, directoryName);
		}

		fileCrrSearchPos = fileFoundSepPos + lineSep.size();
	}

	//process last line (after separator)
	line = objContent.substr(fileCrrSearchPos);
	if (!line.empty())
	{
		ParseOBJLine(line, meshDataVector, crrMeshIndex, i_UseMaterial, materialMap, directoryName);
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

void Model::ParseOBJLine(const std::string& i_Line, std::vector<Model::MeshData*>& io_MeshDataVector, int& io_CrrMeshIndex,
	bool i_UseMaterial, std::map<std::string, Material>& o_MaterialMap, const std::string& i_DirectoryName)
{
	// process line
	size_t lineCrrSearchPos = 0, lineFoundSepPos = 0;
	std::string tokenSep(" ");

	// get first token
	lineFoundSepPos = i_Line.find_first_of(tokenSep, lineCrrSearchPos);
	size_t tokenLength = lineFoundSepPos - lineCrrSearchPos;

	std::string token = i_Line.substr(lineCrrSearchPos, tokenLength);
	// filter out unneeded lines
	if (token.empty() || (token != "mtllib" && token != "o" &&
		token != "v" && token != "vn" && token != "vt" &&
		token != "f" && token != "usemtl"))
	{
		return; //jump to the next line
	}

	lineCrrSearchPos = lineFoundSepPos + tokenSep.size();

	if (token == "mtllib") // material file
	{
		if (i_UseMaterial)
		{
			//get the name of the material file
			token = i_Line.substr(lineCrrSearchPos);

			if (!token.empty())
			{
				ParseMTLFile(token, i_DirectoryName, o_MaterialMap);
			}
		}
	}
	else if (token == "o") // object - aka mesh
	{
		//the name of the object - mesh
		token = i_Line.substr(lineCrrSearchPos);

		if (!token.empty())
		{
			io_MeshDataVector.push_back(new MeshData(token));

			io_CrrMeshIndex++;
		}
	}
	else if (token == "v") // vertex position
	{
		glm::vec3 position;
		size_t idx = 0;

		ParseVertexToken(lineFoundSepPos, lineCrrSearchPos, tokenSep, i_Line, idx, position, 3);

		assert(io_CrrMeshIndex >= 0 && io_CrrMeshIndex < io_MeshDataVector.size());

		if (io_MeshDataVector[io_CrrMeshIndex])
		{
			io_MeshDataVector[io_CrrMeshIndex]->VertexPositions.push_back(position);
		}
	}
	else if (token == "vn") // vertex normal
	{
		glm::vec3 normal;
		size_t idx = 0;

		ParseVertexToken(lineFoundSepPos, lineCrrSearchPos, tokenSep, i_Line, idx, normal, 3);

		assert(io_CrrMeshIndex >= 0 && io_CrrMeshIndex < io_MeshDataVector.size());

		if (io_MeshDataVector[io_CrrMeshIndex])
		{
			io_MeshDataVector[io_CrrMeshIndex]->VertexNormals.push_back(normal);
		}
	}
	else if (token == "vt") // vertex texture coordinates
	{
		glm::vec2 uv;
		glm::vec3 uv_;
		size_t idx = 0;

		ParseVertexToken(lineFoundSepPos, lineCrrSearchPos, tokenSep, i_Line, idx, uv_, 2);

		uv.x = uv_.x;
		uv.y = uv_.y;

		assert(io_CrrMeshIndex >= 0 && io_CrrMeshIndex < io_MeshDataVector.size());

		if (io_MeshDataVector[io_CrrMeshIndex])
		{
			io_MeshDataVector[io_CrrMeshIndex]->VertexUVs.push_back(uv);
		}
	}
	else if (token == "f") // face
	{
		glm::ivec3 positionIndex, uvIndex, normalIndex;
		size_t faceIdx = 0;

		ParseFaceToken(lineFoundSepPos, lineCrrSearchPos, tokenSep, i_Line, faceIdx, positionIndex, uvIndex, normalIndex);

		assert(io_CrrMeshIndex >= 0 && io_CrrMeshIndex < io_MeshDataVector.size());

		if (io_MeshDataVector[io_CrrMeshIndex])
		{
			io_MeshDataVector[io_CrrMeshIndex]->PositionIndices.push_back(positionIndex[0]);
			io_MeshDataVector[io_CrrMeshIndex]->PositionIndices.push_back(positionIndex[1]);
			io_MeshDataVector[io_CrrMeshIndex]->PositionIndices.push_back(positionIndex[2]);
			io_MeshDataVector[io_CrrMeshIndex]->UVsIndices.push_back(uvIndex[0]);
			io_MeshDataVector[io_CrrMeshIndex]->UVsIndices.push_back(uvIndex[1]);
			io_MeshDataVector[io_CrrMeshIndex]->UVsIndices.push_back(uvIndex[2]);
			io_MeshDataVector[io_CrrMeshIndex]->NormalsIndices.push_back(normalIndex[0]);
			io_MeshDataVector[io_CrrMeshIndex]->NormalsIndices.push_back(normalIndex[1]);
			io_MeshDataVector[io_CrrMeshIndex]->NormalsIndices.push_back(normalIndex[2]);
		}
	}
	else if (token == "usemtl") // use material from material file
	{
		if (i_UseMaterial)
		{
			//get the name of the material file
			token = i_Line.substr(lineCrrSearchPos);

			if (!token.empty())
			{
				assert(io_CrrMeshIndex >= 0 && io_CrrMeshIndex < io_MeshDataVector.size());

				if (io_MeshDataVector[io_CrrMeshIndex])
				{
					io_MeshDataVector[io_CrrMeshIndex]->MaterialName = token;
				}
			}
		}
	}
}

void Model::ParseVertexToken(size_t& io_LineFoundSepPos, size_t& io_LineCrrSearchPos, const std::string& i_TokenSep,
							  const std::string& i_Line, size_t& io_Idx, glm::vec3& o_Out, size_t i_CustomSize)
{
	io_LineFoundSepPos = i_Line.find_first_of(i_TokenSep, io_LineCrrSearchPos);

	std::string token;
	size_t tokenLength;

	if (io_LineFoundSepPos == std::string::npos) // last token
	{
		tokenLength = i_Line.size();
	}
	else
	{
		tokenLength = io_LineFoundSepPos - io_LineCrrSearchPos;
	}

	token = i_Line.substr(io_LineCrrSearchPos, tokenLength);
	if (!token.empty())
	{
		o_Out[io_Idx] = std::stof(token);
	}

	if (io_LineFoundSepPos != std::string::npos) // still parsing
	{
		assert(i_CustomSize <= o_Out.length());

		if (io_Idx < i_CustomSize)
		{
			io_Idx++;
		}
		else
		{
			io_Idx = i_CustomSize - 1;
		}

		io_LineCrrSearchPos = io_LineFoundSepPos + i_TokenSep.size();

		ParseVertexToken(io_LineFoundSepPos, io_LineCrrSearchPos, i_TokenSep, i_Line, io_Idx, o_Out, i_CustomSize);
	}
}

void Model::ParseFaceToken(size_t& io_LineFoundSepPos, size_t& io_LineCrrSearchPos, const std::string& i_TokenSep,
						  const std::string& i_Line, size_t& io_FaceIdx, glm::ivec3& o_VertexIdx, glm::ivec3& o_UVIdx, glm::ivec3& o_NormalIdx)
{
	io_LineFoundSepPos = i_Line.find_first_of(i_TokenSep, io_LineCrrSearchPos);

	std::string token;
	size_t tokenLength;

	if (io_LineFoundSepPos == std::string::npos) // last token
	{
		tokenLength = i_Line.size();
	}
	else
	{
		tokenLength = io_LineFoundSepPos - io_LineCrrSearchPos;
	}

	token = i_Line.substr(io_LineCrrSearchPos, tokenLength);
	if (!token.empty())
	{
		size_t tokenCrrSearchPos = 0, tokenFoundSepPos = 0, typeIdx = 0;
		std::string subTokenSep("/");

		ParseFaceSubToken(tokenFoundSepPos, tokenCrrSearchPos, subTokenSep, token, io_FaceIdx, typeIdx, o_VertexIdx, o_UVIdx, o_NormalIdx);
	}

	if (io_LineFoundSepPos != std::string::npos) // still parsing
	{
		if (io_FaceIdx < o_VertexIdx.length())
		{
			io_FaceIdx++;
		}
		else
		{
			io_FaceIdx = o_VertexIdx.length() - 1;
		}

		io_LineCrrSearchPos = io_LineFoundSepPos + i_TokenSep.size();

		ParseFaceToken(io_LineFoundSepPos, io_LineCrrSearchPos, i_TokenSep, i_Line, io_FaceIdx, o_VertexIdx, o_UVIdx, o_NormalIdx);
	}
}

void Model::ParseFaceSubToken(size_t& io_TokenFoundSepPos, size_t& io_TokenCrrSearchPos, const std::string& i_TokenSep, const std::string& i_Token,
							   size_t i_FaceIdx, size_t& io_TypeIdx, glm::ivec3& o_VertexIdx, glm::ivec3& o_UVIdx, glm::ivec3& o_NormalIdx)
{
	io_TokenFoundSepPos = i_Token.find_first_of(i_TokenSep, io_TokenCrrSearchPos);

	std::string subToken;
	size_t tokenLength;

	if (io_TokenFoundSepPos == std::string::npos) // last token
	{
		tokenLength = i_Token.size();
	}
	else
	{
		tokenLength = io_TokenFoundSepPos - io_TokenCrrSearchPos;
	}

	subToken = i_Token.substr(io_TokenCrrSearchPos, tokenLength);
	if (!i_Token.empty())
	{
		unsigned int val = std::stoi(subToken);
		if (io_TypeIdx == 0) o_VertexIdx[i_FaceIdx] = val;
		else if (io_TypeIdx == 1) o_UVIdx[i_FaceIdx] = val;
		else if (io_TypeIdx == 2) o_NormalIdx[i_FaceIdx] = val;
	}

	if (io_TokenFoundSepPos != std::string::npos) // still parsing
	{
		if (io_TypeIdx < o_VertexIdx.length())
		{
			io_TypeIdx++;
		}
		else
		{
			io_TypeIdx = o_VertexIdx.length() - 1;
		}

		io_TokenCrrSearchPos = io_TokenFoundSepPos + i_TokenSep.size();

		ParseFaceSubToken(io_TokenFoundSepPos, io_TokenCrrSearchPos, i_TokenSep, i_Token, i_FaceIdx, io_TypeIdx, o_VertexIdx, o_UVIdx, o_NormalIdx);
	}
}

bool Model::ParseMTLFile(const std::string& i_MTLFileName, const std::string& i_Directory, std::map<std::string, Material>& o_MaterialMap)
{
	std::string mtlFilePath = i_Directory + "/" + i_MTLFileName;

	std::vector<Model::LoadedTextureData> loadedTextureVector;
	std::string crrMaterialName;

	char* pBasePath = SDL_GetBasePath();
	std::string baseName(pBasePath ? pBasePath : "");
	std::string fileName(baseName + mtlFilePath);
	std::string texBaseName(i_Directory + "/");

	SDL_RWops* pMtlFile = SDL_RWFromFile(fileName.c_str(), "rb");
	if (pMtlFile == nullptr)
	{
		ERR("Failed to open the mtl file: %s!", fileName.c_str());
		return false;
	}

	size_t fileSize = (size_t)SDL_RWsize(pMtlFile);

	char* pFileContent = new char[fileSize + 1];

	size_t nbRead = (size_t)SDL_RWread(pMtlFile, pFileContent, sizeof(char), fileSize);
	if (nbRead != fileSize)
	{
		ERR("Mismatch of bytes read vs file size in bytes!");
		return false;
	}

	pFileContent[fileSize] = '\0';

	std::string mtlContent(pFileContent);
	SAFE_ARRAY_DELETE(pFileContent);

	// process MTL file
	size_t fileCrrSearchPos = 0, fileFoundSepPos = 0;
	std::string lineSep("\r\n");

	std::string line;
	while (std::string::npos != (fileFoundSepPos = mtlContent.find_first_of(lineSep, fileCrrSearchPos)))
	{
		size_t lineLength = fileFoundSepPos - fileCrrSearchPos;

		line = mtlContent.substr(fileCrrSearchPos, lineLength);

		if (!line.empty())
		{
			ParseMTLLine(line, o_MaterialMap, crrMaterialName, loadedTextureVector, texBaseName);
		}

		fileCrrSearchPos = fileFoundSepPos + lineSep.size();
	}

	//process last line (after separator)
	line = mtlContent.substr(fileCrrSearchPos);
	if (!line.empty())
	{
		ParseMTLLine(line, o_MaterialMap, crrMaterialName, loadedTextureVector, texBaseName);
	}

	return true;
}

void Model::ParseMTLLine(const std::string& i_Line, std::map<std::string, Material>& o_MaterialMap, std::string& io_CrrMaterialName,
			std::vector<Model::LoadedTextureData>& io_LoadedTextureVector, const std::string& i_TexBaseName)
{
	// process line
	size_t lineCrrSearchPos = 0, lineFoundSepPos = 0;
	std::string tokenSep(" ");

	// get first token
	lineFoundSepPos = i_Line.find_first_of(tokenSep, lineCrrSearchPos);
	size_t tokenLength = lineFoundSepPos - lineCrrSearchPos;

	std::string token = i_Line.substr(lineCrrSearchPos, tokenLength);
	// filter out unneeded lines

	if (token.empty() || (token != "newmtl" && token != "map_Ka" &&
		token != "map_Kd" && token != "map_Ks" && token != "map_Bump"))
	{
		return; //jump to the next line
	}

	lineCrrSearchPos = lineFoundSepPos + tokenSep.size();

	if (token == "newmtl") // material name
	{
		token = i_Line.substr(lineCrrSearchPos);

		if (!token.empty())
		{
			io_CrrMaterialName = token;

			Material material(io_CrrMaterialName);
			o_MaterialMap[io_CrrMaterialName] = material;
		}
	}
	else if (token == "map_Ka") // material ambient texture map
	{
		token = i_Line.substr(lineCrrSearchPos);

		if (!token.empty())
		{
			std::string texPath(i_TexBaseName + "/" + token);

			Material::TextureData texData = ProcessTexture(texPath, Material::TEXTURE_MAP_TYPE::TMT_AMBIENT, io_LoadedTextureVector);

			o_MaterialMap[io_CrrMaterialName].TextureArray.push_back(texData);
		}
	}
	else if (token == "map_Kd") // material diffuse texture map
	{
		token = i_Line.substr(lineCrrSearchPos);

		if (!token.empty())
		{
			std::string texPath(i_TexBaseName + "/" + token);

			Material::TextureData texData = ProcessTexture(texPath, Material::TEXTURE_MAP_TYPE::TMT_DIFFUSE, io_LoadedTextureVector);

			o_MaterialMap[io_CrrMaterialName].TextureArray.push_back(texData);
		}
	}
	else if (token ==  "map_Ks") // material specular texture map
	{
		token = i_Line.substr(lineCrrSearchPos);

		if (!token.empty())
		{
			std::string texPath(i_TexBaseName + "/" + token);

			Material::TextureData texData = ProcessTexture(texPath, Material::TEXTURE_MAP_TYPE::TMT_SPECULAR, io_LoadedTextureVector);

			o_MaterialMap[io_CrrMaterialName].TextureArray.push_back(texData);
		}
	}
	else if (token == "map_Bump") // material normal texture map
	{
		token = i_Line.substr(lineCrrSearchPos);

		if (!token.empty())
		{
			std::string texPath(i_TexBaseName + "/" + token);

			Material::TextureData texData = ProcessTexture(texPath, Material::TEXTURE_MAP_TYPE::TMT_NORMAL, io_LoadedTextureVector);

			o_MaterialMap[io_CrrMaterialName].TextureArray.push_back(texData);
		}
	}
}

Material::TextureData Model::ProcessTexture(const std::string& i_TexPath, const Material::TEXTURE_MAP_TYPE& i_TexType, std::vector<Model::LoadedTextureData>& io_LoadedTextureVector)
{
	Material::TextureData texData;

	bool skip = false;
	for (unsigned int i = 0; i < io_LoadedTextureVector.size(); ++i)
	{
		if (i_TexPath == io_LoadedTextureVector[i].Path)
		{
			skip = true;

			texData.Id = io_LoadedTextureVector[i].Id;
			texData.Type = io_LoadedTextureVector[i].Type;

			break;
		}
	}

	if (!skip)
	{
		unsigned int texId = m_TM.Load2DTexture(i_TexPath.c_str(), GL_REPEAT, GL_LINEAR, true);

		Model::LoadedTextureData loadTexData;
		loadTexData.Id = texId;
		loadTexData.Type = i_TexType;
		loadTexData.Path = i_TexPath;
		io_LoadedTextureVector.push_back(loadTexData);

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