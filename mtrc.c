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
/* mtrc - read miftran rc file */

/* Since we have all the stuff to parse out MIF files, we just use that
 * same code for our RC files.  SO the syntax of our RC files is the
 * same as the syntax of MIF files.
 */

#include <unistd.h>	/* to get R_OK */
#include "mtutil.h"
#include "mttran.h"

extern char *MtFindMTagAlias();

char **MtIncludeDirs=0;
int MtIncludeDirsCount=0;
int MtIncludeDirsAlloc=0;

MtInfo *RcMti;

int	/* 1 if OK, 0 if not */
MtOkArgTypes(mti,typestr,commandname)
MtInfo *mti;
char *typestr;
char *commandname;	/* for error message */
{
	int i,n;
	int atype;

	n = strlen(typestr);
	if (mti->argcount!=n)
		return 0;
	for (i=0; i<n; i++) {
		atype = mti->args[i].type;
		switch (typestr[i]) {
		case 'S':
			if (atype!=MT_STRING && atype!=MT_WORD) {
				if (commandname) {
					MtFileWarning(mti,
						"bad types for %s command",
						commandname);
				}
				return 0;
			}
			MtUnBackslash(mti->args[i].s);
			break;
		default:
			return 0;	/* should never get here */
		}
	}
	return 1;	/* OK */
}

/* If a processing function detects an error, it should set
 * mti->tranerror=1 to abort */

void
MtProcInit(mti)
MtInfo *mti;
{
	int t;

	if (!MtOkArgTypes(mti,"S","init")) {
		return;		/* ignore it and continue */
	}
	MtSubFmt(mti,mti->args[0].s,"");
}

void
MtProcTagAlias(mti)
MtInfo *mti;
{
	int t;

	if (!MtOkArgTypes(mti,"SS","tagalias")) {
		return;		/* ignore it and continue */
	}
	MtAddTagAlias(mti->args[0].s,mti->args[1].s);
}

void
MtProcPrint(mti)
MtInfo *mti;
{
	int t;

	if (!MtOkArgTypes(mti,"S","print")) {
		return;		/* ignore it and continue */
	}
	printf("%s",mti->args[0].s);
}

void
MtProcEPrint(mti)
MtInfo *mti;
{
	int t;

	if (!MtOkArgTypes(mti,"S","eprint")) {
		return;		/* ignore it and continue */
	}
	fprintf(stderr,"%s",mti->args[0].s);
}

void
MtProcTypeSub(mti)
MtInfo *mti;
{
	int t;

	if (!MtOkArgTypes(mti,"SSS","typesub")) {
		return;		/* ignore it and continue */
	}
	t = MtAddTypeSub(mti,mti->args[0].s,mti->args[1].s,mti->args[2].s);
}

void
MtProcStringSub(mti)
MtInfo *mti;
{
	int t;

	if (!MtOkArgTypes(mti,"SS","stringsub")) {
		return;		/* ignore it and continue */
	}
	t = MtAddStringSub(mti,mti->args[0].s,mti->args[1].s);
}

void
MtProcTypeStringSub(mti)
MtInfo *mti;
{
	int t;

	if (!MtOkArgTypes(mti,"SSS","typestringsub")) {
		return;		/* ignore it and continue */
	}
	t = MtAddTypeStringSub(mti,
		mti->args[0].s,mti->args[1].s,mti->args[2].s);
}

void
MtProcInFileName(mti)
MtInfo *mti;
{
	if (!MtOkArgTypes(mti,"S","infilename")) {
		return;		/* ignore it and continue */
	}
	MtSetInFileName(mti->args[0].s);
}

void
MtProcOutFileName(mti)
MtInfo *mti;
{
	if (!MtOkArgTypes(mti,"S","outfilename")) {
		return;		/* ignore it and continue */
	}
	MtSetOutFileName(mti->args[0].s);
}

void
MtProcAltOutFileName(mti)
MtInfo *mti;
{
	if (!MtOkArgTypes(mti,"S","altoutfilename")) {
		return;		/* ignore it and continue */
	}
	MtSetAltOutFileName(mti->args[0].s);
}

