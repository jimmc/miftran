/*
 * Copyright 1993-1995 Globetrotter Software, Inc.
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

extern MtTypeSubNameTran MtTypeSubNameTranTab[];

typedef struct _mttagalias {
	struct _mttagalias *next;
	char *alias;	/* translate from this name */
	char *actual;	/* translate to this name */
	MtSid aliassid;
	MtSid actualsid;
} MtTagAlias;

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
	MtSid pgftag;	/* if set, subsitute only when in this type pgf */
	struct _mtstringsubinfo *next;
} MtStringSubInfo;

int MtDoSub;
extern int MtDoFmt;
MtTypeSubInfo *MtTypeSubInfoList;
MtStringSubInfo *MtStringSubInfoList;
MtStringSubInfo *MtTypeStringSubInfoList;
MtTagAlias *MtTagAliasList;

void
MtAddTagAlias(alias,actual)
char *alias;
char *actual;
{
	MtTagAlias *ta;

	ta = MtMalloc(sizeof(ta[0]));
	ta->next = MtTagAliasList;
	ta->alias = MtStrSave(alias);
	ta->actual = MtStrSave(actual);
	ta->aliassid = MtStringToSid(alias);
	ta->actualsid = MtStringToSid(actual);
	MtTagAliasList = ta;
}

char *
MtFindTagAlias(alias)
char *alias;
{
	MtTagAlias *ta;

	for (ta=MtTagAliasList; ta; ta=ta->next) {
		if (strcmp(ta->alias,alias)==0)
			return (ta->actual);
	}
	return (char *)0;
}

MtSid
MtFindSidTagAlias(aliassid)
MtSid aliassid;
{
	MtTagAlias *ta;

	for (ta=MtTagAliasList; ta; ta=ta->next) {
		if (ta->aliassid==aliassid)
			return (ta->actualsid);
	}
	return (MtSid)0;
}

char *
MtFindMTagAlias(from)
char *from;	/* can be from.to format */
{
	static char *buf=0;
	static int bufalloc=0;
	int altlen;
	char *dot, *altfrom, *altfromleft, *altfromright;

	dot = strchr(from,'.');
	if (dot) {
		*dot = 0;	/* split into left and right parts */
		if (!from[0])
			altfromleft = (char *)0;
		else
			altfromleft = MtFindTagAlias(from);
		if (!dot[1])
			altfromright = (char *)0;
		else
			altfromright = MtFindTagAlias(dot+1);
		if (!altfromleft && !altfromright)
			return 0;
		if (!altfromleft)
			altfromleft = from;
		if (!altfromright)
			altfromright = dot+1;
		altlen = strlen(altfromleft)+strlen(altfromright)+2;
		if (altlen>bufalloc) {
			buf = MtRealloc(buf,altlen);
			bufalloc = altlen;
		}
		sprintf(buf,"%s.%s",altfromleft,altfromright);
		altfrom = buf;
	} else {
		altfrom = MtFindTagAlias(from);
	}
	return altfrom;
}

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
MtAddTypeStringSub(mti,pgftype,from,to)
MtInfo *mti;		/* for error messages */
char *pgftype;
char *from;
char *to;
{
	MtStringSubInfo *si;
	MtStringSubInfo **lp;

	si = MtMalloc(sizeof(si[0]));
	si->fromstr = MtStrSave(from);
	si->tostr = MtStrSave(to);
	si->fromlen = strlen(from);
	si->tolen = strlen(to);
	si->delta = si->tolen - si->fromlen;
	if (pgftype) {
		si->pgftag = MtStringToSid(pgftype);
		lp = &MtTypeStringSubInfoList;
	} else {
		si->pgftag = (MtSid)0;	/* any pgf type */
		lp = &MtStringSubInfoList;
	}
	si->next = *lp;
	*lp = si;
}

void
MtAddStringSub(mti,from,to)
MtInfo *mti;		/* for error messages */
char *from;
char *to;
{
	MtAddTypeStringSub(mti,(char *)0,from,to);
}

/* We could improve this by making it something other than a linear list. */
MtTypeSubInfo *
MtFindTypeSub(what,from)
int what;	/* MT_O_* */
MtSid from;	/* e.g. pgf tag */
{
	MtTypeSubInfo *si;

	if (!from) from=MtStringToSid("");
	for (si=MtTypeSubInfoList; si; si=si->next) {
		if (si->what==what && si->from==from)
			return si;
	}
	return (MtTypeSubInfo *)0;
}

MtStringSubInfo *	/* return data structure representing substitution */
MtFindFirstStringSub(mti,p,pq)
MtInfo *mti;
char *p;		/* where to start searching */
char **pq;		/* RETURN pointer to where substituion is to occur */
{
	MtStringSubInfo *si, *si0, *firstsi;
	int i;
	char *q, *firstq;
	MtSid pgftag, pgfaliastag;

	pgftag = mti->pgftag;
	pgfaliastag = MtFindSidTagAlias(pgftag);
	firstq = 0;
	firstsi = 0;
	for (i=0; i<2; i++) {
	  if (i==0) si0=MtTypeStringSubInfoList;
	  else      si0=MtStringSubInfoList;
	  for (si=si0; si; si=si->next) {
	    if (!si->pgftag || si->pgftag==pgftag || si->pgftag==pgfaliastag) {
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
	  }
	  if (firstq) break;
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
		si = MtFindFirstStringSub(mti,p,&q);
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
	if (!MtDoSub || !MtDoFmt) {
		/* no substitutions */
		s = MtTypeSubIntToString(what);
		if (!s) s = "unknown";
		MtPrintf(mti,"[S:%s %s %s]\n",s,MtSidToString(from),data);
		if (!MtDoSub)
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

/* when we didn't find a substitution, we look for the wildcard sub */
int	/* 1 if we did the translation, 0 if not */
MtSubStar(mti,what,data)
MtInfo *mti;
int what;	/* MT_O_* */
char *data;	/* string data for search or output */
{
	static MtSid MtStarSid=0;
	int t;

	if (!MtStarSid) MtStarSid=MtStringToSid("*");
	t = MtSub(mti,what,MtStarSid,data);
	return t;
}

/* do a substution given a tag sid */
int	/* 1 if we did the translation, 0 if not */
MtSubSid(mti,what,from,data)
MtInfo *mti;
int what;	/* MT_O_* */
MtSid from;	/* e.g. paragraph type */
char *data;	/* string data for search or output */
{
	MtSid altfrom;
	int t;

	t = MtSub(mti,what,from,data);
	if (t) return t;
	altfrom = MtFindSidTagAlias(from);
	t = MtSub(mti,what,altfrom,data);
	if (t) return t;
	t = MtSubStar(mti,what,data);
	return t;
}

/* do a substution given a tag string */
int	/* 1 if we did the translation, 0 if not */
MtSubStr(mti,what,fromstr,data)
MtInfo *mti;
int what;	/* MT_O_* */
char *fromstr;	/* e.g. paragraph type */
char *data;	/* string data for search or output */
{
	MtSid from;
	int t;
	char *altfromstr;

	from = MtStringToSid(fromstr);
	t = MtSub(mti,what,from,data);
	if (t) return t;
	altfromstr = MtFindMTagAlias(fromstr);
	if (altfromstr) {
		from = MtStringToSid(altfromstr);
		t = MtSub(mti,what,from,data);
		if (t) return t;
	}
	t = MtSubStar(mti,what,data);
	return t;
}

/* end */
