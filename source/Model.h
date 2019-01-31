/* Author: BAIRAC MIHAI */

#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include "Mesh.h"
#include "MeshBufferManager.h"
#include "TextureManager.h"

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
	Model(const std::string& i_Name, const std::string& i_Path, bool i_UseMaterial, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, bool i_UseFlattenedModel = false);
	~Model(void);

	bool LoadModel(const std::string& i_Name, const std::string& i_Path, bool i_UseMaterial, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, bool i_UseFlattenedModel = false);

	void Render(const ShaderManager& i_SM, unsigned short i_StartTexUnitId, bool i_IsWireframeMode);
	void Render(bool i_IsWireframeMode);
	void RenderFlattened(void);

	const Model::Limits& GetLimits(void) const;
	const Model::Dimensions& GetDimensions(void) const;
	float GetWidth(void) const;
	float GetHeight(void) const;
	float GetDepth(void) const;

private:
	///// Methods ////
	void Destroy(void);

	//// Variables ////
	std::string m_Name;

	std::vector<Mesh*> m_Meshes;
	TextureManager m_TM;
};

#endif /* MODEL_H */