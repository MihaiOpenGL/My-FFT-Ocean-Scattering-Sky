/* Author: BAIRAC MIHAI */

/*
 ! IMPORTANT! TextureManager supports only DDS images. If there are images in other formaats that are to be used
 then these must be:
 a) vertically flipped (can use IrfanView for this)
 or the texture coordinates in shader must be flipped vertically like this: ve2(uv.x, 1.0f - uv.y)
 b) converted to DDS format (DXT1 or DXT5, can use a convertor for this)

 NOTE ! SINCE OPENGL 3.X GL_CLAMP HAS BEEN REMOVED, USE ONLY GL_CLAMP_TO_EDGE !!!
*/

#include "TextureManager.h"
#include "GL/glew.h"
#include "Common.h"
#include "ErrorHandler.h"
#include "gli/texture.hpp"
#include "gli/load.hpp"
#include "gli/gl.hpp"
#include "glm/vec3.hpp"
#include <fstream>
#include <assert.h>


int TextureManager::m_MaxTexUnits = 0;
int TextureManager::m_MaxTextureArrayLayers = 0;
float TextureManager::m_MaxAnisotropy = 0.0f;

TextureManager::TextureManager ( void )
	: m_Name()
{}

TextureManager::TextureManager ( const std::string& i_Name )
	: m_Name(i_Name)
{
	Init();
}

TextureManager::~TextureManager ( void )
{
	Destroy();
}

void TextureManager::Init ( void )
{
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_MaxTexUnits);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &m_MaxTextureArrayLayers);
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_MaxAnisotropy);
}

void TextureManager::Initialize ( const std::string& i_Name )
{
	m_Name = i_Name;

	Init();
}

void TextureManager::Destroy ( void )
{
	// These are the targets we use
	glBindTexture(GL_TEXTURE_1D, 0);
	glBindTexture(GL_TEXTURE_1D_ARRAY, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glBindTexture(GL_TEXTURE_3D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	for (size_t i = 0; i < m_TextureDataArray.size(); ++ i)
	{
		glDeleteTextures(1, &m_TextureDataArray[i].texId);
	}

	LOG("[" + m_Name + "] Texture Manager has been destroyed successfully!");
}

unsigned int TextureManager::Load1DTexture ( const std::string& i_ImageFileName, unsigned int i_WrapType, unsigned int i_FilterType, bool i_IsGammaCorrected, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering )
{
	gli::texture texture = gli::load(i_ImageFileName);
	if (texture.empty())
	{
		ERR("Image file " + i_ImageFileName + " failed to load!");
		return 0;
	}

	gli::gl GL;
	unsigned int target = GL.translate(texture.target());
	if (target != GL_TEXTURE_1D)
	{
		ERR("Texture target must be GL_TEXTURE_1D!");
		return 0;
	}

	unsigned int texId = GenAndBindTexture(target, i_TexUnitId);

	gli::gl::format format = GL.translate(texture.format());
	if (i_IsGammaCorrected)
	{
		if (format.Internal == gli::gl::internalFormat::INTERNAL_RGB_DXT1)
		{
			format.Internal = gli::gl::internalFormat::INTERNAL_SRGB_DXT1;
		}
		else if (format.Internal == gli::gl::internalFormat::INTERNAL_RGBA_DXT1)
		{
			format.Internal = gli::gl::internalFormat::INTERNAL_SRGB_ALPHA_DXT1;
		}
	}

	glm::tvec3<int> const dimensions(texture.dimensions());

	if (gli::is_compressed(texture.format()))
	{
		m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, format.Internal, format.Internal, format.Type, dimensions.x, dimensions.y, i_WrapType, i_FilterType, i_MipMapCount, 1));
		// allocate memory and load the texture data
		glCompressedTexImage1D(target, 0, format.Internal, dimensions.x, 0, texture.size(), texture.data());
	}
	else
	{
		m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, format.Internal, format.External, format.Type, dimensions.x, dimensions.y, i_WrapType, i_FilterType, i_MipMapCount, 1));
		// allocate memory and load the texture data
		glTexImage1D(target, 0, format.Internal, dimensions.x, 0, format.External, format.Type, texture.data());
	}

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount, i_AnisoFiltering);

	return texId;
}

