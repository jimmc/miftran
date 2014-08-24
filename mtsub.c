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
/* mtsub.c - output substituion stage */

#include <ctype.h>
#include "mtutil.h"

extern char *strstr ARGS((char *s1, char *s2));

extern MtTypeSubNameTran MtTypeSubNameTranTab[];

typedef struct _mttypesubinfo {
	int what;	/* MT_O_* */
	MtSid from;	/* e.g. paragraph name */
	MtSid to;	/* what to convert it to */
	struct _mttypesubinfo *next;
} MtTypeSubInfo;

typedef struct _mtstringsubinfo {
	char *fromstr;	/* what we look for */
	int fromlen;
	char *tostr;	/* what we replace it with */
	int tolen;
	int delta;	/* tolen-fromlen = change in string size */
	struct _mtstringsubinfo *next;
} MtStringSubInfo;

int MtDoSub;
MtTypeSubInfo *MtTypeSubInfoList;
MtStringSubInfo *MtStringSubInfoList;

int
MtTypeSubStringToInt(whatstr)
char *whatstr;
{
	MtTypeSubNameTran *mt;

	for (mt=MtTypeSubNameTranTab; mt->what; mt++)
		if (strcmp(mt->whatstr,whatstr)==0)
			return mt->what;
	return 0;
}

char *
MtTypeSubIntToString(what)
int what;
{
	MtTypeSubNameTran *mt;

	for (mt=MtTypeSubNameTranTab; mt->what; mt++)
		if (mt->what==what)
			return mt->whatstr;
	return (char *)0;
}

void
MtAddTypeSub(mti,whatstr,from,to)
MtInfo *mti;	/* for error messages */
char *whatstr;	/* startpgf, endpgf, etc. */
char *from;
char *to;
{
	MtTypeSubInfo *si;
	int what;

	MtMakeLower(whatstr);
	what = MtTypeSubStringToInt(whatstr);
	if (!what) {
		MtFileWarning(mti,"Can't find sub type %s",whatstr);
		return;
	}
	si = MtMalloc(sizeof(si[0]));
	si->what = what;
	si->from = MtStringToSid(from);
	si->to = MtStringToSid(to);
	si->next = MtTypeSubInfoList;
	MtTypeSubInfoList = si;
}

void
MtAddStringSub(mti,from,to)
MtInfo *mti;		/* for error messages */
char *from;
char *to;
{
	MtStringSubInfo *si;

	si = MtMalloc(sizeof(si[0]));
	si->fromstr = MtStrSave(from);
	si->tostr = MtStrSave(to);
	si->fromlen = strlen(from);
	si->tolen = strlen(to);
	si->delta = si->tolen - si->fromlen;
	si->next = MtStringSubInfoList;
	MtStringSubInfoList = si;
}

/* We could improve this by making it something other than a linear list. */
MtTypeSubInfo *
MtFindTypeSub(what,from)
int what;	/* MT_O_* */
MtSid from;	/* e.g. pgf tag */
{
	MtTypeSubInfo *si;
	static MtSid starsid=0;

	if (!from) from=MtStringToSid("");
	for (si=MtTypeSubInfoList; si; si=si->next) {
		if (si->what==what && si->from==from)
			return si;
	}
	if (!starsid) starsid=MtStringToSid("*");
	if (from!=starsid)
		return MtFindTypeSub(what,starsid);	/* try wildcard */
	return (MtTypeSubInfo *)0;
}

MtStringSubInfo *	/* return data structure representing substitution */
MtFindFirstStringSub(p,pq)
char *p;		/* where to start searching */
char **pq;		/* RETURN pointer to where substituion is to occur */
{
	MtStringSubInfo *si, *firstsi;
	char *q, *firstq;

	firstq = 0;
	firstsi = 0;
	for (si=MtStringSubInfoList; si; si=si->next) {
		q = strstr(p,si->fromstr);
		if (q) {
			if (!firstq || q<firstq ||
			    (q==firstq && si->fromlen>firstsi->fromlen)) {
				/* found a better match, use it */
				firstq = q;
				firstsi = si;
			}
		}
	}
	if (firstq && pq)
		*pq = firstq;
	return firstsi;
}

char *
MtStringSub(mti,data)
MtInfo *mti;
char *data;
{
	int l, ll;
	int slack;
	int i;
	char *p, *q;
	static int bufalloc=0;
	static char *buf=0;
	MtStringSubInfo *si;

	slack = 0;		/* we have this many chars to expand into */
	p = data;
	while (*p) {
		si = MtFindFirstStringSub(p,&q);
		if (!si)
			return data;	/* done with subs */
		if (si->delta>slack) {
			/* no space, we have to reallocate */
			ll = strlen(data);
			l = ll*2;
			if (l<100) l=100;
			if (l>bufalloc) {
				bufalloc = l;
				buf = MtRealloc(buf,l);
			} else {
				l = bufalloc;
			}
			slack = l - ll;
			l = p-data;
			ll = q-data;
			strcpy(buf,data);
			data = buf;
			p = data+l;
			q = data+ll;
		}
		if (si->delta) {
			/* position the string remainder properly */
			l = strlen(q+si->fromlen);
			/* this 'if' is same as memmove, which may not exist */
			if (si->delta<0)
				for (i=0; i<l+1; i++)
					q[si->tolen+i] = q[si->fromlen+i];
			else
				for (i=l+1; i>=0; i--)
					q[si->tolen+i] = q[si->fromlen+i];
			slack += si->delta;
		}
		memcpy(q,si->tostr,si->tolen);
			/* put in the translation */
		p = q+si->tolen;
	}
	return data;
}

int	/* 1 if we did the translation, 0 if not */
MtSub(mti,what,from,data)
MtInfo *mti;
int what;	/* MT_O_* */
MtSid from;	/* e.g. paragraph type */
char *data;	/* string data for search or output */
{
	char *s;
	MtTypeSubInfo *si;

	if (!data) data="";
	if (!MtDoSub) {
		/* no substitutions */
		s = MtTypeSubIntToString(what);
		if (!s) s = "unknown";
		MtPrintf(mti,"%s %s %s\n",s,MtSidToString(from),data);
		return 1;
	}
	si = MtFindTypeSub(what,from);
	if (!si)
		return 0;	/* no translation, ignore this text */
	if (*data)
		data = MtStringSub(mti,data);
	MtSubFmt(mti,si->to,data);
	return 1;
}

/* end */
