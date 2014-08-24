/*
 * Copyright 1993,1994 Globetrotter Software, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Globetrotter Software not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  Globetrotter Software makes
 * no representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * GLOBETROTTER SOFTWARE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL GLOBETROTTER SOFTWARE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * Author:  Jim McBeath, Globetrotter Software, jimmc@globes.com
 */
/* mtop.c - register ops for miftran
 *
 * Jim McBeath, December 16, 1993
 */

#include "mtutil.h"

typedef struct _opintinfo {
	char *name;
	int (*func)();
} OpIntInfo;

typedef struct _opstrinfo {
	char *name;
	char *(*func)();
} OpStrInfo;

int
MtOpIntAdd(n1,n2)
int n1,n2;
{
	return n1+n2;
}

int
MtOpIntSubtract(n1,n2)
int n1,n2;
{
	return n1+n2;
}

int
MtOpIntCopy(n1,n2)
int n1, n2;
{
	return n2;
}

char *
MtOpStrGet(s1,s2)
char *s1, *s2;
{
	char *s;

	s = MtSymGet(s2);
	if (!s) s="";
	return s;
}

char *
MtOpStrSet(s1,s2)
char *s1, *s2;
{
	MtSymSet(s2,s1);
	return s1;
}

char *
MtOpStrCat(s1,s2)
char *s1, *s2;
{
	int l;
	char *s;

	if (!s1) s1="";
	if (!s2) s2="";
	l = strlen(s1)+strlen(s2)+1;
	s = MtMalloc(l);
	strcpy(s,s1);
	strcat(s,s2);
	return s;
}

char *
MtOpStrCopy(s1,s2)
char *s1, *s2;
{
	return MtStrSave(s2);
}

OpIntInfo MtOpIntTab[] = {
	{ "add", MtOpIntAdd },
	{ "+", MtOpIntAdd },
	{ "subtract", MtOpIntSubtract },
	{ "-", MtOpIntSubtract },
	{ "copy", MtOpIntCopy },
	{ 0 },
};

OpStrInfo MtOpStrTab[] = {
	{ "cat", MtOpStrCat },
	{ "copy", MtOpStrCopy },
	{ "get", MtOpStrGet },
	{ "set", MtOpStrSet },
	{ 0 },
};

int
MtOpInt(opname,n1,n2)
char *opname;
int n1;
int n2;
{
	int i;

	for (i=0; MtOpIntTab[i].name; i++) {
		if (strcmp(MtOpIntTab[i].name,opname)==0)
			return (MtOpIntTab[i].func)(n1,n2);
	}
	return n1;	/* not found */
}

/* Returns either s1, or a malloced string which becomes property of caller */
char *
MtOpStr(opname,s1,s2)
char *opname;
char *s1;
char *s2;
{
	int i;

	for (i=0; MtOpStrTab[i].name; i++) {
		if (strcmp(MtOpStrTab[i].name,opname)==0)
			return (MtOpStrTab[i].func)(s1,s2);
	}
	return s1;	/* not found */
}

/* end */
