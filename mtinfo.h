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
/* mtinfo.h */

#include <stdio.h>

typedef char *MtSid;

typedef struct _mttoken {
	int type;
	int alloc;	/* allocated string size for string data */
	char *s;
	int i;
	double d;
} MtToken;
/* We use a stuct (instead of a union) to store the data because we need
 * to keep around the string pointer even when returning a token of a
 * different type, so that we can reuse that allocated data later without
 * having to reallocte.
 */

/* The MtInfo structure gets passed around to just about everything
 * dealing with parsing the MIF file.  */
typedef struct _mtinfo {
    /* Character processing */
	int eof;		/* true when reached eof on input */
	FILE *ifp;		/* input stream pointer */
	char *ifilename;
	int lineno;
	int pushedchar;		/* up to one pushed-back character */

    /* Token processing */
	MtToken token;		/* the current token being returned */
	MtToken pushedtoken;	/* up to one pushed-back token */

    /* Command processing */
	MtSid *ss;		/* array of string IDs */
	int sscount, ssalloc;	/* used and allocated counts for ss */
	int needpopcmd;		/* need to pop cmd after returning END/COMPLT */
	int cmd;		/* MT_CMD_* */
	MtToken *args;		/* args for CMD_COMPLETE */
	int argcount, argalloc;

    /* Translation processing */
	int needpgfstart;
	int needpgfend;
	int infontanchor;	/* set by %H */
	MtSid pgftag;
	MtSid fonttag;
	int markertype;
	int tranerror;	/* set this to abort translation processing */

    /* Output processing */
	int usealtfp;		/* 0 means use ->ofp; 1 means use ->aofp */
	FILE *ofp;
	char *ofilename;
	FILE *aofp;
	char *aofilename;
} MtInfo;

typedef struct _mttypesubnametran {
	int what;
	char *whatstr;
} MtTypeSubNameTran;

/* Token types */
/* EOF -1 */
#define MT_LBRACKET 1
#define MT_RBRACKET 2
#define MT_WORD 3
#define MT_STRING 4
#define MT_INT 5
#define MT_DOUBLE 6

/* Command states */
#define MT_CMD_BEGIN 101
#define MT_CMD_COMPLETE 102
#define MT_CMD_END 103

/* end */
