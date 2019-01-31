/* Author: BAIRAC MIHAI */

#include "Material.h"


Material::Material ( void )
{

}

Material::Material ( const Material::MaterialDetails& i_MaterialDetails )
{
	m_Details = i_MaterialDetails;
}


Material::~Material ( void )
{
}
