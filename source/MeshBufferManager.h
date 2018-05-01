/* Author: BAIRAC MIHAI */

#ifndef MESH_BUFFER_MANAGER_H
#define MESH_BUFFER_MANAGER_H

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#include <string>
#include <vector>
#include <map>

/*
 Manager for vertex data buffer. Usually used for meshes - geometry

 Data streaming info:
 https://www.opengl.org/wiki/Buffer_Object_Streaming
 http://gamedev.stackexchange.com/questions/87074/for-vertex-buffer-steaming-multiple-glbuffersubdata-vs-orphaning
*/

class MeshBufferManager
{
private:
	enum class DRAWING_TYPE 
	{
		DT_INDEXED = 0, 
		DT_NON_INDEXED,
		DT_COUNT
	};

	enum ELEMENT_TYPE 
	{ 
		ET_BI = 2, 
		ET_TRI = 3, 
		ET_QUADRO = 4, 
		ET_COUNT
	};

	//// Variables ////
	std::string m_Name;

	DRAWING_TYPE m_DrawingType;

	unsigned int m_VAOID, m_VBOID, m_IBOID;

	void Destroy ( void );

public:
	enum class ACCESS_TYPE 
	{
		AT_STATIC = 0,
		AT_DYNAMIC,
		AT_COUNT
	};

	struct VertexData
	{
		glm::vec3 position; //0 * 4 = 0
		glm::vec3 normal; //3 * 4 = 12
		glm::vec2 uv; //6 * 4 = 24
	};

	enum class VERTEX_ATTRIBUTE_TYPE 
	{ 
		VAT_POSITION = 0,
		VAT_NORMAL,
		VAT_TANGENT,
		VAT_BITANGENT,
		VAT_UV,
		VAT_COLOR,
		VAT_COUNT
	};

	//// Methods ////
	MeshBufferManager ( void );
	MeshBufferManager ( const std::string& i_Name );
	~MeshBufferManager ( void );

	void Initialize ( const std::string& i_Name );

	void CreateModel ( const std::vector<MeshBufferManager::VertexData>& i_ModelVertexData, MeshBufferManager::ACCESS_TYPE i_AccessType );
	void CreateModel ( const std::vector<MeshBufferManager::VertexData>& i_ModelVertexData, const std::vector<unsigned int>& i_ModelIndexes, MeshBufferManager::ACCESS_TYPE i_AccessType );

	void CreateModelContext ( const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, unsigned int i_VBOID, MeshBufferManager::ACCESS_TYPE i_AccessType );
	void CreateModelContext ( const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, unsigned int i_VBOID, unsigned int i_IBOID, MeshBufferManager::ACCESS_TYPE i_AccessType );
	void CreateModelContext ( const std::vector<MeshBufferManager::VertexData>& i_ModelVertexData, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, MeshBufferManager::ACCESS_TYPE i_AccessType );
	void CreateModelContext ( const std::vector<MeshBufferManager::VertexData>& i_ModelVertexData, const std::vector<unsigned int>& i_ModelIndexes, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, MeshBufferManager::ACCESS_TYPE i_AccessType );

	void BindModelContext ( void ) const;
	void UnBindModelContext ( void ) const;
	
	void UpdateModelVertexData ( const std::vector<MeshBufferManager::VertexData>& i_ModelVertexData ) const;

	unsigned int GetVBOID ( void ) const;
	unsigned int GetIBOID ( void ) const;
	MeshBufferManager::ACCESS_TYPE GetAccessType ( void ) const;

private:
	ACCESS_TYPE m_AccessType;
};

#endif /* MESH_BUFFER_MANAGER_H */
