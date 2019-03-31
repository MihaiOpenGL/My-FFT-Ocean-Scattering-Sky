/* Author: BAIRAC MIHAI */

#ifndef LOGGER_H
#define LOGGER_H

#include "AppConfig.h"
#include "SDL/SDL_log.h"
#include "SDL/SDL_error.h"

#ifdef ENABLE_LOG
	#define ERR(args, ...) do { SDL_LogError(SDL_LOG_CATEGORY_ERROR, "FILE: %s, FUNC: %s, LINE: %s, ERR: %s\n, %s", __FILE__, __FUNCTION__, __LINE__, SDL_GetError(), args, ##__VA_ARGS__); } while (0);
	#define LOG(args, ...) do { SDL_Log(args, ##__VA_ARGS__); } while (0);
#else
	#define	ERR(...) do{}while(0);
	#define LOG(...) do{}while(0);
#endif // ENABLE_LOG

#endif /* LOGGER_H */
