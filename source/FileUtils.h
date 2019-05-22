/* Author: BAIRAC MIHAI */

#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>

/*
 A small helper function to load txt files, basically used to read files
*/

namespace FileUtils
{
	bool LoadFile(const std::string& i_FileName, std::string& o_Data);
}

#endif /* FILE_UTILS_H */
