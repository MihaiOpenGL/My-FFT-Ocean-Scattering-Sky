/* Author: BAIRAC MIHAI */

#include "MeshBufferManager.h"
#include "GL/glew.h"
#include "ErrorHandler.h"
#include <new> //new, delete
#include <vector>
// glm::vec2, glm::vec3 come from the header

#define offsetof(s,m) ((size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))


MeshBufferManager::MeshBufferManager ( void )
	: m_Name(), m_VAOID(0), m_VBOID(0), m_IBOID(0),
	  m_DrawingType(DRAWING_TYPE::DT_COUNT), m_AccessType(ACCESS_TYPE::AT_COUNT)
{}

MeshBufferManager::MeshBufferManager ( const std::string& i_Name )
	: m_Name(i_Name), m_VAOID(0), m_VBOID(0), m_IBOID(0),
	  m_DrawingType(DRAWING_TYPE::DT_COUNT), m_AccessType(ACCESS_TYPE::AT_COUNT)
{}

MeshBufferManager::~MeshBufferManager ( void )
{
	Destroy();
}

void MeshBufferManager::Initialize ( const std::string& i_Name )
{
	m_Name = i_Name;
	m_DrawingType = DRAWING_TYPE::DT_COUNT;
}

void MeshBufferManager::Destroy ( void )
{
	if (m_VAOID)
	{
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &m_VAOID);
		m_VAOID = 0;
	}

	if (m_VBOID)
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &m_VBOID);
		m_VBOID = 0;
	}

	if (m_DrawingType == DRAWING_TYPE::DT_INDEXED)
	{
		if (m_IBOID)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glDeleteBuffers(1, &m_IBOID);
			m_IBOID = 0;
		}
	}

	LOG("[" + m_Name + "] Buffer Manager has been destroyed successfully!");
}

void MeshBufferManager::CreateModel ( const std::vector<MeshBufferManager::VertexData>& i_ModelVertexData, MeshBufferManager::ACCESS_TYPE i_AccessType )
{
	if (i_ModelVertexData.empty())
	{
		ERR("Empty model vertex data!");
		return;
	}

	if (i_AccessType >= ACCESS_TYPE::AT_COUNT)
	{
		ERR("Invalid access type!");
		return;
	}

	glGenBuffers(1, &m_VBOID);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOID);

	glBufferData(GL_ARRAY_BUFFER, i_ModelVertexData.size() * sizeof(MeshBufferManager::VertexData), &i_ModelVertexData[0], i_AccessType == ACCESS_TYPE::AT_STATIC ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_AccessType = i_AccessType;
	m_DrawingType = DRAWING_TYPE::DT_NON_INDEXED;
}

void MeshBufferManager::CreateModel ( const std::vector<MeshBufferManager::VertexData>& i_ModelVertexData, const std::vector<unsigned int>& i_ModelIndexes, MeshBufferManager::ACCESS_TYPE i_AccessType )
{
	if (i_ModelVertexData.empty())
	{
		ERR("Empty model vertex data!");
		return;
	}

	if (i_ModelIndexes.empty())
	{
		ERR("Empty model indexes!");
		return;
	}

	if (i_AccessType >= ACCESS_TYPE::AT_COUNT)
	{
		ERR("Invalid access type!");
		return;
	}

	glGenBuffers(1, &m_VBOID);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOID);

	glBufferData(GL_ARRAY_BUFFER, i_ModelVertexData.size() * sizeof(MeshBufferManager::VertexData), &i_ModelVertexData[0], i_AccessType == ACCESS_TYPE::AT_STATIC ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_IBOID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBOID);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_ModelIndexes.size() * sizeof(unsigned int), &i_ModelIndexes[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_AccessType = i_AccessType;
	m_DrawingType = DRAWING_TYPE::DT_INDEXED;
}

