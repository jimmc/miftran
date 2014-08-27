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
/* main.c for miftran */

#include "mtutil.h"
#include "mtlex.h"
#include "mtcmd.h"
#include "version.h"

extern int MtDoSub;
extern int MtDoFmt;

#define PROC_LEX 1
#define PROC_CMD 2
#define PROC_TRAN 3
#define PROC_SUB 4
#define PROC_FMT 5
static int procmode=PROC_FMT;

static int GotRcFile;

char *MtInFileName;

void
MtSetInFileName(s)
char *s;
{
	MtInFileName = MtStrSave(s);
}

void
ReadRcFile()
{
	char *s;

	s = getenv("MIFTRANRC");
	if (s)
		MtReadRcFile(s);
	else
		MtReadRcFile("miftran.rc");	/* default filename */
	/* TBD - should check for errors */
	GotRcFile = 1;
}

void
DoFile(filename)
char *filename;
{
	FILE *f;
	MtInfo *mti;

	if (!GotRcFile) {
		ReadRcFile();
	}
	if (strcmp(filename,"-")==0) {
		filename = "(stdin)";
		f = stdin;
	} else {
		f = fopen(filename,"r");
		if (!f) {
			fprintf(stderr,"Can't open input file %s\n",filename);
			exit(1);
		}
	}
	mti = MtNewInfo();
	MtSetInputFile(mti,f,filename);
	switch (procmode) {
	case PROC_LEX:
		while (MtGetToken(mti)) {
			MtPrintToken(stdout,mti,&(mti->token));
			fputc('\n',stdout);
		}
		break;
	case PROC_CMD:
		while (MtGetCmd(mti)) {
			MtPrintCmd(stdout,mti);
		}
		break;
	case PROC_TRAN:
		MtDoSub = 0;
		MtDoFmt = 0;
		MtProcTop(mti);
		break;
	case PROC_SUB:
		MtDoSub = 1;
		MtDoFmt = 0;
		MtProcTop(mti);
		break;
	case PROC_FMT:
		MtDoSub = 1;
		MtDoFmt = 1;
		MtProcTop(mti);
		break;
	default:
		fprintf(stderr,"No processing mode specified\n");
		exit(1);
	}
	if (f!=stdin)
		fclose(f);
	MtReleaseInfo(mti);	/* done with it */
}

static
void
usage(f)
FILE *f;
{
fprintf(f,"usage: miftran [options] [infiles]\n");
fprintf(f,"-cmd    Stop after command level parse (for debugging)\n");
fprintf(f,"-fmt    Stop after final output formatting (default)\n");
fprintf(f,"-help   Print out this help text.\n");
fprintf(f,"-lex    Stop after lexical level tokenization (for debugging)\n");
fprintf(f,"-rc file  Specify rc filename to use\n");
fprintf(f,"-sub    Stop after substition, before formatting (for debugging)\n");
fprintf(f,"-tran   Stop after translation level parse (for debugging)\n");
fprintf(f,"-version  Print current version info\n");
fprintf(f,"-I dir  Add directory to scan for <include> files\n");
fprintf(f,"Processing order: lex, cmd, tran, sub, fmt.\n");
fprintf(f,"Switches and input files are effectively immediately,\n");
fprintf(f,"so you can use e.g. '-lex file1 -tran file2'\n");
fprintf(f,"An input file of '-' means stdin; if no input files, uses stdin\n");
fprintf(f,"(unless -help or -version specified).\n");
}

static
void
printversion()
{
	printf("miftran version %d.%d", VERSION, REVISION);
#if PATCHLEVEL
	printf(".%d",PATCHLEVEL);
#endif
	printf(" %s\n%s\n",vdate,copyright);
}

main(argc,argv)
int argc;
char *argv[];
{
	int i;
	char *cmd;
	int gotfile=0;

	MtDoFmt = 1;	/* so that 'init' statements work */
	for (i=1; i<argc; i++) {
		cmd = argv[i];
		if (strcmp(cmd,"-cmd")==0)
			procmode=PROC_CMD;  /* do command level parse */
		else if (strcmp(cmd,"-fmt")==0)
			procmode=PROC_FMT;  /* do formatting */
		else if (strcmp(cmd,"-I")==0) {
			if (++i >= argc) {
				fprintf(stderr,"No value for -I\n");
				exit(2);
			}
			MtAddIncludeDir(argv[i]);
		}
		else if (strcmp(cmd,"-lex")==0)
			procmode=PROC_LEX;  /* do lexical level parse */
		else if (strcmp(cmd,"-rc")==0) {
			if (++i >= argc) {
				fprintf(stderr,"No value for -rc\n");
				exit(2);
			}
			MtReadRcFile(argv[i]);
			/* TBD - check for errors? */
			GotRcFile = 1;
		}
		else if (strcmp(cmd,"-sub")==0)
			procmode=PROC_SUB;  /* do substituion level */
		else if (strcmp(cmd,"-tran")==0)
			procmode=PROC_TRAN;  /* do translation level parse */
		else if (strcmp(cmd,"-version")==0) {
			printversion();
			gotfile=1;
		}
		else if (strcmp(cmd,"-")==0) {
			DoFile("-");
			gotfile=1;
		}
		else if (strcmp(cmd,"-help")==0 || strcmp(cmd,"-?")==0) {
			usage(stdout);	/* print usage to stdout */
			gotfile=1;	/* treat that as getting a file */
		}
		else if (cmd[0]=='-') {
			fprintf(stderr,"Unknown switch %s\n",cmd);
			usage(stderr);	/* on error, print usage to stderr */
			exit(3);
		}
		else {
			/* must be a file argument, process it */
			DoFile(cmd);
			gotfile=1;
		}
	}
	if (!gotfile) {
		if (!GotRcFile) {
			ReadRcFile();
		}
		if (MtInFileName)
			DoFile(MtInFileName);
		else
			DoFile("-");	/* use stdin */
	}
	exit(0);
}

/* end */
