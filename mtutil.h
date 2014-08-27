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
/* mtutil.h - external interface to mtutil.c functions */

#ifdef __STDC__
#define ARGS(args)args
#else
#define ARGS(args)()
#endif

#include "mtinfo.h"

#ifndef __GNUC__
/* gcc gives bogus errors about argument count mismatches on these two. */
void MtWarning ARGS((MtInfo *mti, char *fmt, ...));
void MtFileWarning ARGS((MtInfo *mti, char *fmt, ...));
#endif

char *MtTokenTypeToString ARGS((int tokentype));

void *MtMalloc ARGS((unsigned int n));
void *MtRealloc ARGS((void *oldp, unsigned int n));

MtInfo *MtNewInfo ARGS((void));
void MtSetInputFile ARGS((MtInfo *mti, FILE *f, char *filename));

MtSid MtStringToSid ARGS((char *s));
char *MtSidToString ARGS((MtSid sid));

char *MtStrSave ARGS((char *s));
void MtMakeLower ARGS((char *s));

char *MtSymGet ARGS((char *name));
void  MtSymSet ARGS((char *name, char *value));

/* end */
