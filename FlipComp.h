#ifndef _FLIPCOMP_DYLIB_H_
#define _FLIPCOMP_BYPASS_DYLIB_H_

// This file is to facilitate the export of dll methods when compiling this dll
// and the import of other dll which would use the methods in this dll.

#include "misc/MISC_Dylib.h"

#ifdef COMPILING_FLIPCOMP
#define FLIPCOMP_PUBLIC       MISC_DYLIB_EXPORT
#define FLIPCOMP_INTERNAL     MISC_DYLIB_INTERNAL
#else
#define FLIPCOMP_PUBLIC       MISC_DYLIB_IMPORT
#define FLIPCOMP_INTERNAL
#endif

#endif 
