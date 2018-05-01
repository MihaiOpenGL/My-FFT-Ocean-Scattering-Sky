/* Author: BAIRAC MIHAI */

#ifndef COMMON_H
#define COMMON_H

#define SAFE_DELETE(a) if( (a) != nullptr ) delete (a); (a) = nullptr;
#define SAFE_ARRAY_DELETE(a) if( (a) != nullptr ) delete[] (a); (a) = nullptr;

#endif /* COMMON_H */