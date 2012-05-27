/*
 * Author: WORLD PEOPLE
 * E-Mail: world.people.long@gmail.com
 *
 * File: sds.h 
 * Create Date: 2012-05-27 11:16:56
 *
 */
#ifndef _SDS_H_
#define _SDS_H_
#define SDS_MAX_PREALLOC (1024*1024)
#include <sys/types.h>
#include <stdarg.h>
typedef char * sds;
struct sdshdr
{
	int len;
	int free;
	char buf[];
};

static inline size_t sdslen(const sds s)
{
	struct sdshdr *sh = (void *)(s - (sizeof (struct sdshdr)));
	return sh->len;
}

static inline size_t sdsavail(const sds s)
{
	struct sdshdr *sh = (void *)(s - sizeof(struct sdshdr));
	return sh->free;
}

sds sdsNewLen(const void *init, size_t initLen);
sds sdsNew(const char *init);
sds sdsEmpty();
size_t sdsLen(const sds s);
sds sdsDup(const sds s);
void sdsFree(const sds s);
size_t sdsAvail(sds s);
sds sdsGrowZero(sds s, size_t len);
sds sdsCatLen(sds s, void *t, size_t len);
sds sdsCat(sds s, char *t);
sds sdsCatsds(sds s, sds t);
sds sdsCopyLen(sds s, char *t, size_t len);
sds sdsCopy(sds s, char *t);

sds sdscatvprintf(sds s, const char *fmt, va_list ap);
sds sdsCatprintf(sds s, const char *fmt, ...);

sds sdsTrim(sds s, const char *cset);
sds sdsRange(sds s, int start, int end);
void sdsUpdateLen(sds s);
void sdsClear(sds s);
int sdsCmp(sds s1, sds s2);
sds *sdsSplitLen(char *s, int len, char *sep, int selLen, int *count);
void sdsFreeSplitRes(sds *tokens, int cout);
void sdsToLower(sds s);
void sdsToUpper(sds s);
sds sdsFromLonglong(long long value);
sds sdsCatRepr(sds s, char *p, size_t len);
sds *sdsSplitArgs(char *line, int *argc);
void sdsSplitArgsFree(sds *argv, int argc);
sds sdsMapChars(sds s, char *from, char *to, size_t setlen);

sds sdsMakeRoomFor(sds s, size_t addLen);
void sdsIncrLen(sds s, int incr);
sds sdsRemoveFreeSpace(sds s);
size_t sdsAllocSize(sds s);

#endif