void MeshBufferManager::CreateModelContext ( const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, unsigned int i_VBOID, MeshBufferManager::ACCESS_TYPE i_AccessType )
{
	if (i_ModelVertexAttributes.empty())
	{
		ERR("Empty model vertex attribute locations!");
		return;
	}

	if (!glIsBuffer(i_VBOID))
	{
		ERR("Invalid i_VBOID!");
		return;
	}

	glGenVertexArrays(1, &m_VAOID);
	glBindVertexArray(m_VAOID);

	glBindBuffer(GL_ARRAY_BUFFER, i_VBOID);

	int location = -1;
	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>::const_iterator it;
	if (((it = i_ModelVertexAttributes.find(MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION)) != i_ModelVertexAttributes.end()) && ((location = it->second) > -1))
	{
		glVertexAttribPointer(location, ELEMENT_TYPE::ET_TRI, GL_FLOAT, GL_FALSE, sizeof(MeshBufferManager::VertexData), reinterpret_cast<void*>(offsetof(VertexData, VertexData::position)));
		glEnableVertexAttribArray(location);
	}
	if (((it = i_ModelVertexAttributes.find(MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_NORMAL)) != i_ModelVertexAttributes.end()) && ((location = it->second) > -1))
	{
		glVertexAttribPointer(location, ELEMENT_TYPE::ET_TRI, GL_FLOAT, GL_FALSE, sizeof(MeshBufferManager::VertexData), reinterpret_cast<void*>(offsetof(VertexData, VertexData::normal)));
		glEnableVertexAttribArray(location);
	}
	if (((it = i_ModelVertexAttributes.find(MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV)) != i_ModelVertexAttributes.end()) && ((location = it->second) > -1))
	{
		glVertexAttribPointer(location, ELEMENT_TYPE::ET_BI, GL_FLOAT, GL_FALSE, sizeof(MeshBufferManager::VertexData), reinterpret_cast<void*>(offsetof(VertexData, VertexData::uv)));
		glEnableVertexAttribArray(location);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_AccessType = i_AccessType;
	m_DrawingType = DRAWING_TYPE::DT_NON_INDEXED;
}

void MeshBufferManager::CreateModelContext ( const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, unsigned int i_VBOID, unsigned int i_IBOID, MeshBufferManager::ACCESS_TYPE i_AccessType )
{
	if (i_ModelVertexAttributes.empty())
	{
		ERR("Empty model vertex attribute locations!");
		return;
	}

	if (!glIsBuffer(i_VBOID))
	{
		ERR("Invalid i_VBOID!");
		return;
	}

	glGenVertexArrays(1, &m_VAOID);
	glBindVertexArray(m_VAOID);

	glBindBuffer(GL_ARRAY_BUFFER, i_VBOID);

	int location = -1;
	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>::const_iterator it;
	if (((it = i_ModelVertexAttributes.find(MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION)) != i_ModelVertexAttributes.end()) && ((location = it->second) > -1))
	{
		glVertexAttribPointer(location, ELEMENT_TYPE::ET_TRI, GL_FLOAT, GL_FALSE, sizeof(MeshBufferManager::VertexData), reinterpret_cast<void*>(offsetof(VertexData, VertexData::position)));
		glEnableVertexAttribArray(location);
	}
	if (((it = i_ModelVertexAttributes.find(MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_NORMAL)) != i_ModelVertexAttributes.end()) && ((location = it->second) > -1))
	{
		glVertexAttribPointer(location, ELEMENT_TYPE::ET_TRI, GL_FLOAT, GL_FALSE, sizeof(MeshBufferManager::VertexData), reinterpret_cast<void*>(offsetof(VertexData, VertexData::normal)));
		glEnableVertexAttribArray(location);
	}
	if (((it = i_ModelVertexAttributes.find(MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV)) != i_ModelVertexAttributes.end()) && ((location = it->second) > -1))
	{
		glVertexAttribPointer(location, ELEMENT_TYPE::ET_BI, GL_FLOAT, GL_FALSE, sizeof(MeshBufferManager::VertexData), reinterpret_cast<void*>(offsetof(VertexData, VertexData::uv)));
		glEnableVertexAttribArray(location);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i_IBOID);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_AccessType = i_AccessType;
	m_DrawingType = DRAWING_TYPE::DT_INDEXED;
}


void MeshBufferManager::CreateModelContext ( const std::vector<MeshBufferManager::VertexData>& i_ModelVertexData, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, MeshBufferManager::ACCESS_TYPE i_AccessType )
{
	if (i_ModelVertexData.empty())
	{
		ERR("Empty model vertex data!");
		return;
	}

	if (i_ModelVertexAttributes.empty())
	{
		ERR("Empty model vertex attribute locations!");
		return;
	}

	if (i_AccessType >= ACCESS_TYPE::AT_COUNT)
	{
		ERR("Invalid access type!");
		return;
	}

	glGenVertexArrays(1, &m_VAOID);
	glBindVertexArray(m_VAOID);

	glGenBuffers(1, &m_VBOID);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOID);

	glBufferData(GL_ARRAY_BUFFER, i_ModelVertexData.size() * sizeof(MeshBufferManager::VertexData), &i_ModelVertexData[0], i_AccessType == ACCESS_TYPE::AT_STATIC ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

	int location = -1;
	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>::const_iterator it;
	if (((it = i_ModelVertexAttributes.find(MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION)) != i_ModelVertexAttributes.end()) && ((location = it->second) > -1))
	{
		glVertexAttribPointer(location, ELEMENT_TYPE::ET_TRI, GL_FLOAT, GL_FALSE, sizeof(MeshBufferManager::VertexData), reinterpret_cast<void*>(offsetof(VertexData, VertexData::position)));
		glEnableVertexAttribArray(location);
	}
	if (((it = i_ModelVertexAttributes.find(MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_NORMAL)) != i_ModelVertexAttributes.end()) && ((location = it->second) > -1))
	{
		glVertexAttribPointer(location, ELEMENT_TYPE::ET_TRI, GL_FLOAT, GL_FALSE, sizeof(MeshBufferManager::VertexData), reinterpret_cast<void*>(offsetof(VertexData, VertexData::normal)));
		glEnableVertexAttribArray(location);
	}
	if (((it = i_ModelVertexAttributes.find(MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV)) != i_ModelVertexAttributes.end()) && ((location = it->second) > -1))
	{
		glVertexAttribPointer(location, ELEMENT_TYPE::ET_BI, GL_FLOAT, GL_FALSE, sizeof(MeshBufferManager::VertexData), reinterpret_cast<void*>(offsetof(VertexData, VertexData::uv)));
		glEnableVertexAttribArray(location);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_AccessType = i_AccessType;
	m_DrawingType = DRAWING_TYPE::DT_NON_INDEXED;
}

void MeshBufferManager::CreateModelContext ( const std::vector<MeshBufferManager::VertexData>& i_ModelVertexData, const std::vector<unsigned int>& i_ModelIndexes, const std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>& i_ModelVertexAttributes, MeshBufferManager::ACCESS_TYPE i_AccessType )
{
	if (i_ModelVertexData.empty())
	{
		ERR("Empty model vertex data!");
		return;
	}

	if (i_ModelIndexes.empty())
	{
		ERR("Empty model indexes!");
		return;
	}

	if (i_ModelVertexAttributes.empty())
	{
		ERR("Empty model vertex attribute locations!");
		return;
	}

	if (i_AccessType >= ACCESS_TYPE::AT_COUNT)
	{
		ERR("Invalid access type!");
		return;
	}

	glGenVertexArrays(1, &m_VAOID);
	glBindVertexArray(m_VAOID);

	glGenBuffers(1, &m_VBOID);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOID);

	glBufferData(GL_ARRAY_BUFFER, i_ModelVertexData.size() * sizeof(MeshBufferManager::VertexData), &i_ModelVertexData[0], i_AccessType == ACCESS_TYPE::AT_STATIC ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

	int location = -1;
	std::map<MeshBufferManager::VERTEX_ATTRIBUTE_TYPE, int>::const_iterator it;
	if (((it = i_ModelVertexAttributes.find(MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_POSITION)) != i_ModelVertexAttributes.end()) && ((location = it->second) > -1))
	{
		glVertexAttribPointer(location, ELEMENT_TYPE::ET_TRI, GL_FLOAT, GL_FALSE, sizeof(MeshBufferManager::VertexData), reinterpret_cast<void*>(offsetof(VertexData, VertexData::position)));
		glEnableVertexAttribArray(location);
	}
	if (((it = i_ModelVertexAttributes.find(MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_NORMAL)) != i_ModelVertexAttributes.end()) && ((location = it->second) > -1))
	{
		glVertexAttribPointer(location, ELEMENT_TYPE::ET_TRI, GL_FLOAT, GL_FALSE, sizeof(MeshBufferManager::VertexData), reinterpret_cast<void*>(offsetof(VertexData, VertexData::normal)));
		glEnableVertexAttribArray(location);
	}
	if (((it = i_ModelVertexAttributes.find(MeshBufferManager::VERTEX_ATTRIBUTE_TYPE::VAT_UV)) != i_ModelVertexAttributes.end()) && ((location = it->second) > -1))
	{
		glVertexAttribPointer(location, ELEMENT_TYPE::ET_BI, GL_FLOAT, GL_FALSE, sizeof(MeshBufferManager::VertexData), reinterpret_cast<void*>(offsetof(VertexData, VertexData::uv)));
		glEnableVertexAttribArray(location);
	}

	glGenBuffers(1, &m_IBOID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBOID);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_ModelIndexes.size() * sizeof(unsigned int), &i_ModelIndexes[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_AccessType = i_AccessType;
	m_DrawingType = DRAWING_TYPE::DT_INDEXED;
}

void MeshBufferManager::BindModelContext ( void ) const
{
	if (m_DrawingType == DRAWING_TYPE::DT_INDEXED || m_DrawingType == DRAWING_TYPE::DT_NON_INDEXED)
	{
		glBindVertexArray(m_VAOID);
	}
}

void MeshBufferManager::UnBindModelContext ( void ) const
{
	if (m_DrawingType == DRAWING_TYPE::DT_INDEXED || m_DrawingType == DRAWING_TYPE::DT_NON_INDEXED)
	{
		// http://ogldev.atspace.co.uk/www/tutorial32/tutorial32.html
		// Before leaving we reset the current VAO back to zero and the reason is the same as when we initially created the VAO - we don't want outside code to bind a VB (for example) and change our VAO unintentinally.
		glBindVertexArray(0);
	}
}

void MeshBufferManager::UpdateModelVertexData ( const std::vector<MeshBufferManager::VertexData>& i_ModelVertexData ) const
{
	if (m_AccessType == ACCESS_TYPE::AT_DYNAMIC)
	{
		// The compatibility OpenGL profile makes VAO object 0 a default object.
		// The core OpenGL profile makes VAO object 0 not an object at all.
		// So if VAO 0 is bound in the core profile, you should not call any function
		// that modifies VAO state.This includes binding the GL_ELEMENT_ARRAY_BUFFER with glBindBuffer​.
		// https://www.opengl.org/wiki/Vertex_Specification

		glBindBuffer(GL_ARRAY_BUFFER, m_VBOID); //we bind VBO and not VAO, because VBO binding is not in VAO state // https://www.opengl.org/wiki/Vertex_Specification#Vertex_Array_Object
		glBufferSubData(GL_ARRAY_BUFFER, 0, i_ModelVertexData.size() * sizeof(MeshBufferManager::VertexData), &i_ModelVertexData[0]);
	//	glBufferData(GL_ARRAY_BUFFER, i_ModelVertexData.size() * sizeof(MeshBufferManager::VertexData), &i_ModelVertexData[0], GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

unsigned int MeshBufferManager::GetVBOID ( void ) const
{
	return m_VBOID;
}

unsigned int MeshBufferManager::GetIBOID ( void ) const
{
	return m_IBOID;
}

MeshBufferManager::ACCESS_TYPE MeshBufferManager::GetAccessType ( void ) const
{
	return m_AccessType;
}