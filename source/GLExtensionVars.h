/* Author: BAIRAC MIHAI */

#ifndef GL_EXTENSION_VARS_H
#define GL_EXTENSION_VARS_H

#include "GLConfig.h"

struct GLExtVars
{
	// TODO - update
	char RequiredGLExtensions[193] = "GL_EXT_geometry_shader4 - soft requirement\nGL_ARB_compute_shader - soft requirement\nGL_EXT_texture_filter_anisotropic - hard requirement\n";

	bool IsGeometryShaderSupported;
	bool IsComputeShaderSupported;
	bool IsTexAnisoFilterSupported;

	void Initialize()
	{
		IsGeometryShaderSupported = GLAD_GL_VERSION_3_2;
		IsComputeShaderSupported = GLAD_GL_ARB_compute_shader && GLAD_GL_ARB_arrays_of_arrays && GLAD_GL_ARB_enhanced_layouts &&
			(GLAD_GL_ARB_shader_image_load_store || GLAD_GL_EXT_shader_image_load_store);
		IsTexAnisoFilterSupported = GLAD_GL_ARB_texture_filter_anisotropic || GLAD_GL_EXT_texture_filter_anisotropic;
	}
};

#endif /* GL_EXTENSION_VARS_H */