unsigned int TextureManager::Load1DArrayTexture ( const std::vector<std::string>& i_ImageFileNameArray, unsigned int i_WrapType, unsigned int i_FilterType, bool i_IsGammaCorrected, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering )
{
	if (i_ImageFileNameArray.empty())
	{
		ERR("Image file array is empty!");
		return 0;
	}

	unsigned int texId = 0;
	unsigned int target = 0;
	for (unsigned short i = 0; i < i_ImageFileNameArray.size(); ++i)
	{
		gli::texture texture = gli::load(i_ImageFileNameArray[i]);
		if (texture.empty())
		{
			ERR("Image file " + i_ImageFileNameArray[i] + " failed to load!");
			return 0;
		}

		gli::gl GL;
		target = GL.translate(texture.target());
		if (target != GL_TEXTURE_1D_ARRAY)
		{
			ERR("Texture target must be GL_TEXTURE_1D_ARRAY!");
			return 0;
		}

		gli::gl::format format = GL.translate(texture.format());
		if (i_IsGammaCorrected)
		{
			if (format.Internal == gli::gl::internalFormat::INTERNAL_RGB_DXT1)
			{
				format.Internal = gli::gl::internalFormat::INTERNAL_SRGB_DXT1;
			}
			else if (format.Internal == gli::gl::internalFormat::INTERNAL_RGBA_DXT1)
			{
				format.Internal = gli::gl::internalFormat::INTERNAL_SRGB_ALPHA_DXT1;
			}
		}

		glm::tvec3<int> const dimensions(texture.dimensions());

		if (i == 0)
		{
			texId = GenAndBindTexture(target, i_TexUnitId);

			if (gli::is_compressed(texture.format()))
			{
				m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, format.Internal, format.Internal, format.Type, dimensions.x, dimensions.y, i_WrapType, i_FilterType, i_MipMapCount, i_ImageFileNameArray.size()));

				// allocate space for 1d texture array
				glCompressedTexImage2D(target, 0, format.Internal, dimensions.x, i_ImageFileNameArray.size(), 0, texture.size(), nullptr);
			}
			else
			{
				m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, format.Internal, format.External, format.Type, dimensions.x, dimensions.y, i_WrapType, i_FilterType, i_MipMapCount, i_ImageFileNameArray.size()));

				// allocate space for 1d texture array
				glTexImage2D(target, 0, format.Internal, dimensions.x, i_ImageFileNameArray.size(), 0, format.External, format.Type, nullptr);
			}
		}

		if (gli::is_compressed(texture.format()))
		{
			// copy data from each dds image to the 2d texture array layer
			glCompressedTexSubImage2D(target, 0,  0, i, dimensions.x, i_ImageFileNameArray.size(), format.Internal, texture.size(), texture.data());
		}
		else
		{
			// copy data from each dds image to the 2d texture array layer
			glTexSubImage2D(target, 0, 0, i, dimensions.x, i_ImageFileNameArray.size(), format.External, format.Type, texture.data());
		}
	}

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount, i_AnisoFiltering);

	return texId;
}

unsigned int TextureManager::Load2DTexture ( const std::string& i_ImageFileName, unsigned int i_WrapType, unsigned int i_FilterType, bool i_IsGammaCorrected, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering )
{
	gli::texture texture = gli::load(i_ImageFileName);
	if (texture.empty())
	{
		ERR("Image file " + i_ImageFileName + " failed to load!");
		return 0;
	}

	gli::gl GL;
	unsigned int target = GL.translate(texture.target());
	if (target != GL_TEXTURE_2D)
	{
		ERR("Texture target must be GL_TEXTURE_2D!");
		return 0;
	}

	unsigned int texId = GenAndBindTexture(target, i_TexUnitId);

	gli::gl::format format = GL.translate(texture.format());
	if (i_IsGammaCorrected)
	{
		if (format.Internal == gli::gl::internalFormat::INTERNAL_RGB_DXT1)
		{
			format.Internal = gli::gl::internalFormat::INTERNAL_SRGB_DXT1;
		}
		else if (format.Internal == gli::gl::internalFormat::INTERNAL_RGBA_DXT1)
		{
			format.Internal = gli::gl::internalFormat::INTERNAL_SRGB_ALPHA_DXT1;
		}
	}

	glm::tvec3<int> const dimensions(texture.dimensions());

	if (gli::is_compressed(texture.format()))
	{
		m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, format.Internal, format.Internal, format.Type, dimensions.x, dimensions.y, i_WrapType, i_FilterType, i_MipMapCount, 1));
		// allocate memory and load the texture data
		glCompressedTexImage2D(target, 0, format.Internal, dimensions.x, dimensions.y, 0, texture.size(), texture.data());
	}
	else
	{
		m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, format.Internal, format.External, format.Type, dimensions.x, dimensions.y, i_WrapType, i_FilterType, i_MipMapCount, 1));
		// allocate memory and load the texture data
		glTexImage2D(target, 0, format.Internal, dimensions.x, dimensions.y, 0, format.External, format.Type, texture.data());
	}

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount, i_AnisoFiltering);

	return texId;
}

