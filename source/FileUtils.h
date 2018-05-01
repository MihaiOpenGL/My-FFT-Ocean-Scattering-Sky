/* Author: BAIRAC MIHAI */

#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>

/*
 A small helper function to load txt files, basically used to load shader code
*/

namespace FileUtils
{
	struct TextFile
	{
		std::string text;
	};

	bool LoadTextFile ( const std::string& i_TextFileName, FileUtils::TextFile& o_Text );

}

#endif /* FILE_UTILS_H */