void
MtProcInclude(mti)
MtInfo *mti;
{
	if (!MtOkArgTypes(mti,"S","include")) {
		return;		/* ignore it and continue */
	}
	MtIncludeRcFile(mti,mti->args[0].s);
}

void
MtProcIncludeDir(mti)
MtInfo *mti;
{
	if (!MtOkArgTypes(mti,"S","includedir")) {
		return;		/* ignore it and continue */
	}
	MtAddIncludeDir(mti->args[0].s);
}

MtSidTran RcTranTab[] = {
	{ "init", 0, MtProcInit, 0, 0 },
	{ "tagalias", 0, MtProcTagAlias, 0, 0 },
	{ "print", 0, MtProcPrint, 0, 0 },
	{ "eprint", 0, MtProcEPrint, 0, 0 },
	{ "typesub", 0, MtProcTypeSub, 0, 0 },
	{ "stringsub", 0, MtProcStringSub, 0, 0 },
	{ "typestringsub", 0, MtProcTypeStringSub, 0, 0 },
	{ "infilename", 0, MtProcInFileName, 0, 0 },
	{ "outfilename", 0, MtProcOutFileName, 0, 0 },
	{ "altoutfilename", 0, MtProcAltOutFileName, 0, 0 },
	{ "include", 0, MtProcInclude, 0, 0 },
	{ "includedir", 0, MtProcIncludeDir, 0, 0 },
	{ 0 }
};

MtAddIncludeDir(dir)
char *dir;
{
	if (MtIncludeDirsAlloc==0) {
		MtIncludeDirsAlloc = 10;
		MtIncludeDirs = (char **)MtMalloc(
			MtIncludeDirsAlloc * sizeof(char *));
	}
	if (MtIncludeDirsCount>=MtIncludeDirsAlloc) {
		MtIncludeDirsAlloc *= 2;
		MtIncludeDirs = (char **)MtRealloc((void *)MtIncludeDirs,
			MtIncludeDirsAlloc * sizeof(char *));
	}
	MtIncludeDirs[MtIncludeDirsCount++] = MtStrSave(dir);
}

char *		/* returns path of first readable include file found */
MtFindIncludeFile(filename)
char *filename;
{
	static char *buf=0;
	static int bufalloc=0;
	int lf, l;
	int i;
	char *d;

	if (!filename || !*filename)
		return (char *)0;
	/* Look here (or for absolute) first */
	if (access(filename,R_OK)==0)
		return filename;
	if (filename[0]=='/') {
		/* absolute path, can't prepend directory names */
		return (char *)0;	/* can't read it */
	}
	if (!buf) {
		bufalloc = 1024;
		buf = MtMalloc(bufalloc);
	}
	lf = strlen(filename);
	for (i=0; i<MtIncludeDirsCount; i++) {
		d = MtIncludeDirs[i];
		l = strlen(d)+lf+2;
		if (l>bufalloc) {
			buf = MtRealloc(buf,l);
			bufalloc = l;
		}
		sprintf(buf,"%s/%s",d,filename);
		if (access(buf,R_OK)==0)
			return buf;
	}
	return (char *)0;	/* can't find one */
}

MtIncludeRcFile(mti,filename)
MtInfo *mti;
char *filename;
{
	char *fn;
	FILE *f;
	int t;
	
	fn = MtFindIncludeFile(filename);
	if (!fn) {
		MtWarning(mti,"Can't find include file %s",filename);
		return 1;
	}
	f = fopen(fn,"r");
	if (!f) {
		MtWarning(mti,"Can't open RC include file %s",fn);
		return 1;
	}
	t = MtPushInputFile(RcMti,f,fn);
	return t;	/* continue processing */
}

int
MtReadRcFile(filename)
char *filename;
{
	FILE *f;
	char line[500];
	int t;

	f = fopen(filename,"r");
	if (!f) {
		fprintf(stderr,"Can't open RC file %s\n",filename);
		return 1;
	}
	if (!RcMti) {
		RcMti = MtNewInfo();
	}
	MtSetInputFile(RcMti,f,filename);
	MtPrepareTranTab(RcTranTab);
	t = MtTran(RcMti,RcTranTab);
	fclose(f);
	return t;
}

/* end */
