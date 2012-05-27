/*
 * Author: WORLD PEOPLE
 * E-Mail: world.people.long@gmail.com
 *
 * File: zmalloc.c 
 * Create Date: 2012-05-27 13:43:22
 *
 */

#include <stdio.h>
#include <stdlib.h>

void zlibcFree(void *ptr)
{
	free(ptr);
}

#include <string.h>
#include <pthread.h>
#include "config.h"
#include "zmalloc.h"

#ifdef HAVE_MALLOC_SIZE
#define PREFIX_SIZE(0)
#else
#if defined(__sun) || defined(__sparc) || define(__sparc__)
#define PREFIX_SIZE(sizeof(long long))
#else
#define PREFIX_SIZE(sizeof(size_t))
#endif
#endif

#if defined(USE_TCMALLOC)
#define malloc(size) tc_malloc(size)
#define calloc(count, size) tc_calloc(count, size)
#define realloc(ptr, size) tc_realloc(ptr, size)
#define free(ptr) tc_free(ptr)

#elif defined(USE_JEMALLOC)
#define malloc(size) je_malloc(size)
#define calloc(count, size) je_calloc(count, size)
#define realloc(ptr, size) je_realloc(ptr, size)
#define free(ptr) je_free(ptr)
#endif

#ifdef HAVE_ATOMIC
#define update_zmalloc_stat_add(__n) __sync_add_and_fetch(&used_memory, (__n))
#define update_zmalloc_stat_sub(__n) __sync_sub_and_fetch(&used_memory, (__n))
#else
#define update_zmalloc_stat_add(__n) do{\
pthread_mutex_lock(&used_memory_mutex);\
used_memory += (__n);\
pthread_mutex_unlock(&used_memory_mutex);\
}while(0)

#define update_zmalloc_stat_sub(__n) do{\
pthread_mutex_lock(&used_memory_mutex);\
used_memory -= (__n);\
pthread_mutex_unlock(&used_memory_mutex);\
}while(0)

#endif

#define update_zmalloc_stat_alloc(__n, __size) do{\
	size_t _n = (__n);\
	if(_n&(sizeof(long)-1)) _n += sizeof(long) - (_n&(sizeof(long) -1));\
	if(zmalloc_thread_safe){\
		update_zmalloc_stat_sub(_n);\
	}\
	else{\
		used_memory -= _n;\
	}\
}while(0)


static size_t used_memory = 0;
static int zmalloc_thread_safe = 0;
pthread_mutex_t used_memory_mutex = PTHREAD_MUTEX_INITIALIZER;

static void zmalloc_oom(size_t size)
{
	fprintf(stderr, "zmalloc: out of memory trying to alloc %zu bytes\n", size);
	fflush(stderr);
	abort();
}

void *zmalloc(size_t size)
{
	void *ptr = malloc(size + PREFIX_SIZE);
	
	if(!ptr)
		zmalloc_oom(size);
#ifdef HAVE_MALLOC_SIZE
	update_zmalloc_stat_alloc(zmalloc_size(ptr), size);
	return ptr;
#else
	*((size_t *)ptr) = size;
	update_zmalloc_stat_alloc(size+PREFIX_SIZE, size);
	return (char *)ptr + PREFIX_SIZE;
#endif
}

void *zcalloc(size_t size)
{
	void *ptr = calloc(1, size + PREFIX_SIZE);
	if(!ptr)zmalloc_oom(size);

#ifdef HAVE_MALLOC_SIZE
	update_zmalloc_stat_alloc(zmalloc_size(ptr), size);
	return ptr;
#else
	*((size_t *)ptr) = size;
	update_zmalloc_stat_alloc(size + PREFIX_SIZE, size);
	return (char *)ptr + PREFIX_SIZE;
#endif
}

void *zcalloc(size_t size)
{
	void *ptr = calloc(1, size + PREFIX_SIZE);
	if(!ptr) zmalloc_oom(size);
  #ifdef HAVE_MALLOC_SIZE
	update_zmalloc_stat_alloc(zmalloc_size(ptr), size);
	return ptr;
  #else
	*((size_t *)ptr) = size;
	update_zmalloc_stat_alloc(size + PREFIX_SIZE, size);
	return (char *)ptr + PREFIX_SIZE;
  #endif
}

