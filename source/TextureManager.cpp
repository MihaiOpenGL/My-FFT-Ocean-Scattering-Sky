/* Author: BAIRAC MIHAI */

/*
 ! IMPORTANT! TextureManager supports only BMP image loading.

 NOTE ! SINCE OPENGL 3.X GL_CLAMP HAS BEEN REMOVED, USE ONLY GL_CLAMP_TO_EDGE !!!
*/

#include "TextureManager.h"
#include "CommonHeaders.h"
#include "GLConfig.h"
#include "GlobalConfig.h"
#include "glm/vec3.hpp"
#include "SDL/SDL_rwops.h"
#include "SDL/SDL_filesystem.h"
#include "SDL/SDL_surface.h"
#include "SDL/SDL_image.h"
#include <cassert>


int TextureManager::m_MaxTexUnits = 0;
int TextureManager::m_MaxTextureArrayLayers = 0;
float TextureManager::m_MaxAnisotropy = 0.0f;

TextureManager::TextureManager ( void )
	: m_Name("Default"), m_IsTexAnisoFilterSupported(false)
{
	LOG("Texture Manager [%s] successfully created!", m_Name.c_str());
}

TextureManager::TextureManager ( const std::string& i_Name, const GlobalConfig& i_Config)
	: m_Name(i_Name), m_IsTexAnisoFilterSupported(false)
{
	Init(i_Config);
}

TextureManager::~TextureManager ( void )
{
	Destroy();
}

void TextureManager::Initialize(const std::string& i_Name, const GlobalConfig& i_Config)
{
	m_Name = i_Name;

	Init(i_Config);

	LOG("Texture Manager [%s] successfully created!", m_Name.c_str());
}

void TextureManager::Init (const GlobalConfig& i_Config)
{
	m_IsTexAnisoFilterSupported = i_Config.GLExtVars.IsTexAnisoFilterSupported;

	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_MaxTexUnits);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &m_MaxTextureArrayLayers);

	if (m_IsTexAnisoFilterSupported)
	{
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &m_MaxAnisotropy);
	}
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

	LOG("Texture Manager [%s] successfully destroyed!", m_Name.c_str());
}

unsigned int TextureManager::Load1DTexture ( const std::string& i_ImageFileName, unsigned int i_WrapType, unsigned int i_FilterType, bool i_IsGammaCorrected, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering )
{
	char* pBasePath = SDL_GetBasePath();
	std::string imageName(pBasePath ? pBasePath : "");
	imageName += i_ImageFileName;

	SDL_RWops* pF = SDL_RWFromFile(imageName.c_str(), "rb");
	if (!pF)
	{
		ERR("Failed to open image: %s!", imageName.c_str());
		return 0;
	}

	SDL_Surface* pSurface = IMG_LoadBMP_RW(pF);
	if (!pSurface)
	{
		ERR("Failed to load image: %s!, Error: %s", imageName.c_str(), IMG_GetError());
		return 0;
	}

	unsigned int target = GL_TEXTURE_1D;

	unsigned int texId = GenAndBindTexture(target, i_TexUnitId);

	unsigned int format = GL_RGB;
	unsigned int internalFormat = GL_RGB;
	unsigned int type = GL_UNSIGNED_BYTE;

	SetupPixelFormat(pSurface->format, i_IsGammaCorrected, format, internalFormat);

	m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, internalFormat, format, type, pSurface->w, pSurface->h, i_WrapType, i_FilterType, i_MipMapCount, 1));

	glTexImage1D(target, 0, internalFormat, pSurface->w, 0, format, type, pSurface->pixels);

	SDL_FreeSurface(pSurface);

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

	char* pBasePath = SDL_GetBasePath();

	unsigned int texId = 0;
	unsigned int target = GL_TEXTURE_1D_ARRAY;
	for (unsigned short i = 0; i < i_ImageFileNameArray.size(); ++i)
	{
		std::string imageName(pBasePath ? pBasePath : "");
		imageName += i_ImageFileNameArray[i];

		SDL_RWops* pF = SDL_RWFromFile(imageName.c_str(), "rb");
		if (!pF)
		{
			ERR("Failed to open image: %s!", imageName.c_str());
			return 0;
		}

		SDL_Surface* pSurface = IMG_LoadBMP_RW(pF);
		if (!pSurface)
		{
			ERR("Failed to load image: %s!, Error: %s", imageName.c_str(), IMG_GetError());
			return 0;
		}

		unsigned int format = GL_RGB;
		unsigned int internalFormat = GL_RGB;
		unsigned int type = GL_UNSIGNED_BYTE;

		SetupPixelFormat(pSurface->format, i_IsGammaCorrected, format, internalFormat);

		if (i == 0)
		{
			texId = GenAndBindTexture(target, i_TexUnitId);

			m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, internalFormat, format, type, pSurface->w, pSurface->h, i_WrapType, i_FilterType, i_MipMapCount, i_ImageFileNameArray.size()));

			// allocate space for 1d texture array
			glTexImage2D(target, 0, internalFormat, pSurface->w, i_ImageFileNameArray.size(), 0, format, type, nullptr);
		}

		// copy data from each dds image to the 1d texture array layer
		glTexSubImage2D(target, 0, 0, i, pSurface->w, i_ImageFileNameArray.size(), format, type, pSurface->pixels);

		SDL_FreeSurface(pSurface);
	}

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount, i_AnisoFiltering);

	return texId;
}

