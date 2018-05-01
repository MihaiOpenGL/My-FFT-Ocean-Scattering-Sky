/* Author: BAIRAC MIHAI */

#include "FileUtils.h"

#include <fstream>

#include "ErrorHandler.h"


namespace FileUtils
{
	bool LoadTextFile ( const std::string& i_TextFileName, FileUtils::TextFile& o_Text )
	{
		if (i_TextFileName.empty())
		{
			ERR("Empty text file name!");
			return false;
		}

		std::ifstream textFile(i_TextFileName, std::ios::in);

		if (!textFile.good())
		{
			ERR("Failed to open " + i_TextFileName + " text file name!");
			return false;
		}

		o_Text.text.resize(static_cast<unsigned int>(textFile.tellg()));
		o_Text.text = std::string((std::istreambuf_iterator<char>(textFile)), std::istreambuf_iterator<char>());

		textFile.close();

		return true;
	}
}
