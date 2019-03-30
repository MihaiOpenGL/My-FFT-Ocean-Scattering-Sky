/* Author: BAIRAC MIHAI */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#if defined(WIN32) || defined(_WIN32) ||defined(__WIN32__) || defined(__WINDOWS__)
#define ENABLE_LOG
//#define ENABLE_GL_ERROR_CHECK
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__LINUX__)
#define ENABLE_LOG
#elif defined(__APPLE__) || defined(__MACOSX__)
#define ENABLE_LOG
#elif defined(__ANDROID__)
#define ENABLE_LOG
#endif

#endif /* APP_CONFIG_H */