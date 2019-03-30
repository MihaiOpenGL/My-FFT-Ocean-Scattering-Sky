/* Author: BAIRAC MIHAI */

#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <vector>

/*
 A simple Wavefront OBJ material class
*/

struct Material
{
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

	Material ( void );
	Material(const std::string& i_Name);
	~Material ( void );

	std::string Name;
	// FOR NOW WE SUPPORT ONLY TEXTURE MAPS !!!
	TextureDataArray TextureArray;
};

#endif /* MATERIAL_H */
