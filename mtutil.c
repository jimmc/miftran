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
/* mtutil.c - utility functions for Mif parser */

#include <stdio.h>
#include <varargs.h>
#include <ctype.h>
#include "mtinfo.h"

static MtToken zapToken = {0};
static MtInfo zapMti = {0};

extern void *malloc(), *realloc();

void
MtWarning(mti,fmt,va_alist)
MtInfo *mti;
char *fmt;
va_dcl
{
	va_list pvar;

	va_start(pvar);
	fprintf(stderr,"Warning: ");
	vfprintf(stderr,fmt,pvar);
	fputs("\n",stderr);
	va_end(pvar);
}

void
MtFileWarning(mti,fmt,va_alist)
MtInfo *mti;
char *fmt;
va_dcl
{
	va_list pvar;

	va_start(pvar);
	fprintf(stderr,"Warning: line %d, file %s: ",
			mti->lineno,mti->ifilename);
	vfprintf(stderr,fmt,pvar);
	fputs("\n",stderr);
	va_end(pvar);
}

char *
MtTokenTypeToString(tokentype)
int tokentype;
{
	static char buf[40];

	switch (tokentype) {
	case EOF:		return "EOF";
	case MT_LBRACKET:	return "<";
	case MT_RBRACKET:	return ">";
	case MT_WORD:		return "WORD";
	case MT_STRING:		return "STRING";
	case MT_INT:		return "INT";
	case MT_DOUBLE:		return "DOUBLE";
	default:
		(void)sprintf(buf,"(type=%d)",tokentype);
		return buf;
	}
}

void
MtNoMem(n)
unsigned int n;
{
	fprintf(stderr,"Out of memory: requested size=%d\n",n);
	exit(2);
}

void *
MtMalloc(n)
unsigned int n;
{
	void *p;

	p = malloc(n);
	if (p) return p;
	MtNoMem(n);
	/* NOTREACHED */
}

void *
MtRealloc(oldp,n)
void *oldp;
unsigned int n;
{
	void *p;

	if (!oldp)
		return MtMalloc(n);
	p = realloc(oldp,n);
	if (p) return p;
	MtNoMem(n);
	/* NOTREACHED */
}

void
MtFree(p)
void *p;
{
	free(p);
}

void
MtUnBackslash(p)
char *p;
{
	char *s, *d;

	s = d = p;	/* compress in place */

	while (*s) {
		if (*s=='\\') {
			s++;
			switch (*s) {
			case 0: break;	/* \ at eos, ignore it */
			case 'n': *d++='\n'; s++; break;
			case 't': *d++='\t'; s++; break;
			case '\\': *d++='\\'; s++; break;
			default: *d++ = *s++; break;
			/* TBD - look for octal numbers */
			}
		} else {
			*d++ = *s++;
		}
	}
	*d = 0;
}

/* Allocate and return an MtInfo structure */
MtInfo *
MtNewInfo()
{
	MtInfo *mti;

	mti = MtMalloc(sizeof(MtInfo));
	*mti = zapMti;
	return mti;
}

/* free up the memory used by the MtInfo */
void
MtReleaseInfo(mti)
MtInfo *mti;
{
	int i;

	for (i=0; i<mti->argalloc; i++) {
		if (mti->args[i].s)
			MtFree(mti->args[i].s);
	}
	if (mti->args)
		MtFree(mti->args);
	if (mti->token.s)
		MtFree(mti->token.s);
	if (mti->ss)
		MtFree(mti->ss);
	if (mti->ofilename)
		MtFree(mti->ofilename);
	if (mti->aofilename)
		MtFree(mti->aofilename);
	MtFree(mti);
}

void
MtSetInputFile(mti,f,filename)
MtInfo *mti;
FILE *f;
char *filename;
{
	mti->ifp = f;
	mti->ifilename = filename;
	mti->lineno = 1;
}

/* The StringToSid stuff implements a simple symbol table to allow converting
 * a string to an ID which can later be directly compared */
typedef struct _strentry {
	char *s;
	struct _strentry *next;
} StrEntry;
static StrEntry *StrEntryTable;

MtSid
MtStringToSid(s)
char *s;
{
	StrEntry *se;

	if (!s) s="";
	for (se=StrEntryTable; se; se=se->next)
		if (strcmp(s,se->s)==0)
			return (MtSid)(se->s);
	se = MtMalloc(sizeof(StrEntry));
	se->s = MtMalloc(strlen(s)+1);
	(void)strcpy(se->s,s);
	se->next = StrEntryTable;
	StrEntryTable = se;
	return (MtSid)(se->s);
}

char *
MtSidToString(sid)
MtSid sid;
{
	if (!sid)
		sid = MtStringToSid("");
	return (char *)sid;
}

char *
MtStrSave(s)
char *s;
{
	char *d;

	d = MtMalloc(strlen(s)+1);
	strcpy(d,s);
	return d;
}

char *
MtMakeLower(s)
char *s;
{
	char *p;

	for (p=s; *p; p++)
		if (isupper(*p))
			*p = tolower(*p);
}

/* Dumb symbol table stuff.  Useful for storing a few symbols. */
typedef struct _mtsym {
	struct _mtsym *next;
	char *name;
	char *value;
} MtSym;

static MtSym *MtSymBase;

MtSym *
MtSymGetI(name)
char *name;
{
	MtSym *mts;
	
	for (mts=MtSymBase; mts; mts=mts->next) {
		if (strcmp(mts->name,name)==0)
			return mts;
	}
	return (MtSym *)0;
}

char *
MtSymGet(name)
char *name;
{
	MtSym *mts;
	
	mts = MtSymGetI(name);
	if (mts)
		return mts->value;
	return (char *)0;
}

void
MtSymSet(name,value)
char *name;
char *value;
{
	MtSym *mts;

	mts = MtSymGetI(name);
	if (mts) {
		MtFree(mts->value);
		mts->value = MtStrSave(value);
		return;
	}
	mts = MtMalloc(sizeof(*mts));
	mts->name = MtStrSave(name);
	mts->value = MtStrSave(value);
	mts->next = MtSymBase;
	MtSymBase = mts;
}

/* end */