unsigned int TextureManager::Load2DArrayTexture ( const std::vector<std::string>& i_ImageFileNameArray, unsigned int i_WrapType, unsigned int i_FilterType, bool i_IsGammaCorrected, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering )
{
	if (i_ImageFileNameArray.empty())
	{
		ERR("Image file array is empty!");
		return 0;
	}

	unsigned int texId = 0;
	unsigned int target = 0;
	for (unsigned short i = 0; i < i_ImageFileNameArray.size(); ++i)
	{
		gli::texture texture = gli::load(i_ImageFileNameArray[i]);
		if (texture.empty())
		{
			ERR("Image file " + i_ImageFileNameArray[i] + " failed to load!");
			return 0;
		}

		gli::gl GL;
		target = GL.translate(texture.target());
		if (target != GL_TEXTURE_2D_ARRAY)
		{
			ERR("Texture target must be GL_TEXTURE_2D_ARRAY!");
			return 0;
		}

		gli::gl::format format = GL.translate(texture.format());
		if (i_IsGammaCorrected)
		{
			if (format.Internal == gli::gl::internalFormat::INTERNAL_RGB_DXT1)
			{
				format.Internal = gli::gl::internalFormat::INTERNAL_SRGB_DXT1;
			}
			else if (format.Internal == gli::gl::internalFormat::INTERNAL_RGBA_DXT1)
			{
				format.Internal = gli::gl::internalFormat::INTERNAL_SRGB_ALPHA_DXT1;
			}
		}

		glm::tvec3<int> const dimensions(texture.dimensions());

		if (i == 0)
		{
			texId = GenAndBindTexture(target, i_TexUnitId);

			if (gli::is_compressed(texture.format()))
			{
				m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, format.Internal, format.Internal, format.Type, dimensions.x, dimensions.y, i_WrapType, i_FilterType, i_MipMapCount, i_ImageFileNameArray.size()));

				// allocate space for 2d texture array
				glCompressedTexImage3D(target, 0, format.Internal, dimensions.x, dimensions.y, i_ImageFileNameArray.size(), 0, texture.size(), nullptr);
			}
			else
			{
				m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, format.Internal, format.External, format.Type, dimensions.x, dimensions.y, i_WrapType, i_FilterType, i_MipMapCount, i_ImageFileNameArray.size()));

				// allocate space for 2d texture array
				glTexImage3D(target, 0, format.Internal, dimensions.x, dimensions.y, i_ImageFileNameArray.size(), 0, format.External, format.Type, nullptr);
			}
		}

		if (gli::is_compressed(texture.format()))
		{
			// copy data from each dds image to the 2d texture array layer
			glCompressedTexSubImage3D(target, 0, 0, 0, i, dimensions.x, dimensions.y, i_ImageFileNameArray.size(), format.Internal, texture.size(), texture.data());
		}
		else
		{
			// copy data from each dds image to the 2d texture array layer
			glTexSubImage3D(target, 0, 0, 0, i, dimensions.x, dimensions.y, i_ImageFileNameArray.size(), format.External, format.Type, texture.data());
		}
	}

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount, i_AnisoFiltering);

	return texId;
}

unsigned int TextureManager::LoadCubeMapTexture ( const std::string& i_ImageFileName, unsigned int i_WrapType, unsigned int i_FilterType, bool i_IsGammaCorrected, short i_TexUnitId, short i_MipMapCount )
{
	gli::texture texture = gli::load(i_ImageFileName);
	if (texture.empty())
	{
		ERR("Image file " + i_ImageFileName + " failed to load!");
		return 0;
	}

	gli::gl GL;
	unsigned int target = GL.translate(texture.target());
	if (target != GL_TEXTURE_CUBE_MAP)
	{
		ERR("Texture target must be GL_TEXTURE_CUBE_MAP!");
		return 0;
	}

	unsigned int texId = GenAndBindTexture(target, i_TexUnitId);

	gli::gl::format format = GL.translate(texture.format());
	if (i_IsGammaCorrected)
	{
		if (format.Internal == gli::gl::internalFormat::INTERNAL_RGB_DXT1)
		{
			format.Internal = gli::gl::internalFormat::INTERNAL_SRGB_DXT1;
		}
		else if (format.Internal == gli::gl::internalFormat::INTERNAL_RGBA_DXT1)
		{
			format.Internal = gli::gl::internalFormat::INTERNAL_SRGB_ALPHA_DXT1;
		}
	}

	glm::tvec3<int> const dimensions(texture.dimensions());

	m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, GL_TEXTURE_CUBE_MAP, format.Internal, format.Internal, format.Type, dimensions.x, dimensions.y, i_WrapType, i_FilterType, i_MipMapCount, 1));

	for (unsigned short i = 0; i < 6; ++ i)
	{
		if (gli::is_compressed(texture.format()))
		{
			// allocate memory and load the texture data
			glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format.Internal, dimensions.x, dimensions.y, 0, texture.size(), texture.data(0, i, 0));
		}
		else
		{
			// allocate memory and load the texture data
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format.Internal, dimensions.x, dimensions.y, 0, format.External, format.Type, texture.data(0, i, 0));
		}
	}

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount);

	return texId;
}

