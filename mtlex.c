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
/* mtlex - parse input stream into tokens */

#include "mtutil.h"
#include "mtlex.h"

/* reads the next character from the input and returns it */
int		/* next char, or EOF */
MtGetChar(mti)
MtInfo *mti;
{
	int c;
	FILE *f;

	if (mti->pushedchar) {
		c = mti->pushedchar;
		mti->pushedchar = 0;
		return c;
	}
	if (mti->eof) return EOF;
	f = mti->ifp;
	if (feof(f) || (c=fgetc(f))==EOF) {
		mti->eof = 1;
		return EOF;
	}
	if (c=='\n')
		mti->lineno++;
	return c;
}

static
void
MtGetNumber(mti)
MtInfo *mti;
{
	int c, n;
	int t, div;
	double d;
	int negflag;

	n = 0;
	t = MT_INT;		/* assume int */
	c = MtGetChar(mti);
	if (c=='-') {
		negflag = 1;
		c = MtGetChar(mti);
	} else
		negflag = 0;
	while (isdigit(c)) {
		n *= 10;
		n += c-'0';
		c = MtGetChar(mti);
	}
	if (c=='.') {
		t = MT_DOUBLE;	/* switch types */
		d = (double)n;
		div = 1;	/* divisor for fractional part */
		n = 0;
		c = MtGetChar(mti);
		while (isdigit(c)) {
			div *= 10;
			n *= 10;
			n += c-'0';
			c = MtGetChar(mti);
		}
		d += ((double)n/(double)div);
	}
	/* A number may be followed by a double quote to mean inches,
	 * or by a space and pt to mean points.  We ignore these details
	 * for now: we eat the double quote, and let the pt through
	 * as a separate word */
	if (c!='"') {
		mti->pushedchar = c;
	}
	if (negflag) {
		n = -n;
		d = -d;
	}
	mti->token.type = t;
	if (t==MT_INT)
		mti->token.i = n;
	else
		mti->token.d = d;
}

static
void
MtGetStrPutChar(mti,c,pp,pn)
MtInfo *mti;
int c;
char **pp;	/* pointer to buffer pointer */
int *pn;	/* pointer to count */
{
	int n;
	char *p;

	p = *pp;
	n = *pn;

	*p++ = c;
	if (++n>=mti->token.alloc) {
		mti->token.alloc *= 2;
		mti->token.s = MtRealloc(
			mti->token.s,mti->token.alloc);
		p = mti->token.s + n;
	}
	*pn = n;
	*pp = p;
}

/* Set type into mti->token.type, then call this function to suck in the
 * rest of the input string. */
static
void
MtGetString(mti)
MtInfo *mti;
{
	int c, n;
	char *p;
	int t;
	int quotedquote;

	n = 0;
	t = (mti->token.type==MT_WORD);
	if (mti->token.alloc==0) {
		mti->token.alloc = 240;
		mti->token.s = MtMalloc(mti->token.alloc);
	}
	p = mti->token.s;
	while (!mti->eof) {
		c = MtGetChar(mti);
		if (c==EOF)
			break;	/* end of file */
		if (t && !(isalnum(c)||c=='_')) {
			mti->pushedchar = c;
			break;	/* end of word */
		}
		if (c=='\\') {
			c = MtGetChar(mti);
			if (c==EOF) break;
			if (c=='\'')
				MtGetStrPutChar(mti,c,&p,&n);
			else if (c=='\n')
				; /* line continuation, eat the bs-nl */
			else {
				MtGetStrPutChar(mti,'\\',&p,&n);
				MtGetStrPutChar(mti,c,&p,&n);
			}
		} else if (c=='\'')
			break;	/* end of quoted string */
		else
			MtGetStrPutChar(mti,c,&p,&n);	/* normal char */
	}
	*p = 0;
}

/* Copy data from one token to another.  Assumes that dest token is valid,
 * in particular will use alloc and s in dest token. */
void
MtCopyToken(src,dest)
MtToken *src;
MtToken *dest;
{
	int n;

	dest->type = src->type;
	switch(src->type) {
	case MT_WORD:
	case MT_STRING:
		/* copy string from src to dest */
		n = strlen(src->s);
		if (n+1>dest->alloc) {
			dest->alloc = n+1;
			dest->s = MtRealloc(dest->s,dest->alloc);
		}
		(void)strcpy(dest->s,src->s);
		break;
	case MT_INT:
		dest->i = src->i;
		break;
	case MT_DOUBLE:
		dest->d = src->d;
		break;
	default:
		break;	/* for everything else, type is enough */
	}
}

void
MtUngetToken(mti)
MtInfo *mti;
{
	MtToken tk;

	if (mti->pushedtoken.type) {
		MtWarning(mti,"Attempt to push more than one token");
		return;
	}
	tk = mti->pushedtoken;
		/* Save pushedtoken info in case of buffer (alloc, s) */
	mti->pushedtoken = mti->token;
	mti->token = tk;
	mti->token.type = 0;
}

/* reads next token from input stream and returns it */
int		/* 1 if got a token, 0 if no more */
MtGetToken(mti)
MtInfo *mti;
{
	int c, n;
	char *p;
	MtToken tk;

	if (mti->pushedtoken.type) {
		tk = mti->token;
		mti->token = mti->pushedtoken;
		mti->pushedtoken = tk;
		mti->pushedtoken.type = 0;
		return 1;
	}
	c = MtGetChar(mti);
	while (!mti->eof) {
		switch (c) {
		case EOF:
			mti->token.type = EOF;
			return 0;
		case '<':
			mti->token.type = MT_LBRACKET;
			return 1;
		case '>':
			mti->token.type = MT_RBRACKET;
			return 1;
		case '#':
			while (!mti->eof && c!='\n')
				c = MtGetChar(mti);	/* eat comment */
			continue;	/* with next char already in c */
		case '`':
			mti->token.type = MT_STRING;
			MtGetString(mti);
			return 1;
		default:
			break;	/* anything else, process below */
		}
		if (isspace(c)) {
			while (isspace(c))
				c = MtGetChar(mti);	/* eat whitespace */
			continue;	/* with next char already in c */
		}
		if (isdigit(c) || c=='-') {
			mti->pushedchar = c;
			MtGetNumber(mti);
			return 1;
		}
		if (isalpha(c)||c=='_') {
			mti->token.type = MT_WORD;
			mti->pushedchar = c;
			MtGetString(mti);
			return 1;
		}
		MtFileWarning(mti,"Unrecognized input character %c (%03o)",c,c);
		c = MtGetChar(mti);
	}
	mti->token.type = EOF;
	return 0;
}

void
MtPrintToken(f,mti,token)
FILE *f;
MtInfo *mti;
MtToken *token;
{
	switch (token->type) {
	case EOF:
		fprintf(f,"EOF");
		break;
	case MT_LBRACKET:
		fprintf(f,"<");
		break;
	case MT_RBRACKET:
		fprintf(f,">");
		break;
	case MT_WORD:
		fprintf(f,"WORD %s",token->s);
		break;
	case MT_STRING:
		fprintf(f,"STRING \"%s\"",token->s);
		break;
	case MT_INT:
		fprintf(f,"INT %d",token->i);
		break;
	case MT_DOUBLE:
		fprintf(f,"DOUBLE %g",token->d);
		break;
	default:
		fprintf(f,"Unknown token type %d",token->type);
		break;
	}
}

/* end */
