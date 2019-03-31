/* Author: BAIRAC MIHAI */

#ifndef COMMON_H
#define COMMON_H

#include "Logger.h"
#include "GLErrorHandler.h"

// safe delete
#define SAFE_DELETE(a) if( (a) != nullptr ) delete (a); (a) = nullptr;
#define SAFE_ARRAY_DELETE(a) if( (a) != nullptr ) delete[] (a); (a) = nullptr;

#endif /* COMMON_H */