unsigned int TextureManager::LoadCubeMapTexture ( const std::vector<std::string>& i_ImageFileArray, unsigned int i_WrapType, unsigned int i_FilterType, bool i_IsGammaCorrected, short i_TexUnitId, short i_MipMapCount )
{
	if (i_ImageFileArray.empty())
	{
		ERR("Image file array is empty!");
		return 0;
	}

	if (i_ImageFileArray.size() != 6)
	{
		ERR("Image file array must ccontain 6 images/faces!");
		return 0;
	}

	unsigned int texId = GenAndBindTexture(GL_TEXTURE_CUBE_MAP, i_TexUnitId);

	for (unsigned short i = 0; i < i_ImageFileArray.size(); ++ i)
	{
		gli::texture texture = gli::load(i_ImageFileArray[i]);
		if (texture.empty())
		{
			ERR("Image file " + std::string(i_ImageFileArray[i]) + " failed to load!");
			return 0;
		}

		gli::gl GL;
		unsigned int target = GL.translate(texture.target());
		if (target != GL_TEXTURE_2D)
		{
			ERR("Texture target must be GL_TEXTURE_2D!");
			return 0;
		}

		gli::gl::format format = GL.translate(texture.format());
		if (i_IsGammaCorrected)
		{
			if (format.Internal == gli::gl::internalFormat::INTERNAL_RGB_DXT1)
			{
				format.Internal = gli::gl::internalFormat::INTERNAL_SRGB_DXT1;
			}
			else if (format.Internal == gli::gl::internalFormat::INTERNAL_RGBA_DXT1)
			{
				format.Internal = gli::gl::internalFormat::INTERNAL_SRGB_ALPHA_DXT1;
			}
		}

		glm::tvec3<int> const dimensions(texture.dimensions());

		if (gli::is_compressed(texture.format()))
		{
			if (i == 0)
			{
				m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, GL_TEXTURE_CUBE_MAP, format.Internal, format.Internal, format.Type, dimensions.x, dimensions.y, i_WrapType, i_FilterType, i_MipMapCount, 1));
			}
			// allocate memory and load the texture data
			glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format.Internal, dimensions.x, dimensions.y, 0, texture.size(), texture.data());
		}
		else
		{
			if (i == 0)
			{
				m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, GL_TEXTURE_CUBE_MAP, format.Internal, format.External, format.Type, dimensions.x, dimensions.y, i_WrapType, i_FilterType, i_MipMapCount, 1));
			}
			// allocate memory and load the texture data
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format.Internal, dimensions.x, dimensions.y, 0, format.External, format.Type, texture.data());
		}
	}

	SetupTextureParameteres(GL_TEXTURE_CUBE_MAP, i_WrapType, i_FilterType, i_MipMapCount);

	return texId;
}

unsigned int TextureManager::Create1DTexture ( unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned int i_WrapType, unsigned int i_FilterType, void* i_pData, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering )
{
	unsigned int target = GL_TEXTURE_1D;

	unsigned int texId = GenAndBindTexture(target, i_TexUnitId);

	m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, i_FormatInternal, i_FormatExternal, i_FormatType, i_Width, 0, i_WrapType, i_FilterType, i_MipMapCount, 1));
	// allocate memory and load the texture data
	glTexImage1D(target, 0, i_FormatInternal, i_Width, 0, i_FormatExternal, i_FormatType, i_pData);

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount, i_AnisoFiltering);

	return texId;
}

unsigned int TextureManager::Create1DArrayTexture ( unsigned short i_LayerCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned int i_WrapType, unsigned int i_FilterType, void* i_pData, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering )
{
	if (!TextureManager::CheckLayerCount(i_LayerCount))
	{
		ERR("Number of layers exceeds the supported level!");
		return 0;
	}

	unsigned int target = GL_TEXTURE_1D_ARRAY;

	unsigned int texId = GenAndBindTexture(target, i_TexUnitId);

	m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, i_FormatInternal, i_FormatExternal, i_FormatType, i_Width, 0, i_WrapType, i_FilterType, i_MipMapCount, i_LayerCount));
	// allocate memory and load the texture data
	glTexImage2D(target, 0, i_FormatInternal, i_Width, i_LayerCount, 0, i_FormatExternal, i_FormatType, i_pData);

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount, i_AnisoFiltering);

	return texId;
}

