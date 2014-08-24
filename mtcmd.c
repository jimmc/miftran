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
/* mtcmd.c - commend level parsing of MIF files */

#include "mtutil.h"
#include "mtlex.h"
#include "mtcmd.h"

static MtToken zapArgToken = {0};

int	/* returns 0 if no more to do, 1 if stuff in mti (code in mti->cmd) */
MtGetCmd(mti)
MtInfo *mti;
{
	char *xtypestr;
	int sz, i;
	MtSid sid;
	char moreinfo[30];
	char *s;

	if (mti->needpopcmd) {
		/* Pop the top command off first */
		if (mti->sscount==0) {
			MtWarning(mti,
				"Attempt to pop command when nothing to pop");
			return 0;	/* this error should never happen */
		}
		mti->sscount--;
		mti->needpopcmd = 0;
	}
	mti->argcount = 0;
	if (!MtGetToken(mti))
		return 0;	/* no more commands */
	switch (mti->token.type) {
	case MT_RBRACKET:
		mti->needpopcmd = 1;
		mti->cmd = MT_CMD_END;	/* end of the command */
		return 1;
	case MT_LBRACKET:	/* new command nesting */
		if (!MtGetToken(mti)) {
			MtFileWarning(mti,"Expected WORD after <, got EOF");
			return 0;
		}
		if (mti->token.type!=MT_WORD) {
			xtypestr = MtTokenTypeToString(mti->token.type);
			MtFileWarning(mti,"Expected WORD after <, got %s",
				xtypestr);
			return 0;
		}
		sid = MtStringToSid(mti->token.s);
		if (mti->sscount>=mti->ssalloc) {
			if (!mti->ssalloc)
				mti->ssalloc = 12;
			else
				mti->ssalloc *= 2;
			sz = mti->ssalloc * sizeof(mti->ss[0]);
			mti->ss = MtRealloc(mti->ss,sz);
		}
		mti->ss[mti->sscount++] = sid;
		/* Now see if we have any args */
		if (!MtGetToken(mti)) {
			MtFileWarning(mti,
				"Expected token after <WORD, got EOF");
			return 0;
		}
		switch (mti->token.type) {
		case MT_LBRACKET:
			MtUngetToken(mti);
			/* push back the token */
			mti->cmd = MT_CMD_BEGIN;
			return 1;
		case MT_RBRACKET:	/* command with no args */
			mti->needpopcmd = 1;
			mti->cmd = MT_CMD_COMPLETE;
			return 1;
		}
		/* Not EOF, not begin or end bracket, must be args */
		while (mti->token.type) {
			if (mti->argcount>=mti->argalloc) {
				if (!mti->argalloc)
					mti->argalloc = 7;
				else
					mti->argalloc *= 2;
				sz = mti->argalloc * sizeof(mti->args[0]);
				mti->args = MtRealloc(mti->args,sz);
				for (i=mti->argcount; i<mti->argalloc; i++)
					mti->args[i] = zapArgToken;
			}
			MtCopyToken(&(mti->token),mti->args+mti->argcount);
			mti->argcount++;
			if (!MtGetToken(mti))
				break;	/* EOF */
			if (mti->token.type==MT_RBRACKET)
				break;
			if (mti->token.type==MT_LBRACKET) {
				MtFileWarning(mti,"Unexpected < after args");
				return 0;
			}
		}
		if (mti->token.type && mti->token.type!=MT_RBRACKET)
			MtUngetToken(mti);
		mti->needpopcmd = 1;
		mti->cmd = MT_CMD_COMPLETE;
		return 1;
	default:
		xtypestr = MtTokenTypeToString(mti->token.type);
		switch (mti->token.type) {
		case MT_WORD: case MT_STRING:
			s = mti->token.s;
			if (!s) s="<nil>";
			if (strlen(s)>sizeof(moreinfo)-5) {
				sprintf(moreinfo,"(%*.*s...) ",
					sizeof(moreinfo)-7,sizeof(moreinfo)-7,
					s);
			} else {
				sprintf(moreinfo,"(%s) ",s);
			}
			break;
		default:
			moreinfo[0] = 0;
			break;
		}
		MtFileWarning(mti,"Unexpected %s %sin input",xtypestr,moreinfo);
		return 0;
	}
	/* NOTREACHED */
}

/* Call this after a MT_CMD_BEGIN to skip up to and including the matching
 * MT_CMD_END */
void
MtSkipCmd(mti)
MtInfo *mti;
{
	int level;
	level = mti->sscount;
	MtGetCmd(mti);
	while (mti->sscount>level) {
		MtGetCmd(mti);
		if (mti->cmd==MT_CMD_END && mti->sscount<=level)
			break;
	}
}

void
MtPrintCmd(f,mti)
FILE *f;
MtInfo *mti;
{
	int i;

	switch (mti->cmd) {
	case MT_CMD_BEGIN:
		fprintf(f,"%d: BEGIN %s\n",
			mti->sscount, MtSidToString(mti->ss[mti->sscount-1]));
		break;
	case MT_CMD_END:
		fprintf(f,"%d: END %s\n",
			mti->sscount, MtSidToString(mti->ss[mti->sscount-1]));
		break;
	case MT_CMD_COMPLETE:
		fprintf(f,"%d: COMPLETE %s(%d) ",
			mti->sscount, MtSidToString(mti->ss[mti->sscount-1]),
			mti->argcount);
		for (i=0; i<mti->argcount; i++) {
			MtPrintToken(f,mti,mti->args+i);
			fputc(' ',f);
		}
		fputc('\n',f);
		break;
	default:
		fprintf(f,"%d: UNKNOWN cmd %d\n",mti->cmd);
		break;
	}
}

/* end */
