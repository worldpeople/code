/*
 * Author: WORLD PEOPLE
 * E-Mail: world.people.long@gmail.com
 *
 * File: sds.c 
 * Create Date: 2012-05-27 12:48:49
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "sds.h"
#include "zmalloc.h"

sds sdsNewLen(const void *init, size_t initLen)
{
	struct sdshdr *sh;
	if(init)
	{
		sh = zmalloc(sizeof(struct sdshdr) + initLen + 1);
	}
	else
	{
		sh = zcalloc(sizeof(struct sdshdr) + initLen + 1);
	}
	if(sh == NULL) return NULL;
 	sh->len = initLen;
	sh->free = 0;
	if(initLen && init)
	{
		memcpy(sh->buf, init, initLen);
	}
	sh->buf[initLen] = '\0';
	return (char *)sh->buf;
}

sds sdsEmpty(void)
{
	return sdsNewLen("", 0);
}