unsigned int TextureManager::Load2DTexture ( const std::string& i_ImageFileName, unsigned int i_WrapType, unsigned int i_FilterType, bool i_IsGammaCorrected, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering )
{
	char* pBasePath = SDL_GetBasePath();
	std::string imageName(pBasePath ? pBasePath : "");
	imageName += i_ImageFileName;

	SDL_RWops* pF = SDL_RWFromFile(imageName.c_str(), "rb");
	if (!pF)
	{
		ERR("Failed to open image: %s!", imageName.c_str());
		return 0;
	}

	SDL_Surface* pSurface = IMG_LoadBMP_RW(pF);
	if (!pSurface)
	{
		ERR("Failed to load image: %s!, Error: %s", imageName.c_str(), IMG_GetError());
		return 0;
	}

	unsigned int target = GL_TEXTURE_2D;

	unsigned int texId = GenAndBindTexture(target, i_TexUnitId);

	unsigned int format = GL_RGB;
	unsigned int internalFormat = GL_RGB;
	unsigned int type = GL_UNSIGNED_BYTE;

	SetupPixelFormat(pSurface->format, i_IsGammaCorrected, format, internalFormat);

	m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, internalFormat, format, type, pSurface->w, pSurface->h, i_WrapType, i_FilterType, i_MipMapCount, 1));
	glTexImage2D(target, 0, internalFormat, pSurface->w, pSurface->h, 0, format, type, pSurface->pixels);

	SDL_FreeSurface(pSurface);

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

	char* pBasePath = SDL_GetBasePath();

	unsigned int texId = 0;
	unsigned int target = GL_TEXTURE_2D_ARRAY;
	for (unsigned short i = 0; i < i_ImageFileNameArray.size(); ++i)
	{
		std::string imageName(pBasePath ? pBasePath : "");
		imageName += i_ImageFileNameArray[i];

		SDL_RWops* pF = SDL_RWFromFile(imageName.c_str(), "rb");
		if (!pF)
		{
			ERR("Failed to open image: %s!", imageName.c_str());
			return 0;
		}

		SDL_Surface* pSurface = IMG_LoadBMP_RW(pF);
		if (!pSurface)
		{
			ERR("Failed to load image: %s!, Error: %s", imageName.c_str(), IMG_GetError());
			return 0;
		}

		unsigned int format = GL_RGB;
		unsigned int internalFormat = GL_RGB;
		unsigned int type = GL_UNSIGNED_BYTE;

		SetupPixelFormat(pSurface->format, i_IsGammaCorrected, format, internalFormat);

		if (i == 0)
		{
			texId = GenAndBindTexture(target, i_TexUnitId);

			m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, internalFormat, format, type, pSurface->w, pSurface->h, i_WrapType, i_FilterType, i_MipMapCount, i_ImageFileNameArray.size()));

			// allocate space for 2d texture array
			glTexImage3D(target, 0, internalFormat, pSurface->w, pSurface->h, i_ImageFileNameArray.size(), 0, format, type, nullptr);
		}

		// copy data from each dds image to the 2d texture array layer
		glTexSubImage3D(target, 0, 0, 0, i, pSurface->w, pSurface->h, i_ImageFileNameArray.size(), format, type, pSurface->pixels);

		SDL_FreeSurface(pSurface);
	}

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount, i_AnisoFiltering);

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

	unsigned int target = GL_TEXTURE_CUBE_MAP;
	unsigned int texId = GenAndBindTexture(target, i_TexUnitId);

	char* pBasePath = SDL_GetBasePath();

	for (unsigned short i = 0; i < i_ImageFileArray.size(); ++i)
	{
		std::string imageName(pBasePath ? pBasePath : "");
		imageName += i_ImageFileArray[i];

		SDL_RWops* pF = SDL_RWFromFile(imageName.c_str(), "rb");
		if (!pF)
		{
			ERR("Failed to open image: %s!", imageName.c_str());
			return 0;
		}

		SDL_Surface* pSurface = IMG_LoadBMP_RW(pF);
		if (!pSurface)
		{
			ERR("Failed to load image: %s!, Error: %s", imageName.c_str(), IMG_GetError());
			return 0;
		}

		unsigned int format = GL_RGB;
		unsigned int internalFormat = GL_RGB;
		unsigned int type = GL_UNSIGNED_BYTE;

		SetupPixelFormat(pSurface->format, i_IsGammaCorrected, format, internalFormat);

		if (i == 0)
		{
			m_TextureDataArray.push_back(TextureInfo(texId, i_TexUnitId, target, internalFormat, format, type, pSurface->w, pSurface->h, i_WrapType, i_FilterType, i_MipMapCount, 1));
		}
		// allocate memory and load the texture data
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, pSurface->w, pSurface->h, 0, format, type, pSurface->pixels);

		SDL_FreeSurface(pSurface);
	}

	SetupTextureParameteres(target, i_WrapType, i_FilterType, i_MipMapCount);

	return texId;
}

