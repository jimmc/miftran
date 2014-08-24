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
/* mttran.c - generic translation processing */

#include "mtutil.h"
#include "mtcmd.h"
#include "mttran.h"

void
MtPrepareTranTab(trantab)
MtSidTran *trantab;
{
	MtSidTran *tran;

	if (trantab->sid) return;	/* already prepared */
	for (tran=trantab; tran->name; tran++) {
		tran->sid = MtStringToSid(tran->name);
		if (tran->trantab)
			MtPrepareTranTab(tran->trantab);
	}
}

int	/* 0 if all OK, else <>0 if a called function returns <>0 */
MtTran(mti,trantab)
MtInfo *mti;
MtSidTran *trantab;
{
	MtSid sid;
	int t;
	void (*p)();
	MtSidTran *tran;

	mti->tranerror = 0;
	while (MtGetCmd(mti)) {
		if (mti->cmd==MT_CMD_END) {
			mti->skip = 0;
			return 0;	/* done at this level */
		}
		if (mti->cmd!=MT_CMD_BEGIN && mti->cmd!=MT_CMD_COMPLETE) {
			MtFileWarning(mti,"Bad cmd");
			mti->tranerror = 1;
			mti->skip = 0;
			return 1;	/* Should never get here */
		}
		if (mti->skip) {
			MtSkipCmd(mti);	/* skip this command */
			continue;	/* skip until end of this level */
		}
		sid = mti->ss[mti->sscount-1];
		for (tran=trantab; tran->sid; tran++)
			if (sid==tran->sid)
				break;
		if (tran->sid) {
			/* found it */
			p = (tran->prefuncp);
			if (p) {
				(*p)(mti);
				if (mti->tranerror)
					return mti->tranerror;
			}
			if (mti->cmd==MT_CMD_BEGIN && tran->trantab) {
				t = MtTran(mti,tran->trantab);
				if (t) return t;
			}
			p = (tran->postfuncp);
			if (p) {
				(*p)(mti);
				if (mti->tranerror)
					return mti->tranerror;
			}
		} else if (mti->cmd==MT_CMD_BEGIN) {
			/* Not found in translation table, ignore it */
			MtSkipCmd(mti);	/* skip this command */
		}
	}
	return 0;
}

/* end */
