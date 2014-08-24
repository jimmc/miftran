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
/* mtrc - read miftran rc file */

/* Since we have all the stuff to parse out MIF files, we just use that
 * same code for our RC files.  SO the syntax of our RC files is the
 * same as the syntax of MIF files.
 */

#include "mtutil.h"
#include "mttran.h"

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

MtSidTran RcTranTab[] = {
	{ "print", 0, MtProcPrint, 0, 0 },
	{ "eprint", 0, MtProcEPrint, 0, 0 },
	{ "typesub", 0, MtProcTypeSub, 0, 0 },
	{ "stringsub", 0, MtProcStringSub, 0, 0 },
	{ "infilename", 0, MtProcInFileName, 0, 0 },
	{ "outfilename", 0, MtProcOutFileName, 0, 0 },
	{ "altoutfilename", 0, MtProcAltOutFileName, 0, 0 },
	{ 0 }
};

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