unsigned int TextureManager::Load2DRawTexture(const std::string& i_ImageFileName, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_DataType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering, unsigned short i_DataOffset)
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

	// TODO fix image load - move in FileUtils
	char* basePath = SDL_GetBasePath();
	std::string fileName(basePath ? basePath : "");
	fileName += i_ImageFileName;

	SDL_RWops* pF = SDL_RWFromFile(fileName.c_str(), "rb");
	if (!pF)
	{
		ERR("Failed to open %s image file name!", fileName.c_str());
		return 0;
	}

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

	size_t fileSize = i_Width * i_Height * channelCount;

	if (i_DataOffset >= fileSize)
	{
		ERR("Data offset %d is bigger than the size of the image!", i_DataOffset);
		return 0;
	}

	fileSize += i_DataOffset;

	size_t nbRead = 0;
	unsigned char* pUCharData = nullptr;
	float* pFloatData = nullptr;
	if (i_DataType == GL_UNSIGNED_BYTE)
	{
		pUCharData = new unsigned char[fileSize];
		assert(pUCharData != nullptr);
		nbRead = (size_t)SDL_RWread(pF, pUCharData, sizeof(unsigned char), fileSize);
	}
	else if (i_DataType == GL_FLOAT)
	{
		pFloatData = new float[fileSize];
		assert(pFloatData != nullptr);
		nbRead = (size_t)SDL_RWread(pF, pFloatData, sizeof(float), fileSize);
	}

	if (nbRead != fileSize)
	{
		ERR("Mismatch of bytes read vs file size in bytes!");
		return 0;
	}

	SDL_RWclose(pF);

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

