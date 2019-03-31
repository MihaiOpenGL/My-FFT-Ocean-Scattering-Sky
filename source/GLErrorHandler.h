/* Author: BAIRAC MIHAI */

#ifndef GL_ERROR_HANDLER_H
#define GL_ERROR_HANDLER_H

#include "AppConfig.h"
#include "GLConfig.h"
#include "Logger.h"

///// OpenGL Error checking
#ifdef ENABLE_GL_ERROR_CHECK
#define GL_ERROR_CHECK_START glGetError();
#define GL_ERROR_CHECK_END \
LOG("OpenGL Error: "); \
switch (glGetError()) \
{ \
case GL_INVALID_ENUM: \
	LOG("Invalid Enum!"); break; \
case GL_INVALID_VALUE: \
	LOG("Invalid Value!"); break; \
case GL_INVALID_OPERATION: \
	LOG("Invalid Operation!"); break; \
case GL_INVALID_FRAMEBUFFER_OPERATION: \
	LOG("Invalid Frame buffer Operation!"); break; \
case GL_OUT_OF_MEMORY: \
	LOG("Out of Memory!"); break; \
case GL_STACK_UNDERFLOW: \
	LOG("Stack Underflow!"); break; \
case GL_STACK_OVERFLOW: \
	LOG("Stack Overflow!"); break; \
default: \
	LOG("No Error!"); break; \
}
#else
#define GL_ERROR_CHECK_START
#define GL_ERROR_CHECK_END 
#endif // ENABLE_GL_ERROR_CHECK
/////

#endif /* GL_ERROR_HANDLER_H */
