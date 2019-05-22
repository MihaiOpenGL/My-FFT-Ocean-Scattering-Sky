/* Author: BAIRAC MIHAI */

#ifndef SDL_CONFIG_H
#define SDL_CONFIG_H

#if  defined(ANDROID) || defined(__ANDROID__)
#define SDL_MAIN_NEEDED
#elif defined(WIN32) || defined(_WIN32) ||defined(__WIN32__) || defined(__WINDOWS__)
#define SDL_MAIN_HANDLED
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__LINUX__)
#define SDL_MAIN_HANDLED
#elif defined(__APPLE__) || defined(__MACOSX__)
#define SDL_MAIN_HANDLED
#endif

#endif /* SDL_CONFIG_H */