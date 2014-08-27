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
/* mtout.c - output routines for miftran */

#include <varargs.h>
#include "mtutil.h"

char *MtOutFileName;
int MtOutFileNum;
char *MtAltOutFileName;
int MtAltOutFileNum;

void
MtSetOutFileName(name)
char *name;
{
	MtOutFileName = MtStrSave(name);
	MtOutFileNum = 0;
}

void
MtSetAltOutFileName(name)
char *name;
{
	MtAltOutFileName = MtStrSave(name);
	MtAltOutFileNum = 0;
}

static void
MtPutRegStr(mti,s)
MtInfo *mti;
char *s;
{
	extern char *MtStringRegisters[];
	extern char *MtOpStrCat();
	char *olds;
	int r;
	
	r = mti->odest;
	olds = MtStringRegisters[r];
	if (olds) {
		MtStringRegisters[r] = MtOpStrCat(olds,s);
		MtFree(olds);
	} else {
		MtStringRegisters[r] = MtStrSave(s);
	}
}

static FILE *
MtGetOfp(mti)
MtInfo *mti;
{
	FILE *ofp;

	if (mti->odest<0)
		ofp = mti->aofp;
	else
		ofp = mti->ofp;
	if (!ofp)
		ofp = stdout;
	return ofp;
}

void
MtPutc(mti,c)
MtInfo *mti;
int c;
{
	if (mti->odest>0) {
		char buf[2];
		buf[0] = c;
		buf[1] = 0;
		MtPutRegStr(mti,buf);
		return;
	}
	fputc(c,MtGetOfp(mti));
}

void
MtPuts(mti,s)
MtInfo *mti;
char *s;
{
	if (mti->odest>0) {
		MtPutRegStr(mti,s);
		return;
	}
	fputs(s,MtGetOfp(mti));
}

void
MtPrintf(mti,fmt,va_alist)
MtInfo *mti;
char *fmt;
va_dcl
{
	va_list pvar;
	FILE *fp;

	va_start(pvar);
	if (mti->odest>0) {
		char buf[2000];
		vsprintf(buf,fmt,pvar);
		MtPutRegStr(mti,buf);
	} else {
		fp = MtGetOfp(mti);
		vfprintf(fp,fmt,pvar);
	}
	va_end(pvar);
}

static int
MtCloseOutputFile(mti,altflag)
MtInfo *mti;
int altflag;	/* 1 to close altfile, 0 to close main */
{
	FILE **fpp;
	char **ofnp;
	int t;

	if (altflag) {
		fpp = &(mti->aofp);
		ofnp = &(mti->aofilename);
	} else {
		fpp = &(mti->ofp);
		ofnp = &(mti->ofilename);
	}
	if (*fpp && (*fpp)!=stdout) {
		t = fflush(*fpp) || fclose(*fpp);
		if (t) {
			MtWarning(mti,"Error closing output file %s",*ofnp);
			return 1;
		}
		*fpp = 0;
	}
	return 0;
}

int	/* 0 if OK, 1 if error */
MtSetOutputFile(mti,num,mode,relflag,altflag)
MtInfo *mti;
int num;	/* number output file to use */
int mode;	/* 0=switch to already open file; 1=open(w); 2=open(a) */
int relflag;	/* if true, add num to current output file number, else abs. */
int altflag;	/* true to use alternate output filename */
{
	int t;
	char fnbuf[500];
	FILE *fp, **fpp;
	char *ofn, **ofnp;
	int *fnp;

	if (altflag) {
		ofn = MtAltOutFileName;
		fnp = &MtAltOutFileNum;
		fpp = &(mti->aofp);
		ofnp = &(mti->aofilename);
	} else {
		ofn = MtOutFileName;
		fnp = &MtOutFileNum;
		fpp = &(mti->ofp);
		ofnp = &(mti->ofilename);
	}
	if (mode==0) {
		if (!*fpp)
			return 1;	/* no file to switch to */
		mti->prevodest = mti->odest;
		mti->odest = altflag?-1:0;
		return 0;
	}
	if (!ofn) return 1;
	t = MtCloseOutputFile(mti,altflag);
	if (t) return t;

	if (relflag)
		*fnp += num;	/* relative */
	else
		*fnp = num;	/* absolute */
	sprintf(fnbuf,ofn,*fnp);
	fp = fopen(fnbuf,(mode==2)?"a":"w");
	if (!fp) {
		MtWarning(mti,"Can't open output file %s",fnbuf);
		return 1;
	}
	if (*ofnp) {
		MtFree(*ofnp);
	}
	*fpp = fp;
	*ofnp = MtStrSave(fnbuf);
	mti->prevodest = mti->odest;
	mti->odest = altflag?-1:0;
	return 0;
}

/* end */
