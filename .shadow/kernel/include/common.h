#ifndef COMMON_H__
#define COMMON_H__

#include <kernel.h>
#include <klib.h>
#include <klib-macros.h>

#ifdef DEBUG 
# define dassert(cond) assert(cond)
# define dpanic_on(cond, s) panic_on(cond, s)
#else
# define dassert(cond) 
# define dpanic_on(cond, s)
#endif

#ifdef DEBUGLOG
# define logging(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
# define logging(fmt, ...) 
#endif

#endif
