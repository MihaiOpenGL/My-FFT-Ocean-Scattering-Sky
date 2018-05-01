/* Author: BAIRAC MIHAI */

#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include "GL/glew.h"

#include <iostream>
#include <string>

#define DO_LOG_ERR

#ifdef DO_LOG_ERR
	#define ERR(msg) do { std::cout << msg << "\nFILE: " << __FILE__ << ", FUNC: " << __FUNCTION__  << ", LINE: " << __LINE__ << std::endl; } while (0);
	#define LOG(msg) do { std::cout << msg << std::endl; } while (0);
#else
	#define	ERR(...) do{}while(0);
	#define LOG(...) do{}while(0);
#endif

///// OpenGL Error checking
#ifdef ENABLE_ERROR_CHECK
#define ERROR_CHECK_START glGetError();

#define ERROR_CHECK_END \
std::cout << "OpenGL Error: "; \
switch (glGetError()) \
{ \
case GL_INVALID_ENUM: \
	std::cout << "Invalid Enum!\n"; break; \
case GL_INVALID_VALUE: \
	std::cout << "Invalid Value!\n"; break; \
case GL_INVALID_OPERATION: \
	std::cout << "Invalid Operation!\n"; break; \
case GL_INVALID_FRAMEBUFFER_OPERATION: \
	std::cout << "Invalid Frame buffer Operation!\n"; break; \
case GL_OUT_OF_MEMORY: \
	std::cout << "Out of Memory!\n"; break; \
case GL_STACK_UNDERFLOW: \
	std::cout << "Stack Underflow!\n"; break; \
case GL_STACK_OVERFLOW: \
	std::cout << "Stack Overflow!\n"; break; \
default: \
	std::cout << "No Error!\n"; break; \
}
#else
#define ERROR_CHECK_START
#define ERROR_CHECK_END 
#endif
/////

#endif /* ERRORHANDLER_H */