unsigned int TextureManager::Create2DTexture ( unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, void* i_pData, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering )
{
	unsigned int target = GL_TEXTURE_2D;

	unsigned int texId = GenAndBindTexture(target, i_TexUnitId);

	m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, i_FormatInternal, i_FormatExternal, i_FormatType, i_Width, i_Height, i_WrapType, i_FilterType, i_MipMapCount, 1));
	// allocate memory and load the texture data
	glTexImage2D(target, 0, i_FormatInternal, i_Width, i_Height, 0, i_FormatExternal, i_FormatType, i_pData);

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount, i_AnisoFiltering);

	return texId;
}

unsigned int TextureManager::Create2DRawTexture ( const std::string& i_ImageFileName, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_DataType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering, unsigned short i_DataOffset )
{
	if (i_FormatExternal != GL_RED && i_FormatExternal != GL_RG &&
		i_FormatExternal != GL_RGB && i_FormatExternal != GL_RGBA &&
		i_FormatExternal != GL_RGB16F && i_FormatExternal != GL_RGBA16F)
	{
		ERR("Only GL_RED, GL_RG, GL_RGB, GL_RGBA, GL_RGB16F and GL_RGBA16F external formats are supported!");
		return 0;
	}

	if (i_DataType != GL_UNSIGNED_BYTE && i_DataType != GL_FLOAT)
	{
		ERR("Only GL_UNSIGNED_BYTE and GL_FLOAT data types are supported!");
		return 0;
	}

	FILE* pF = fopen(i_ImageFileName.c_str(), "rb");

	if (!pF)
	{
		ERR("Failed to open " + i_ImageFileName + " image file name!");
		return 0;
	}

	unsigned char* pUCharData = nullptr;
	float* pFloatData = nullptr;

	unsigned short channelCount = 1;
	switch (i_FormatExternal)
	{
	case GL_RED:
		channelCount = 1;
		break;
	case GL_RG:
		channelCount = 2;
		break;
	case GL_RGB:
	case GL_RGB16F:
		channelCount = 3;
		break;
	case GL_RGBA:
	case GL_RGBA16F:
		channelCount = 4;
		break;
	}

	unsigned long size = i_Width * i_Height * channelCount;

	if (i_DataOffset >= size)
	{
		ERR("Data offset " << i_DataOffset << " is bigger than the size of the image!");
		return 0;
	}

	size += i_DataOffset;

	if (i_DataType == GL_UNSIGNED_BYTE)
	{
		pUCharData = new unsigned char[size];
		assert(pUCharData != nullptr);
		fread(pUCharData, 1, size * sizeof(unsigned char), pF);
	}
	else if (i_DataType == GL_FLOAT)
	{
		pFloatData = new float[size];
		assert(pFloatData != nullptr);
		fread(pFloatData, 1, size * sizeof(float), pF);
	}

	fclose(pF);

	unsigned int target = GL_TEXTURE_2D;

	unsigned int texId = GenAndBindTexture(target, i_TexUnitId);

	m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, i_FormatInternal, i_FormatExternal, i_DataType, i_Width, i_Height, i_WrapType, i_FilterType, i_MipMapCount, 1));
	// allocate memory and load the texture data

	if (i_DataType == GL_UNSIGNED_BYTE)
	{
		glTexImage2D(target, 0, i_FormatInternal, i_Width, i_Height, 0, i_FormatExternal, i_DataType, pUCharData + i_DataOffset);
		
		SAFE_ARRAY_DELETE(pUCharData);
	}
	else if (i_DataType == GL_FLOAT)
	{
		glTexImage2D(target, 0, i_FormatInternal, i_Width, i_Height, 0, i_FormatExternal, i_DataType, pFloatData + i_DataOffset);
		
		SAFE_ARRAY_DELETE(pFloatData);
	}

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount, i_AnisoFiltering);

	return texId;
}

unsigned int TextureManager::Create2DArrayTexture ( unsigned short i_LayerCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, void* i_pData, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering )
{
	if (!TextureManager::CheckLayerCount(i_LayerCount))
	{
		ERR("Number of layers exceeds the supported level!");
		return 0;
	}

	unsigned int target = GL_TEXTURE_2D_ARRAY;

	unsigned int texId = GenAndBindTexture(target, i_TexUnitId);

	m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, i_FormatInternal, i_FormatExternal, i_FormatType, i_Width, i_Height, i_WrapType, i_FilterType, i_MipMapCount, i_LayerCount));
	// allocate memory and load the texture data
	glTexImage3D(target, 0, i_FormatInternal, i_Width, i_Height, i_LayerCount, 0, i_FormatExternal, i_FormatType, i_pData);

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount, i_AnisoFiltering);

	return texId;
}

