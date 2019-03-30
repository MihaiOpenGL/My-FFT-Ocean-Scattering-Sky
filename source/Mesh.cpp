/* Author: BAIRAC MIHAI */

#include "Mesh.h"
#include "CommonHeaders.h"
#include "Material.h"
#include "GLConfig.h"
#include <limits>


Mesh::Mesh ( void )
	: m_Name("Default"), m_VertexCount(0), m_UseMaterial(false), m_UseFlattenedModel(false)
{
	LOG("Mesh [%s] successfully created!", m_Name.c_str());
}

Mesh::Mesh ( const std::string& i_Name, const std::vector<MeshBufferManager::VertexData>& i_VertexData, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, const Material::TextureDataArray& i_TextureArray, bool i_UseMaterial, bool i_UseFlattenedModel )
{
	m_Name = i_Name;
	m_UseMaterial = i_UseMaterial;
	m_UseFlattenedModel = i_UseFlattenedModel;

	if (i_UseMaterial)
	{
		m_TextureArray = i_TextureArray;
	}

	m_MBM.Initialize(i_Name);
	m_MBM.CreateModelContext(i_VertexData, i_ModelVertexAttributes, MeshBufferManager::ACCESS_TYPE::AT_STATIC);

	m_VertexCount = i_VertexData.size();

	// compute additional useful mesh data like: minX, maxX, minY, maxY, minZ, maxZ
	m_Limits.MinX = std::numeric_limits<float>::max(); m_Limits.MaxX = 0.0f;
	m_Limits.MinY = std::numeric_limits<float>::max(), m_Limits.MaxY = 0.0f;
	m_Limits.MinZ = std::numeric_limits<float>::max(), m_Limits.MaxZ = 0.0f;
	for (unsigned short i = 0; i < i_VertexData.size(); ++i)
	{
		if (i_VertexData[i].position.x < m_Limits.MinX) m_Limits.MinX = i_VertexData[i].position.x;
		if (i_VertexData[i].position.x >= m_Limits.MaxX) m_Limits.MaxX = i_VertexData[i].position.x;

		if (i_VertexData[i].position.y < m_Limits.MinY) m_Limits.MinY = i_VertexData[i].position.y;
		if (i_VertexData[i].position.y >= m_Limits.MaxY) m_Limits.MaxY = i_VertexData[i].position.y;

		if (i_VertexData[i].position.z < m_Limits.MinZ) m_Limits.MinZ = i_VertexData[i].position.z;
		if (i_VertexData[i].position.z >= m_Limits.MaxZ) m_Limits.MaxZ = i_VertexData[i].position.z;
	}

	if (m_UseFlattenedModel)
	{
		// flatten the model by modifying the y coordinate
		std::vector<MeshBufferManager::VertexData> flattenedVertexData(i_VertexData);
		for (unsigned short i = 0; i < flattenedVertexData.size(); ++i)
		{
			//flattenedVertexData[i].position.y = 0.0f;
			flattenedVertexData[i].position.y = (m_Limits.MaxY - m_Limits.MinY) * 0.333f; //use 1/3 the height of the mesh
		}

		m_FlattenedMBM.Initialize(i_Name);
		m_FlattenedMBM.CreateModelContext(flattenedVertexData, i_ModelVertexAttributes, MeshBufferManager::ACCESS_TYPE::AT_STATIC);
	}

	LOG("Mesh [%s] successfully created!", m_Name.c_str());
}

Mesh::~Mesh ( void )
{
	LOG("Mesh [%s] successfully destroyed!", m_Name.c_str());
}

void Mesh::Render ( const ShaderManager& i_SM, const TextureManager& i_TM, unsigned short i_StartTexUnitId, bool i_IsWireframeMode )
{
	if (m_UseMaterial)
	{
		unsigned short unitTexId = i_StartTexUnitId;

		for (unsigned short i = 0; i < m_TextureArray.size(); ++i)
		{
			std::string samplerName;

			switch (m_TextureArray[i].Type)
			{
			case Material::TEXTURE_MAP_TYPE::TMT_AMBIENT:
				samplerName = "AmbientalMap";
				break;
			case Material::TEXTURE_MAP_TYPE::TMT_DIFFUSE:
				samplerName = "DiffuseMap";
				break;
			case Material::TEXTURE_MAP_TYPE::TMT_SPECULAR:
				samplerName = "SpecularMap";
				break;
			case Material::TEXTURE_MAP_TYPE::TMT_NORMAL:
				samplerName = "NormalMap";
				break;
			default:
				ERR("Invalid Map Type !!!");
				break;
			}

			int loc = glGetUniformLocation(i_SM.GetProgramID(), ("u_Material." + samplerName).c_str());

			if (loc != -1)
			{
				i_TM.BindTexture(m_TextureArray[i].Id, true, unitTexId);

				i_SM.SetUniform(loc, unitTexId);

				++unitTexId;
			}
		}
	}

	m_MBM.BindModelContext();

	glDrawArrays(i_IsWireframeMode ? GL_LINES : GL_TRIANGLES, 0, m_VertexCount);
	
	m_MBM.UnBindModelContext();
}

void Mesh::Render ( bool i_IsWireframeMode )
{
	m_MBM.BindModelContext();

	glDrawArrays(i_IsWireframeMode ? GL_LINES : GL_TRIANGLES, 0, m_VertexCount);

	m_MBM.UnBindModelContext();
}

void Mesh::RenderFlattened ( void )
{
	if (m_UseFlattenedModel)
	{
		m_FlattenedMBM.BindModelContext();

		glDrawArrays(GL_TRIANGLES, 0, m_VertexCount);

		m_FlattenedMBM.UnBindModelContext();
	}
}

const Mesh::Limits& Mesh::GetLimits ( void ) const
{
	return m_Limits;
}