/* Author: BAIRAC MIHAI */

#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>
#include "glm/glm.hpp"
#include "Material.h"
#include "MeshBufferManager.h"
#include "TextureManager.h"
#include "ShaderManager.h"

/*
 A simple Wavefront OBJ mesh class
*/

class Mesh 
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

	Mesh(void);
	Mesh(const std::string& i_Name, const std::vector<MeshBufferManager::VertexData>& i_VertexData, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, const Material::TextureDataArray& i_TextureArray, bool i_UseMaterial = false, bool i_UseFlattenedModel = false);
	~Mesh(void);

	void Render(const ShaderManager& i_SM, const TextureManager& i_TM, unsigned short i_StartTexUnitId, bool i_IsWireframeMode);
	void Render(bool i_IsWireframeMode);
	void RenderFlattened(void);

	const Mesh::Limits& GetLimits(void) const;

private:
	//// Variables /////
	std::string m_Name;
	MeshBufferManager m_MBM, m_FlattenedMBM;
	unsigned int m_VertexCount;

	bool m_UseMaterial;
	bool m_UseFlattenedModel;

	Material::TextureDataArray m_TextureArray;
};

#endif /* MESH_H */