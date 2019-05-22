/* Author: BAIRAC MIHAI */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#if defined(ANDROID) || defined(__ANDROID__)
//#define ENABLE_LOG //TODO
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#define ENABLE_LOG
//#define ENABLE_GL_ERROR_CHECK
#define USE_GUI
#define USE_FFTW
#elif defined(__APPLE__) || defined(__MACOSX__)
#define ENABLE_LOG
#define USE_GUI
#define USE_FFTW
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__LINUX__)
#define ENABLE_LOG
#define USE_GUI
#define USE_FFTW
#endif

#endif /* APP_CONFIG_H */