unsigned int TextureManager::Load3DRawTexture(const std::string& i_ImageFileName, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_DataType, unsigned short i_Width, unsigned short i_Height, unsigned short i_Depth, unsigned int i_WrapType, unsigned int i_FilterType, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering, unsigned short i_DataOffset)
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

	// TODO fix image load - move in FileUtils
	char* basePath = SDL_GetBasePath();
	std::string fileName(basePath ? basePath : "");
	fileName += i_ImageFileName;

	SDL_RWops* pF = SDL_RWFromFile(fileName.c_str(), "rb");
	if (!pF)
	{
		ERR("Failed to open %s image file name!", fileName.c_str());
		return 0;
	}

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

	size_t fileSize = i_Width * i_Height * i_Depth * channelCount;

	if (i_DataOffset >= fileSize)
	{
		ERR("Data offset %d is bigger than the size of the image!", i_DataOffset);
		return 0;
	}

	fileSize += i_DataOffset;

	size_t nbRead = 0;
	unsigned char* pUCharData = nullptr;
	float* pFloatData = nullptr;
	if (i_DataType == GL_UNSIGNED_BYTE)
	{
		pUCharData = new unsigned char[fileSize];
		assert(pUCharData);
		nbRead = (size_t)SDL_RWread(pF, pUCharData, sizeof(unsigned char), fileSize);
	}
	else if (i_DataType == GL_FLOAT)
	{
		pFloatData = new float[fileSize];
		assert(pFloatData != nullptr);
		nbRead = (size_t)SDL_RWread(pF, pFloatData, sizeof(unsigned char), fileSize);
	}

	if (nbRead != fileSize)
	{
		ERR("Mismatch of bytes read vs file size in bytes!");
		return 0;
	}

	SDL_RWclose(pF);

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

unsigned int TextureManager::Create2DArrayTexture(unsigned short i_LayerCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, void* i_pData, short i_TexUnitId, short i_MipMapCount, bool i_AnisoFiltering)
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

void TextureManager::SetupPixelFormat(const SDL_PixelFormat* i_pFormat, bool i_IsGammaCorrected, unsigned int& o_FormatExternal, unsigned int& o_FormatInternal)
{
	if (!i_pFormat)
	{
		ERR("Image has invalid pixel format!");
		return;
	}

	if (i_pFormat->format != SDL_PIXELFORMAT_RGB24 &&
		i_pFormat->format != SDL_PIXELFORMAT_BGR24 &&
		i_pFormat->format != SDL_PIXELFORMAT_RGBA32 &&
		i_pFormat->format != SDL_PIXELFORMAT_BGRA32)
	{
		ERR("The supported image formats are: RGB, BGR, RGBA, BGRA!");
		return;
	}

	switch (i_pFormat->format)
	{
	case SDL_PIXELFORMAT_RGB24:
		o_FormatExternal = GL_RGB;
		o_FormatInternal = i_IsGammaCorrected ? GL_SRGB : GL_RGB;
		break;
	case SDL_PIXELFORMAT_BGR24:
		o_FormatExternal = GL_BGR;
		o_FormatInternal = i_IsGammaCorrected ? GL_SRGB : GL_RGB;
		break;
	case SDL_PIXELFORMAT_RGBA32:
		o_FormatExternal = GL_RGBA;
		o_FormatInternal = i_IsGammaCorrected ? GL_SRGB_ALPHA : GL_RGBA;
		break;
	case SDL_PIXELFORMAT_BGRA32:
		o_FormatExternal = GL_BGRA;
		o_FormatInternal = i_IsGammaCorrected ? GL_SRGB_ALPHA : GL_RGBA;
		break;
	}
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

	if (m_IsTexAnisoFilterSupported)
	{
		if (i_AnisoFiltering)
		{
			glTexParameterf(i_Target, GL_TEXTURE_MAX_ANISOTROPY, m_MaxAnisotropy);
		}
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
				glTexImage1D(ti.target, 0, ti.formatInternal, ti.width, 0, ti.formatExternal, ti.formatType, i_pNewData);

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
				glTexImage2D(ti.target, 0, ti.formatInternal, ti.width, ti.height, 0, ti.formatExternal, ti.formatType, i_pNewData);

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
				glTexImage3D(ti.target, 0, ti.formatInternal, ti.width, ti.height, ti.layerCount, 0, ti.formatExternal, ti.formatType, i_pNewData);

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
				glTexImage2D(ti.target, 0, ti.formatInternal, ti.width, ti.height, 0, ti.formatExternal, ti.formatType, nullptr);

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