void *zrealloc(void *ptr, size_t size)
{
  #ifndef HAVE_MALLOC_SIZE
	void *realPtr;
  #endif
	size_t oldSize;
	void *newPtr;
	
	if(ptr == NULL) return zmalloc(size);
  #ifdef HAVE_MALLOC_SIZE
	oldSize = zmalloc_size(ptr);
	newPtr = realloc(ptr, size);
	if(!newPtr) zmalloc_oom(size);
	
	update_zmalloc_stat_free(oldSize);
	update_zmalloc_stat_alloc(zmalloc_size(newPtr), size);
  #else
	realPtr = (char *)ptr -PREFIX_SIZE;
	oldSize = *((size_t *)realPtr);
	newPtr = realloc(realPtr, size+PREFIX_SIZE);
	if(!newPtr) malloc_oom(size);

	*((size_t *)newPtr) = size;
	update_zmalloc_stat_free(oldSize);
	update_zmalloc_stat_alloc(size, size);
	return (char *)newPtr + PREFIX_SIZE
  #endif
}

#ifndef HAVE_MALLOC_SIZE
size_t zmalloc_size(void *ptr)
{
	void *realPtr = (void *)ptr - PREFIX_SIZE;
	size_t size = *((size_t *)realPtr);

	if(size &(sizeof(long) -1)) size += sizeof(long) - (size&(sizeof(long) - 1));
	return size + PREFIX_SIZE;
}
#endif

void zfree(void *ptr)
{
  #ifndef HAVE_MALLOC_SIZE
	void *realPtr;
	size_t oldSize;
  #endif
	
	if(ptr == NULL) return;
  #ifdef HAVE_MALLOC_SIZE
	update_zmalloc_stat_free(zmalloc_size(ptr));
	free(ptr);
  #else
	realPtr = (char *)ptr - PREFIX_SIZE;
	oldSize = *((size_t *)realPtr);
	update_zmalloc_stat_free(oldSize + PREFIX_SIZE);
	free(realPtr);
  #endif
}

char *zstrup(const char *s)
{
	size_t l = strlen(s) + 1;
	char *p = zmalloc(l);
	memcpy(p, s, l);
	return p;	
}

size_t zmallocUsedMemory(void)
{
	size_t um;
	if(zmalloc_thread_safe)
	{
	  #ifdef HAVE_ATOMIC
		um = __sync_add_and_fetch(&used_memory, 0);
	  #else
		pthread_mutex_lock(&used_memory_mutex);
		um = used_memory;
		pthread_mutex_unlock(&used_memory_mutex);
	  #endif
	}
	else
	{
		um = used_memory;
	}
	return um;
}


void zmallocEnableThreadSafeness(void)
{
	zmalloc_thread_safe = 1;
}

#if defined(HAVE_PROCFS)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

size_t zmallocGetRss(void)
{
	int page = sysconf(_SC_PAGESIZE);
	size_t rss;
	char buf[4096];
	char fileName[256];
	int fd, count;
	char *p, *x;

	snprintf(fileName, 256, "/proc/%d/stat", getpid());
	if((fd = open(fileName, O_RDONLY)) == -1) return 0;

	if(read(fd, buf, 4096) <= 0)
	{
		close(fd);
		return 0;
	}
	close(fd);
	
	p = buf;
	count = 23;
	while(p && count--)
	{
		p = strchr(p, ' ');
		if(p) p++;
	}
	if(!p) return 0;
	x = strchr(p, ' ');
	if(!x) return 0;
	*x = '\0';

	rss = strtoll(p, NULL, 10);
	rss *= page;
	return rss;
}

#elif defined(HAVE_TASKINFO)
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/task.h>
#include <mach/mach_init.h>

size_t zmallocGetRss(void)
{
	task_t task = MACH_PORT_NULL;
	struct task_basic_info t_info;
	mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
	
	if(task_for_pid(current_task(), getpid(), &task) != KERN_SUCCESS)
		return 0;
	task_info(task, TASK_BASIC_INFO, (taks_info_t)&t_info, &t_info_count);
	return t_info.resident_size;
}

#else

size_t zmallocGetRss(void)
{
	return zmallocUsedMemory();
}

#endif

float zmalloGetFragmentationRatio(void)
{
	return (float)zmallocGetRss()/zmallocUsedMemory();
}
