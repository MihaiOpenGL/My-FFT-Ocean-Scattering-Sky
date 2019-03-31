/* Author: BAIRAC MIHAI */

#ifndef GL_EXTENSION_VARS_H
#define GL_EXTENSION_VARS_H

#include "GLConfig.h"

struct GLExtVars
{
	char RequiredGLExtensions[193] = "GL_EXT_geometry_shader4 - soft requirement\nGL_ARB_compute_shader - soft requirement\nGL_EXT_texture_filter_anisotropic - hard requirement\nGL_EXT_texture_compression_s3tc - hard requirement\n";

	bool IsGeometryStageSupported;
	bool IsComputeStageSupported;
	bool IsTexAnisoFilterSupported;
	bool IsTexDDSSupported;

	void Initialize()
	{
		IsGeometryStageSupported = GLAD_GL_EXT_geometry_shader4 || GLAD_GL_VERSION_3_2;
		IsComputeStageSupported = GLAD_GL_ARB_compute_shader || GLAD_GL_VERSION_4_3;
		IsTexAnisoFilterSupported = GLAD_GL_EXT_texture_filter_anisotropic || GLAD_GL_VERSION_4_6;
		IsTexDDSSupported = GLAD_GL_EXT_texture_compression_s3tc;
	}
};

#endif /* GL_EXTENSION_VARS_H */