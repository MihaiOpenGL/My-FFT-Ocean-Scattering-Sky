/* Author: BAIRAC MIHAI */

#ifndef GL_CONFIG_H
#define GL_CONFIG_H

#if defined(ANDROID) || defined(__ANDROID__)
#include "../glad/glad_gl32.h"
#elif defined(WIN32) || defined(_WIN32) ||defined(__WIN32__) || defined(__WINDOWS__)
#include "../glad/glad_gl32.h"
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__LINUX__)
#include "../glad/glad_gl32.h"
#elif defined(__APPLE__) || defined(__MACOSX__)
#include "../glad/glad_gl32.h"
#endif

#endif /* GL_CONFIG_H */