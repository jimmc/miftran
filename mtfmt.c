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
/* mtfmt.c - output formatting stage */

#include <ctype.h>
#include "mtutil.h"

extern char *getenv ARGS((const char *s1));
extern char *strrchr ARGS((const char *s1, const char c));

extern char *MtOutFileName;
extern int MtOutFileNum;

#define NUMREGISTERS 100	/* or use -DNUMREGISTER=nnn in Makefile */

int MtDoFmt;
char *MtStringRegisters[NUMREGISTERS] = {0};
int MtIntRegisters[NUMREGISTERS] = {0};

typedef struct _xcmdinfo {
	char *name;
	void (*func)();
} XCmdInfo;

void MtCmdSkip(mti)
MtInfo *mti;
{
	mti->skip = 1;	/* set skip flag */
}

XCmdInfo MtXCmdTab[] = {
	{ "skip", MtCmdSkip },
	{ 0 },
};

void
MtXCmd(mti,cmd)
MtInfo *mti;
char *cmd;
{
	int i;

	for (i=0; MtXCmdTab[i].name; i++) {
		if (strcmp(MtXCmdTab[i].name,cmd)==0) {
			(MtXCmdTab[i].func)(mti);
			return;
		}
	}
	return;
}

void
MtSubFmt(mti,fmt,data)
MtInfo *mti;
char *fmt;	/* the formatted string, including % escapes */
char *data;	/* data string */
{
	char *s, *p;
	int i, n;
	int num, num2;
	int posflag, negflag, relflag;
	int dotflag;
	int strflag;
	char *d;
	static char *str=0;
	static int stralloc=0;
#define INIT_STR_SIZE 1000

	if (!MtDoFmt) {
		/* show formatting before actually doing it */
		MtPrintf(mti,"\n[F:%s]\n",fmt);
	}
	if (stralloc==0) {
		stralloc = INIT_STR_SIZE;
		str = MtMalloc(stralloc);
	}
	for (p=fmt; *p; p++) {
		if (*p!='%') {
			MtPutc(mti,*p);	/* regular char, output it */
			continue;	/*  and look for next char */
		}
		/* Here to process % sequences */
		p++;
		num = 0;
		num2 = 0;
		if (*p=='+') {
			posflag=1;
			p++;
		} else
			posflag=0;
		if (*p=='-') {
			negflag=1;
			p++;
		} else
			negflag=0;
		relflag = posflag||negflag;
		while (isdigit(*p)) {
			num = num*10 + (*p)-'0';
			p++;
		}
		if (*p=='.') {
			dotflag=1;
			p++;
			while (isdigit(*p)) {
				num2 = num2*10 + (*p)-'0';
				p++;
			}
		} else
			dotflag=0;
		if (negflag)
			num = -num;
		if (*p=='"') {
			strflag=1;
			d = str;
			p++;
			while (*p) {
				if (d-str>stralloc-4) {
					stralloc *= 2;
					str = MtRealloc(str,stralloc);
				}
				if (*p=='"') {
					p++;
					break;
				}
				if (*p=='\\') {
					if (!*++p)
						break;
					*d++ = '\\';
					/* continue, to copy char after \ */
				}
				*d++ = *p++;
			}
			*d = 0;
			MtUnBackslash(str);	/* compress in place */
		} else {
			str[0] = 0;
			strflag=0;
		}
		/* Upper case chars have side effects;
		 * lower case chars just print things out. */
		switch(*p) {
		case '\0':	/* % at end of string, ignore it */
			p--;	/* adjust for p++ in for loop */
			break;
		case '%':
			MtPutc(mti,'%');
			break;
		case 'A':	/* set alternate output file */
			if (p[-1]=='%') relflag=1;
				/* make %A same as %+0A */
			if (MtDoFmt)
				MtSetOutputFile(mti,num,num2,relflag,1);
			else
				MtPrintf(mti,"[SetOutput:%d,%d,%d,1]\n",
					num,num2,relflag);
			break;
		case 'E':	/* store env val into register */
			if (num<=0 || num>=NUMREGISTERS)
				break;	/* ignore if out of range */
			str = getenv(str);
			if (!str) str="";
			if (posflag) {
				if (*str)
					n = atoi(str);
				else
					n = 0;
				MtIntRegisters[num] = n;
				if (!MtDoFmt)
					MtPrintf(mti,"[IntReg(%d)=%d]",num,n);
			} else {
				if (MtStringRegisters[num])
					MtFree(MtStringRegisters[num]);
				MtStringRegisters[num] = MtStrSave(str);
				if (!MtDoFmt)
					MtPrintf("[StrReg(%d)=%s]",num,str);
			}
			break;
		case 'F':	/* set main output file */
			if (p[-1]=='%') {
				int altflag = (mti->prevodest<0);
				/* %F means switch back to previous file */
				if (mti->prevodest>0 && MtDoFmt) {
					mti->odest = mti->prevodest;
					break;
				}
				if (MtDoFmt)
					MtSetOutputFile(mti,0,0,1,altflag);
						/* like %+0F or %+0A */
				else
					MtPrintf(mti,"[SetOutput:0,0,1,%d]",
						altflag);
				break;
			}
			if (MtDoFmt)
				MtSetOutputFile(mti,num,num2,relflag,0);
			else
				MtPrintf(mti,"[SetOutput:%d,%d,%d,0]",
					num,num2,relflag);
			break;
		case 'H':
			mti->infontanchor = 1;
			break;
		case 'L':	/* store literal data into register */
			if (num<=0 || num>=NUMREGISTERS)
				break;	/* ignore if out of range */
			if (posflag) {
				MtIntRegisters[num] = num2;
				if (!MtDoFmt)
				    MtPrintf(mti,"[IntReg(%d)=%d]",num,num2);
			} else {
				if (MtStringRegisters[num])
					MtFree(MtStringRegisters[num]);
				MtStringRegisters[num] = MtStrSave(str);
				if (!MtDoFmt)
				    MtPrintf(mti,"[StrReg(%d)=%d]",num,str);
			}
			break;
		case 'O':	/* dual-register operation */
			if (num<=0 || num>=NUMREGISTERS)
				break;	/* ignore if N1 out of range */
			if (num2<=0 || num2>=NUMREGISTERS)
				break;	/* ignore if N2 out of range */
			if (posflag) {
				n = MtOpInt(str,MtIntRegisters[num],
					MtIntRegisters[num2]);
				MtIntRegisters[num] = n;
				if (!MtDoFmt)
				    MtPrintf(mti,"[IntReg(%d)=%d]",num,n);
			} else {
				extern char *MtOpStr();
				s = MtOpStr(str,MtStringRegisters[num],
					MtStringRegisters[num2]);
				if (s!=MtStringRegisters[num]) {
					if (MtStringRegisters[num])
						MtFree(MtStringRegisters[num]);
					MtStringRegisters[num] = s;
					if (!MtDoFmt)
					    MtPrintf(mti,"[StrReg(%d)=%s]",
							num,s);
				}
			}
			break;
		case 'R':	/* store data into register */
			if (num<=0 || num>=NUMREGISTERS)
				break;	/* ignore if out of range */
			if (posflag) {
				/* + means use int registers */
				if (num2<0)
					break;	/* reserved for future use */
				s = data;
				while (*s) {
					while (*s && !isdigit(*s)) s++;
					if (!*s) break;
					i = 0;
					while (isdigit(*s)) {
						i = i*10 + *s-'0';
						s++;
					}
					if (num2==0) {
						MtIntRegisters[num] = i;
						if (!MtDoFmt)
						    MtPrintf(mti,
							"[IntReg(%d)=%d]",
							num,i);
					}
					if (--num2<0)
						break;
				}
				break;
			} else {
				if (MtStringRegisters[num])
					MtFree(MtStringRegisters[num]);
				MtStringRegisters[num] = MtStrSave(data);
				if (!MtDoFmt)
				    MtPrintf(mti,"[StrReg(%d)=%s]",num,data);
			}
			break;
		case 'S':	/* set output stream to point to string reg N */
			if (num<=0 || num>=NUMREGISTERS)
				break;	/* ignore if out of range */
			if (MtDoFmt) {
				mti->prevodest = mti->odest;
				mti->odest = num;
			} else {
				MtPrintf(mti,"[SetOutput:StrReg(%d)]",num);
			}
			break;
		case 'X':	/* extended command */
			MtXCmd(mti,str);	/* do the extended command */
			break;
		case 'f':	/* print main outfilename */
			if (p[-1]=='%') relflag=1;
				/* make %f same as %+0f */
			/* num arg is used in printed filename */
			if (!MtOutFileName)
				break;
			if (relflag)
				num += MtOutFileNum;
			s = strrchr(MtOutFileName,'/');
			if (s) s++;
			else s=MtOutFileName;
			MtPrintf(mti,s,num);
			break;
		case 'n':	/* a hack: insert a newline if the last char
				 * of the data is a space. */
			if (data && *data && data[strlen(data)-1]==' ')
				MtPuts(mti,"\n");
			break;
		case 'r':	/* retrieve data from register */
			if (num<=0 || num>=NUMREGISTERS)
				break;	/* ignore if out of range */
			if (posflag) {
				/* use int registers */
				MtPrintf(mti,"%d",MtIntRegisters[num]);
			} else {
				if (MtStringRegisters[num])
					MtPuts(mti,MtStringRegisters[num]);
			}
			break;
		case 's':	/* insert the current data string */
			MtPuts(mti,data);
			break;
		default:
			MtPutc(mti,*p);
			break;
		}
	}
}

/* end */
