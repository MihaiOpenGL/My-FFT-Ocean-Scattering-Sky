/* Author: BAIRAC MIHAI */

#include "Material.h"
#include "CommonHeaders.h"

Material::Material(void)
	: Name("Default")
{
	LOG("Material [%s] successfully created!", Name.c_str());
}

Material::Material(const std::string& i_Name)
	: Name(i_Name)
{
	LOG("Material [%s] successfully created!", Name.c_str());
}

Material::~Material ( void )
{
	LOG("Material [%s] successfully destroyed!", Name.c_str());
}