unsigned int TextureManager::Create3DRawTexture ( const std::string& i_ImageFileName, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_DataType, unsigned short i_Width, unsigned short i_Height, unsigned short i_Depth, unsigned int i_WrapType, unsigned int i_FilterType, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering, unsigned short i_DataOffset )
{
	if (i_FormatExternal != GL_RED && i_FormatExternal != GL_RG &&
		i_FormatExternal != GL_RGB && i_FormatExternal != GL_RGBA &&
		i_FormatExternal != GL_RGB16F && i_FormatExternal != GL_RGBA16F)
	{
		ERR("Only GL_RGB, GL_RGBA, GL_RGB16F and GL_RGBA16F external formats are supported!");
		return 0;
	}

	if (i_DataType != GL_UNSIGNED_BYTE && i_DataType != GL_FLOAT)
	{
		ERR("Only GL_UNSIGNED_BYTE and GL_FLOAT data types are supported!");
		return 0;
	}

	FILE* pF = fopen(i_ImageFileName.c_str(), "rb");

	if (!pF)
	{
		ERR("Failed to open " + i_ImageFileName + " image file name!");
		return 0;
	}

	unsigned char* pUCharData = nullptr;
	float* pFloatData = nullptr;

	unsigned short channelCount = 1;
	switch (i_FormatExternal)
	{
	case GL_RED:
		channelCount = 1;
		break;
	case GL_RG:
		channelCount = 2;
		break;
	case GL_RGB:
	case GL_RGB16F:
		channelCount = 3;
		break;
	case GL_RGBA:
	case GL_RGBA16F:
		channelCount = 4;
		break;
	}

	unsigned long size = i_Width * i_Height * i_Depth * channelCount;

	if (i_DataOffset >= size)
	{
		ERR("Data offset " << i_DataOffset << " is bigger than the size of the image!");
		return 0;
	}

	size += i_DataOffset;

	if (i_DataType == GL_UNSIGNED_BYTE)
	{
		pUCharData = new unsigned char[size];
		assert(pUCharData);
		fread(pUCharData, 1, size * sizeof(unsigned char), pF);
	}
	else if (i_DataType == GL_FLOAT)
	{
		pFloatData = new float[size];
		assert(pFloatData != nullptr);
		fread(pFloatData, 1, size * sizeof(float), pF);
	}

	fclose(pF);

	unsigned int target = GL_TEXTURE_3D;

	unsigned int texId = GenAndBindTexture(target, i_TexUnitId);

	m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, i_FormatInternal, i_FormatExternal, i_DataType, i_Width, i_Height, i_WrapType, i_FilterType, i_MipMapCount, 1));
	// allocate memory and load the texture data
	if (i_DataType == GL_UNSIGNED_BYTE)
	{
		glTexImage3D(target, 0, i_FormatInternal, i_Width, i_Height, i_Depth, 0, i_FormatExternal, i_DataType, pUCharData + i_DataOffset);
		
		SAFE_ARRAY_DELETE(pUCharData);
	}
	else if (i_DataType == GL_FLOAT)
	{
		glTexImage3D(target, 0, i_FormatInternal, i_Width, i_Height, i_Depth, 0, i_FormatExternal, i_DataType, pFloatData + i_DataOffset);
		
		SAFE_ARRAY_DELETE(pFloatData);
	}

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount, i_AnisoFiltering);

	return texId;
}

unsigned int TextureManager::CreateCubeMapTexture ( unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, void* i_pData, short i_TexUnitId, short i_MipMapCount )
{
	unsigned int target = GL_TEXTURE_CUBE_MAP;

	unsigned int texId = GenAndBindTexture(target, i_TexUnitId);

	m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, i_FormatInternal, i_FormatExternal, i_FormatType, i_Width, i_Height, i_WrapType, i_FilterType, i_MipMapCount, 1));

	for (unsigned short i = 0; i < 6; ++i)
	{
		// allocate memory and load the texture data
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, i_FormatInternal, i_Width, i_Height, 0, i_FormatExternal, i_FormatType, nullptr);


		if (i_pData != nullptr)
		{
			// copy data
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, i, i_Width, 0, i_FormatExternal, i_FormatType, i_pData);
		}
	}

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount);

	return texId;
}

