/*
 * Author: WORLD PEOPLE
 * E-Mail: world.people.long@gmail.com
 *
 * File: zmalloc.h 
 * Create Date: 2012-05-27 13:18:39
 *
 */

#ifndef _ZMALLOC_H_
#define _ZMALLOC_H_
#define __xstr(s) __str(s)
#define __str(s) #s

#if defined(USE_TCMALLOC)
#define ZMALLOC_LIB ("tcmalloc-"__xstr(TC_VERSION_MAJOR)"."__XSTR(TC_VERSION_MINOR))
#include <google/tcmalloc.h>
#if (TV_VERSION_MAJOR == 1 && TC_VERSION_MINOR >= 6) || (TC_VERSION_MAJOR > 1)
#define HAVE_MALLOC_SIZE 1
#define zmalloc_size(p) tc_malloc_size(p)
#else
#error "Newer version of tcmalloc required"
#endif

#elif defined(USE_JEMALLOC)
#define ZMALLOC_LIB("jemalloc-"__xstr(JEMALLOC_VERSION_MAJOR)"."__xstr(JEMALLOC_VERSION_MINOR)"."__xstr(JEMALLOC_VERSION_BUGFIX))
#define JEMALLOC_MANGLE
#include <jemalloc/jemalloc.h>
#if (JEMALLOC_VERSION_MAJOR == 2 && JEMALLOC_VERSION_MINOR >=1) || (JEMALLOC_VERSION_MAJOR > 2)
#define HAVE_MALLOC_SIZE 1
#define zmalloc_size(p) JEMALLOC_P(malloc_usable_size)(p)
#else
#error "Newer version of jemalloc required"
#endif

#elif defined(__APPLE__)
#include <malloc/malloc.h>
#define HAVE_MALLOC_SIZE 1
#define zmalloc_size(p) malloc_size(p)
#endif

#ifndef ZMALLOC_LIB
#define ZMALLOC_LIB "libc"
#endif

void *zmalloc(size_t size);
void *zcalloc(size_t size);
void *zrealloc(void *ptr, size_t size);
void zfree(void *ptr);
char *zstrdup(const char *s);
size_t zmallocUsedMemory(void);
void zmallocEnableThreadSafeness(void);
float zmallocGetFragmentationRatio(void);
size_t zmallocGetRss(void);
void zlibcFree(void *ptr);

#ifndef HAVE_MALLOC_SIZE
size_t zmalloc_size(void *ptr);
#endif

#endif
