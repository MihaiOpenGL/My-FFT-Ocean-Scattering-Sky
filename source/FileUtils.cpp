/* Author: BAIRAC MIHAI */

#include "FileUtils.h"
#include "CommonHeaders.h"
#include "SDL/SDL_rwops.h"
#include "SDL/SDL_filesystem.h"
#include <cstdio>
#include <cassert>

namespace FileUtils
{
	// TODO - add format
	bool LoadFile(const std::string& i_FileName, std::string& o_Data)
	{
		if (i_FileName.empty())
		{
			ERR("Empty file name!");
			return false;
		}

		char* basePath = SDL_GetBasePath();
		std::string fileName(basePath ? basePath : "");
		fileName += i_FileName;
		SDL_RWops* pF = SDL_RWFromFile(fileName.c_str(), "rb");
		if (!pF)
		{
			ERR("Failed to open %s file!", i_FileName.c_str());
			return false;
		}

		size_t fileSize = (size_t)SDL_RWsize(pF);

		char* pData = new char[fileSize + 1];
		assert(pData != nullptr);

		size_t nbRead = (size_t)SDL_RWread(pF, pData, sizeof(char), fileSize);

		if (nbRead != fileSize)
		{
			ERR("Mismatch of bytes read vs file size in bytes!");
			return false;
		}

		pData[fileSize] = '\0';
		o_Data = pData;

		SAFE_ARRAY_DELETE(pData);

		SDL_RWclose(pF);

		return true;
	}
}