unsigned int TextureManager::GenAndBindTexture(unsigned int i_Target, short i_TexUnitId)
{
	if (i_Target != GL_TEXTURE_1D && i_Target != GL_TEXTURE_1D_ARRAY && i_Target != GL_TEXTURE_2D &&
		i_Target != GL_TEXTURE_2D_ARRAY && i_Target != GL_TEXTURE_3D && i_Target != GL_TEXTURE_CUBE_MAP)
	{
		ERR("Tex target is invalid!");
		return 0;
	}

	if (i_TexUnitId >= m_MaxTexUnits)
	{
		ERR("Tex Unit Id exceeds the supported level!");
		return 0;
	}

	unsigned int texId = 0;
	glGenTextures(1, &texId);

	if (i_TexUnitId >= 0) glActiveTexture(GL_TEXTURE0 + i_TexUnitId);
	glBindTexture(i_Target, texId);

	return texId;
}

unsigned int TextureManager::SetupTextureParameteres(unsigned int i_Target, unsigned int i_WrapType, unsigned int i_FilterType, short i_MipMapCount, bool i_AnisoFiltering)
{
	if (i_WrapType != GL_REPEAT && i_WrapType != GL_CLAMP_TO_EDGE)
	{
		ERR("Only GL_REPEAT and GL_CLAMP_TO_EDGE wrap types are supported!");
		return 0;
	}
	
	switch (i_Target)
	{
		case GL_TEXTURE_1D:
		case GL_TEXTURE_1D_ARRAY:
			glTexParameteri(i_Target, GL_TEXTURE_WRAP_S, i_WrapType);
			break;
		case GL_TEXTURE_2D:
		case GL_TEXTURE_2D_ARRAY:
			glTexParameteri(i_Target, GL_TEXTURE_WRAP_S, i_WrapType);
			glTexParameteri(i_Target, GL_TEXTURE_WRAP_T, i_WrapType);
			break;
		case GL_TEXTURE_3D:
		case GL_TEXTURE_CUBE_MAP:
			glTexParameteri(i_Target, GL_TEXTURE_WRAP_S, i_WrapType);
			glTexParameteri(i_Target, GL_TEXTURE_WRAP_T, i_WrapType);
			glTexParameteri(i_Target, GL_TEXTURE_WRAP_R, i_WrapType);
			break;
		default:
			ERR("Invalid target when setting filter type!")
			break;
	}

	if (i_FilterType != GL_NEAREST && i_FilterType != GL_LINEAR)
	{
		ERR("Only GL_NEAREST and GL_LINEAR filter types are supported(not taking mipmaps into account)!");
		return 0;
	}

	glTexParameteri(i_Target, GL_TEXTURE_MAG_FILTER, i_FilterType);
	glTexParameteri(i_Target, GL_TEXTURE_MIN_FILTER, i_FilterType);

	if (i_AnisoFiltering)
	{
		glTexParameterf(i_Target, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_MaxAnisotropy);
	}

	glTexParameteri(i_Target, GL_TEXTURE_BASE_LEVEL, 0);


	if (i_MipMapCount > 1000)
	{
		ERR("MipMap count can not be bigger than 1000!");
		return 0;
	}

	if (i_MipMapCount >= 0)
	{
		if (i_FilterType == GL_NEAREST)
		{
			glTexParameteri(i_Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		}
		else
		{
			glTexParameteri(i_Target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}

		if (i_MipMapCount > 0)
		{
			glTexParameteri(i_Target, GL_TEXTURE_MAX_LEVEL, i_MipMapCount - 1);
		}
		glGenerateMipmap(i_Target);
	}

	return 0;
}

void TextureManager::Update1DTextureData ( unsigned int i_TexId, void* i_pNewData ) const
{
	assert(i_pNewData != nullptr);

	//NOTE! maybe use a std::map instead of std::vector for faster lookup

	for (unsigned short i = 0; i < m_TextureDataArray.size(); ++i)
	{
		if (m_TextureDataArray[i].texId == i_TexId)
		{
			const TextureInfo& ti = m_TextureDataArray[i];

			glBindTexture(ti.target, i_TexId);

			if (ti.target == GL_TEXTURE_1D)
			{
				if (ti.formatInternal == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
					ti.formatInternal == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ||
					ti.formatInternal == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ||
					ti.formatInternal == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
					)
				{
					glCompressedTexImage1D(ti.target, 0, ti.formatInternal, ti.width, 0, ti.width * ti.formatType, i_pNewData);
				}
				else
				{
					glTexImage1D(ti.target, 0, ti.formatInternal, ti.width, 0, ti.formatExternal, ti.formatType, i_pNewData);
				}

				return;
			}
		}
	}
}

void TextureManager::Update2DTextureData ( unsigned int i_TexId, void* i_pNewData ) const
{
	assert(i_pNewData != nullptr);

	//TODO maybe use a std::map instead of std::vector for faster lookup

	for (unsigned short i = 0; i < m_TextureDataArray.size(); ++i)
	{
		if (m_TextureDataArray[i].texId == i_TexId)
		{
			const TextureInfo& ti = m_TextureDataArray[i];

			glBindTexture(ti.target, i_TexId);

			if (ti.target == GL_TEXTURE_2D)
			{
				if (ti.formatInternal == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
					ti.formatInternal == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ||
					ti.formatInternal == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ||
					ti.formatInternal == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
					)
				{
					glCompressedTexImage2D(ti.target, 0, ti.formatInternal, ti.width, ti.height, 0, ti.width * ti.height * ti.formatType, i_pNewData);
				}
				else
				{
					glTexImage2D(ti.target, 0, ti.formatInternal, ti.width, ti.height, 0, ti.formatExternal, ti.formatType, i_pNewData);
				}

				return;
			}
		}
	}
}

void TextureManager::Update2DArrayTextureData ( unsigned int i_TexId, void* i_pNewData ) const
{
	assert(i_pNewData != nullptr);

	//TODO maybe use a std::map instead of std::vector for faster lookup

	for (unsigned short i = 0; i < m_TextureDataArray.size(); ++i)
	{
		if (m_TextureDataArray[i].texId == i_TexId)
		{
			const TextureInfo& ti = m_TextureDataArray[i];

			glBindTexture(ti.target, i_TexId);

			if (ti.target == GL_TEXTURE_2D_ARRAY)
			{
				if (ti.formatInternal == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
					ti.formatInternal == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ||
					ti.formatInternal == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ||
					ti.formatInternal == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
					)
				{
					glCompressedTexImage3D(ti.target, 0, ti.formatInternal, ti.width, ti.height, ti.layerCount, 0, ti.width * ti.height * ti.formatType, i_pNewData);
				}
				else
				{
					glTexImage3D(ti.target, 0, ti.formatInternal, ti.width, ti.height, ti.layerCount, 0, ti.formatExternal, ti.formatType, i_pNewData);
				}

				return;
			}
		}
	}
}

void TextureManager::Update2DTextureSize ( unsigned int i_TexId, unsigned short i_Width, unsigned short i_Height )
{
	for (unsigned short i = 0; i < m_TextureDataArray.size(); ++i)
	{
		if (m_TextureDataArray[i].texId == i_TexId)
		{
			TextureInfo& ti = m_TextureDataArray[i];

			ti.width = i_Width;
			ti.height = i_Height;

			glBindTexture(ti.target, i_TexId);

			// NOTE! Update the texture with NO DATA !!!!
			if (ti.target == GL_TEXTURE_2D)
			{
				if (ti.formatInternal == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
					ti.formatInternal == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ||
					ti.formatInternal == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ||
					ti.formatInternal == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
					)
				{
					glCompressedTexImage2D(ti.target, 0, ti.formatInternal, ti.width, ti.height, 0, ti.width * ti.height * ti.formatType, nullptr);
				}
				else
				{
					glTexImage2D(ti.target, 0, ti.formatInternal, ti.width, ti.height, 0, ti.formatExternal, ti.formatType, nullptr);
				}

				return;
			}
		}
	}
}

void TextureManager::BindTexture ( unsigned int i_TexId,  bool i_GenerateMipMaps, short i_TexUnitId ) const
{
	for (unsigned short i = 0; i < m_TextureDataArray.size(); ++ i)
	{
		if (m_TextureDataArray[i].texId == i_TexId)
		{
			const TextureInfo& ti = m_TextureDataArray[i];

			short texUnitId = ti.texUnitId;
			if (i_TexUnitId >= 0) texUnitId = i_TexUnitId;
			glActiveTexture(GL_TEXTURE0 + texUnitId);
			glBindTexture(ti.target, ti.texId);

			if (i_GenerateMipMaps && ti.mipMapCount >= 0) glGenerateMipmap(ti.target);

			return;
		}
	}
}

unsigned int TextureManager::GetTextureId ( unsigned short i_Index ) const
{
	assert(i_Index < m_TextureDataArray.size());

	return m_TextureDataArray[i_Index].texId;
}

unsigned short TextureManager::GetTextureUnitId ( unsigned short i_Index ) const
{
	assert(i_Index < m_TextureDataArray.size());

	return m_TextureDataArray[i_Index].texUnitId;
}

unsigned int TextureManager::GetDepthTextureId ( void ) const
{
	assert(m_TextureDataArray.size() > 0);

	return m_TextureDataArray[m_TextureDataArray.size() - 1].texId;
}

bool TextureManager::CheckLayerCount ( unsigned short i_LayerCount )
{
	if (i_LayerCount == 0 || i_LayerCount >= m_MaxTextureArrayLayers)
	{
		return false;
	}

	return true;
}
