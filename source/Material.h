/* Author: BAIRAC MIHAI */

#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <vector>

/*
 A simple Wavefront OBJ material class
*/

class Material
{
public:
	enum TEXTURE_MAP_TYPE 
	{ 
		TMT_AMBIENT = 0,
		TMT_DIFFUSE,
		TMT_SPECULAR,
		TMT_NORMAL,
		COUNT_TMT
	};

	struct TextureData
	{
		unsigned int Id;
		Material::TEXTURE_MAP_TYPE Type;
	};

	typedef std::vector<TextureData> TextureDataArray;

	struct MaterialDetails
	{
		unsigned short Id;
		std::string Name;
		
		TextureDataArray TextureArray;
	};

	Material ( void );
	Material ( const Material::MaterialDetails& i_MaterialDetails );
	~Material ( void );

private:
	// FOR NOW WE SUPPORT ONLY TEXTURE MAPS !!!

	MaterialDetails m_Details;
};

#endif /* MATERIAL_H */
