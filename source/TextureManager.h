/* Author: BAIRAC MIHAI

 This manager uses GLI
 page: http://gli.g-truc.net/0.8.2/index.html
 License: http://gli.g-truc.net/copying.txt

*/

#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <string>
#include <vector>

class GlobalConfig;

/*
 Manager for textures: loads and creates: 2D, 2D arrays and cubemaps textures

 This manager uses the GLI free and open-source library: http://gli.g-truc.net/0.8.2/index.html

 THIS TEXTURE MANAGER LOADS ONLY DDS IMAGES !!!
 
 DXT texture format and Texture compression 
 https://www.opengl.org/wiki/S3_Texture_Compression
*/


class TextureManager
{
public:
	TextureManager(void);
	TextureManager(const std::string& i_Name, const GlobalConfig& i_Config);
	~TextureManager(void);

	void Initialize(const std::string& i_Name, const GlobalConfig& i_Config);
	void Destroy(void);

	// i_MipMapCount can have the values: 
	// -1 - no mipmaps
	// 0 - use mipmaps (full range)
	// n - use mipmaps down to range [size .. n]
	unsigned int Load1DTexture(const std::string& i_ImageFileName, unsigned int i_WrapType, unsigned int i_FilterType, bool i_IsGammaCorrected, short i_TexUnitId = -1, short i_MipMapCount = -1, bool i_AnisoFiltering = true);
	unsigned int Load1DArrayTexture(const std::vector<std::string>& i_ImageFileNameArray, unsigned int i_WrapType, unsigned int i_FilterType, bool i_IsGammaCorrected, short i_TexUnitId = -1, short i_MipMapCount = -1, bool i_AnisoFiltering = true);
	unsigned int Load2DTexture(const std::string& i_ImageFileName, unsigned int i_WrapType, unsigned int i_FilterType, bool i_IsGammaCorrected, short i_TexUnitId = -1, short i_MipMapCount = -1, bool i_AnisoFiltering = true);
	unsigned int Load2DArrayTexture(const std::vector<std::string>& i_ImageFileNameArray, unsigned int i_WrapType, unsigned int i_FilterType, bool i_IsGammaCorrected, short i_TexUnitId = -1, short i_MipMapCount = -1, bool i_AnisoFiltering = true);
	unsigned int LoadCubeMapTexture(const std::string& i_ImageFileName, unsigned int i_WrapType, unsigned int i_FilterType, bool i_IsGammaCorrected, short i_TexUnitId = -1, short i_MipMapCount = -1);
	unsigned int LoadCubeMapTexture(const std::vector<std::string>& i_ImageFileArray, unsigned int i_WrapType, unsigned int i_FilterType, bool i_IsGammaCorrected, short i_TexUnitId = -1, short i_MipMapCount = -1);

	unsigned int Create1DTexture(unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned int i_WrapType, unsigned int i_FilterType, void* i_pData = nullptr, short i_TexUnitId = -1, short i_MipMapCount = -1, bool i_AnisoFiltering = true);
	unsigned int Create1DArrayTexture(unsigned short i_LayerCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned int i_WrapType, unsigned int i_FilterType, void* i_pData = nullptr, short i_TexUnitId = -1, short i_MipMapCount = -1, bool i_AnisoFiltering = true);
	unsigned int Create2DTexture(unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_DataType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, void* i_pData = nullptr, short i_TexUnitId = -1, short i_MipMapCount = -1, bool i_AnisoFiltering = true);
	unsigned int Create2DRawTexture(const std::string& i_ImageFileName, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_DataType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, short i_TexUnitId = -1, short i_MipMapCount = -1, bool i_AnisoFiltering = false, unsigned short i_DataOffset = 0);
	unsigned int Create2DArrayTexture(unsigned short i_LayerCount, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, void* i_pData = nullptr, short i_TexUnitId = -1, short i_MipMapCount = -1, bool i_AnisoFiltering = true);
	unsigned int Create3DRawTexture(const std::string& i_ImageFileName, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_DataType, unsigned short i_Width, unsigned short i_Height, unsigned short i_Depth, unsigned int i_WrapType, unsigned int i_FilterType, short i_TexUnitId = -1, short i_MipMapCount = -1, bool i_AnisoFiltering = false, unsigned short i_DataOffset = 0);
	unsigned int CreateCubeMapTexture(unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, void* i_pData = nullptr, short i_TexUnitId = -1, short i_MipMapCount = -1);

	unsigned int GenAndBindTexture(unsigned int i_Target, short i_TexUnitId);

	unsigned int SetupTextureParameteres(unsigned int i_Target, unsigned int i_WrapType, unsigned int i_FilterType, short i_MipMapCount, bool i_AnisoFiltering = false);

	void Update1DTextureData(unsigned int i_TexId, void* i_pNewData) const;
	void Update2DTextureData(unsigned int i_TexId, void* i_pNewData) const;
	void Update2DArrayTextureData(unsigned int i_TexId, void* i_pNewData) const;
	// NOTE! No update for cubemap textures

	// for now only 2d textures can be updated
	void Update2DTextureSize(unsigned int i_TexId, unsigned short i_Width, unsigned short i_Height);

	void BindTexture(unsigned int i_TexId, bool i_GenerateMipMaps = false, short i_TexUnitId = -1) const;

	unsigned int GetTextureId(unsigned short i_Index) const;
	unsigned short GetTextureUnitId(unsigned short i_Index) const;

	unsigned int GetDepthTextureId(void) const;

	static bool CheckLayerCount(unsigned short i_LayerCount);

private:
	struct TextureInfo
	{
		TextureInfo ( unsigned int i_TexId, unsigned short i_TexUnitId, unsigned int i_Target, unsigned int i_FormatInternal, unsigned int i_FormatExternal, unsigned int i_FormatType, unsigned short i_Width, unsigned short i_Height, unsigned int i_WrapType, unsigned int i_FilterType, unsigned short i_MipMapCount, unsigned int i_LayerCount)
			: texId(i_TexId), texUnitId(i_TexUnitId), target(i_Target), formatInternal(i_FormatInternal), formatExternal(i_FormatExternal), formatType(i_FormatType), width(i_Width), height(i_Height), wrapType(i_WrapType), filterType(i_FilterType), mipMapCount(i_MipMapCount), layerCount(i_LayerCount)
		{}

		unsigned int texId;
		unsigned short texUnitId;
		unsigned int target;

		unsigned int formatInternal;
		unsigned int formatExternal;
		unsigned int formatType;
		unsigned short width;
		unsigned short height;
		unsigned int wrapType;
		unsigned int filterType;
		unsigned short mipMapCount;
		unsigned int layerCount;
	};

	//// Methods ////
	void Init(const GlobalConfig& i_Config);

	//// Variables ////
	// self init
	std::string m_Name;

	// this variable uses the same address space for all instances
	static int m_MaxTexUnits;
	static int m_MaxTextureArrayLayers;
	static float m_MaxAnisotropy;

	std::vector<TextureInfo> m_TextureDataArray;

	bool m_IsTexDDSSupported, m_IsTexAnisoFilterSupported;
};

#endif /* TEXTURE_MANAGER_H */
