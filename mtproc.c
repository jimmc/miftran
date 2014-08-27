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
/* mtproc.c - top level parse/translate processing routines */

#include "mtutil.h"
#include "mttran.h"

/* Output substitution keys */
MtTypeSubNameTran MtTypeSubNameTranTab[] = {
#define MT_O_STRING 1
	{ MT_O_STRING, "string" },
#define MT_O_PGFNUMSTRING 2
	{ MT_O_PGFNUMSTRING, "pgfnumstring" },
#define MT_O_MARKERTEXT 10
	{ MT_O_MARKERTEXT, "markertext" },
#define MT_O_XREFTEXT 20
	{ MT_O_XREFTEXT, "xreftext" },
#define MT_O_XREFEND 21
	{ MT_O_XREFEND, "xrefend" },
#define MT_O_HYPERTEXT 22
	{ MT_O_HYPERTEXT, "hypertext" },
#define MT_O_CHAR 30
	{ MT_O_CHAR, "char" },
#define MT_O_TAB 31
	{ MT_O_TAB, "tab" },
#define MT_O_HARDRETURN 32
	{ MT_O_HARDRETURN, "hardreturn" },
#define MT_O_EMDASH 33
	{ MT_O_EMDASH, "emdash" },
#define MT_O_ENDASH 34
	{ MT_O_ENDASH, "endash" },
#define MT_O_HARDSPACE 35
	{ MT_O_HARDSPACE, "hardspace" },
#define MT_O_HARDHYPHEN 36
	{ MT_O_HARDHYPHEN, "hardhyphen" },
#define MT_O_STARTPGF 100
	{ MT_O_STARTPGF, "startpgf" },
#define MT_O_ENDPGF 101
	{ MT_O_ENDPGF, "endpgf" },
#define MT_O_SWITCHPGF 102
	{ MT_O_SWITCHPGF, "switchpgf" },
#define MT_O_TFTAG 103
	{ MT_O_TFTAG, "textflowtag" },
#define MT_O_STARTFONT 200
	{ MT_O_STARTFONT, "startfont" },
#define MT_O_ENDFONT 201
	{ MT_O_ENDFONT, "endfont" },
#define MT_O_STARTPSFONT 202
	{ MT_O_STARTPSFONT, "startpsfont" },
#define MT_O_STARTFONTWEIGHT 203
	{ MT_O_STARTFONTWEIGHT, "startfontweight" },
#define MT_O_STARTFONTANGLE 204
	{ MT_O_STARTFONTANGLE, "startfontangle" },
#define MT_O_STARTFILE 300
	{ MT_O_STARTFILE, "startfile" },
#define MT_O_ENDFILE 301
	{ MT_O_ENDFILE, "endfile" },
#define MT_O_AFRAMEID 400
	{ MT_O_AFRAMEID, "aframeid" },	/* during definition */
#define MT_O_AFRAMEFILE 401
	{ MT_O_AFRAMEFILE, "aframefile" },	/* during definition */
#define MT_O_AFRAME 402
	{ MT_O_AFRAME, "aframe" },	/* reference to an aframe */
#define MT_O_VARNAME 500
	{ MT_O_VARNAME, "varname" },
#define MT_O_VARDEF 501
	{ MT_O_VARDEF, "vardef" },
#define MT_O_VARREF 502
	{ MT_O_VARREF, "varref" },
#define MT_O_TBLID 600
	{ MT_O_TBLID, "tableid" },	/* during definition */
#define MT_O_TBLBEGIN 601
	{ MT_O_TBLBEGIN, "tablebegin" },/* during definition */
#define MT_O_TBLROW 602
	{ MT_O_TBLROW, "tablerow" },	/* during definition */
#define MT_O_TBLCELL 603
	{ MT_O_TBLCELL, "tablecell" },	/* during definition */
#define MT_O_TBLEND 604
	{ MT_O_TBLEND, "tableend" },	/* during definition */
#define MT_O_ATBL 605
	{ MT_O_ATBL, "atable" },	/* reference to an atbl */

	{ 0 }
	
};

/* Processing functions called from the translation tables below */
/* On error, the function shouls set mti->tranerror=1 and return */

int
MtProcSubStrPgf(mti,what,fromstr,data)
MtInfo *mti;
int what;       /* MT_O_* */
char *fromstr;  /* e.g. paragraph type */
char *data;     /* string data for search or output */
{
	int t;
	char *pgfstr;
	char buf[300];

	/* first see if there is a paragraph-specific version */
	pgfstr = MtSidToString(mti->pgftag);
	sprintf(buf,"%s.%s",pgfstr,fromstr);
	t = MtSubStr(mti,what,buf,data);
	if (!t) {
		/* Couldn't find "pgfname.fromstr", look for just "fromstr" */
		t = MtSubStr(mti,what,fromstr,data);
	}
	if (!t) {
		/* Couldn't find that either, look for "*.fromstr" */
		sprintf(buf,"*.%s",pgfstr,fromstr);
		t = MtSubStr(mti,what,buf,data);
	}
	return t;
}

static
void
CheckPgfStart(mti)
MtInfo *mti;
{
	if (mti->needpgfstart && mti->pgftag) {
		MtSubSid(mti,MT_O_STARTPGF,mti->pgftag,"");
		mti->needpgfstart = 0;
		mti->needpgfend = 1;
	}
}

static
void
CheckEndFont(mti)
MtInfo *mti;
{
	if (mti->infontanchor) {
		MtProcSubStrPgf(mti,MT_O_HYPERTEXT,"endanchor","");
		mti->infontanchor = 0;
	}
	if (mti->fonttag) {
		MtSubSid(mti,MT_O_ENDFONT,mti->fonttag,"");
		mti->fonttag = 0;
	}
}

void
MtProcString(mti)
MtInfo *mti;
{
	char *s;

	CheckPgfStart(mti);
	s = mti->args[0].s;
	MtSubSid(mti,MT_O_STRING,mti->pgftag,s);
}

void
MtProcPgfNumString(mti)
MtInfo *mti;
{
	char *s;

	CheckPgfStart(mti);
	s = mti->args[0].s;
	MtSubSid(mti,MT_O_PGFNUMSTRING,mti->pgftag,s);
}

void
MtProcHypertext(mti)
MtInfo *mti;
{
	char *s, *sp;

	s = mti->args[0].s;
	sp = strchr(s,' ');
	if (sp)
		*sp++ = 0;	/* null terminate cmd word, point to data */
	else
		sp = "";	/* no data word */
	/* s is command word, such as "newlink",
	 * sp is data word (the rest of the string) */
	MtProcSubStrPgf(mti,MT_O_HYPERTEXT,s,sp);
}

void
MtProcMarkerType(mti)
MtInfo *mti;
{
	mti->markertype = mti->args[0].i;
}

void
MtProcMarkerText(mti)
MtInfo *mti;
{
	char *s;
	char mnumstr[10];

	if (mti->markertype==8) {	/* marker type 8 is hypertext */
		MtProcHypertext(mti);
		return;
	}
	s = mti->args[0].s;
	sprintf(mnumstr,"%d",mti->markertype);
	MtProcSubStrPgf(mti,MT_O_MARKERTEXT,mnumstr,s);
}

void
MtProcXrefText(mti)
MtInfo *mti;
{
	char *s;

	s = mti->args[0].s;
	MtSubSid(mti,MT_O_XREFTEXT,(MtSid)0,s);
}

void
MtProcXrefEnd(mti)
MtInfo *mti;
{
	MtSubSid(mti,MT_O_XREFEND,(MtSid)0,"");
}

void
MtProcFontThing(mti,fontthing,conditional)
MtInfo *mti;
int fontthing;	/* MT_O_something */
int conditional;	/* 1 means only use this if no current font set */
{
	char *s;

	if (conditional && mti->fonttag!=0)
		return;	/* ignore it */
	CheckEndFont(mti);
	s = mti->args[0].s;
	if (s && *s) {
		CheckPgfStart(mti);
		mti->fonttag = MtStringToSid(s);
		MtSubSid(mti,fontthing,mti->fonttag,"");
	}
}

void
MtProcFTag(mti)
MtInfo *mti;
{
	MtProcFontThing(mti,MT_O_STARTFONT,0);
}

void
MtProcFPostScriptName(mti)
MtInfo *mti;
{
	MtProcFontThing(mti,MT_O_STARTPSFONT,1);
}


void
MtProcFWeight(mti)
MtInfo *mti;
{
	MtProcFontThing(mti,MT_O_STARTFONTWEIGHT,1);
}


void
MtProcFAngle(mti)
MtInfo *mti;
{
	MtProcFontThing(mti,MT_O_STARTFONTANGLE,1);
}

void
MtProcChar(mti)
MtInfo *mti;
{
	char *s;

	CheckPgfStart(mti);
	s = mti->args[0].s;
	MtMakeLower(s);
	if (strcmp(s,"tab")==0)
		MtSubSid(mti,MT_O_TAB,mti->pgftag,"");
	else if (strcmp(s,"hardreturn")==0)
		MtSubSid(mti,MT_O_HARDRETURN,mti->pgftag,"");
	else if (strcmp(s,"hardspace")==0)
		MtSubSid(mti,MT_O_HARDSPACE,mti->pgftag,"");
	else if (strcmp(s,"hardhyphen")==0)
		MtSubSid(mti,MT_O_HARDHYPHEN,mti->pgftag,"");
	else if (strcmp(s,"emdash")==0)
		MtSubSid(mti,MT_O_EMDASH,mti->pgftag,"");
	else if (strcmp(s,"endash")==0)
		MtSubSid(mti,MT_O_ENDASH,mti->pgftag,"");
	else
		MtSubSid(mti,MT_O_CHAR,mti->pgftag,s);
}

static
void
SwitchPgf(mti,from,to)
MtInfo *mti;
MtSid from, to;
{
	char *fromstr, *tostr;
	char pbuf[500];
	int t;

	fromstr = MtSidToString(from);
	tostr = MtSidToString(to);
	sprintf(pbuf,"%s.%s",fromstr,tostr);
	t = MtSubStr(mti,MT_O_SWITCHPGF,pbuf,"");
	if (!t && from!=to) {
		sprintf(pbuf,"%s.*",fromstr);
		MtSubStr(mti,MT_O_SWITCHPGF,pbuf,"");
		sprintf(pbuf,"*.%s",tostr);
		MtSubStr(mti,MT_O_SWITCHPGF,pbuf,"");
	}
}

void
MtProcPgfTag(mti)
MtInfo *mti;
{
	char *type;
	MtSid newpgftag;

	type = mti->args[0].s;
	newpgftag = MtStringToSid(type);
	SwitchPgf(mti,mti->pgftag,newpgftag);
	mti->pgftag = newpgftag;
	mti->needpgfstart = 1;
}

void
MtProcTFTag(mti)	/* Text Flow tag */
MtInfo *mti;
{
	char *type;

	type = mti->args[0].s;	/* the text flow tag */
	MtSubStr(mti,MT_O_TFTAG,type,"");
}

void
MtProcParaPre(mti)
MtInfo *mti;
{
	mti->needpgfstart = 1;
	mti->needpgfend = 0;
}

void
MtProcParaPost(mti)
MtInfo *mti;
{
	CheckEndFont(mti);
	if (mti->needpgfend) {
		MtSubSid(mti,MT_O_ENDPGF,mti->pgftag,"");
		mti->needpgfend = 0;
	}
}

void
MtProcAframeId(mti)
MtInfo *mti;
{
	char buf[30];

	sprintf(buf,"%d",mti->args[0].i);
	MtSubSid(mti,MT_O_AFRAMEID,(MtSid)0,buf);
}

void
MtProcImportObFile(mti)
MtInfo *mti;
{
	char *s;

	s = mti->args[0].s;
	MtSubSid(mti,MT_O_AFRAMEFILE,(MtSid)0,s);
}

void
MtProcAframe(mti)
MtInfo *mti;
{
	char buf[30];

	sprintf(buf,"%d",mti->args[0].i);
	MtSubSid(mti,MT_O_AFRAME,(MtSid)0,buf);
}

void
MtProcVariableName(mti)
MtInfo *mti;
{
	char *s;

	s = mti->args[0].s;
	MtSubSid(mti,MT_O_VARNAME,(MtSid)0,s);
}

void
MtProcVariableDef(mti)
MtInfo *mti;
{
	char *s;

	s = mti->args[0].s;
	MtSubSid(mti,MT_O_VARDEF,(MtSid)0,s);
}

void
MtProcVariableRef(mti)
MtInfo *mti;
{
	char *s;

	CheckPgfStart(mti);
	s = mti->args[0].s;
	MtSubSid(mti,MT_O_VARREF,mti->pgftag,s);
}

void
MtProcTblCell(mti)
MtInfo *mti;
{
	MtSubSid(mti,MT_O_TBLCELL,(MtSid)0,"");
}

void
MtProcTblRow(mti)
MtInfo *mti;
{
	MtSubSid(mti,MT_O_TBLROW,(MtSid)0,"");
}

void
MtProcTblBegin(mti)
MtInfo *mti;
{
	MtSubSid(mti,MT_O_TBLBEGIN,(MtSid)0,"");
}

void
MtProcTblEnd(mti)
MtInfo *mti;
{
	MtSubSid(mti,MT_O_TBLEND,(MtSid)0,"");
}

void
MtProcTblId(mti)
MtInfo *mti;
{
	char buf[30];

	sprintf(buf,"%d",mti->args[0].i);
	MtSubSid(mti,MT_O_TBLID,(MtSid)0,buf);
}

void
MtProcAtbl(mti)
MtInfo *mti;
{
	char buf[30];

	sprintf(buf,"%d",mti->args[0].i);
	MtSubSid(mti,MT_O_ATBL,(MtSid)0,buf);

}

/* Translation tables to drive calling the action functions above */

MtSidTran MarkerTranTab[] = {
	{ "MType", 0, MtProcMarkerType, 0, 0 },
	{ "MText", 0, MtProcMarkerText, 0, 0 },
	{ 0 }
};

MtSidTran XrefTranTab[] = {
	{ "XRefSrcText", 0, MtProcXrefText, 0, 0 },
	{ 0 }
};

MtSidTran FontTranTab[] = {
	{ "FTag", 0, MtProcFTag, 0, 0 },
	{ "FPostScriptName", 0, MtProcFPostScriptName, 0, 0 },
	{ "FWeight", 0, MtProcFWeight, 0, 0 },
	{ "FAngle", 0, MtProcFAngle, 0, 0 },
	{ 0 }
};

MtSidTran VarRefTranTab[] = {
	{ "VariableName", 0, MtProcVariableRef, 0, 0 },
	{ 0 }
};

MtSidTran ParaLineTranTab[] = {
	{ "String", 0, MtProcString, 0, 0 },
	{ "Char", 0, MtProcChar, 0, 0 },
	{ "Marker", 0, 0, 0, MarkerTranTab },
	{ "XRef", 0, 0, 0, XrefTranTab },
	{ "XRefEnd", 0, MtProcXrefEnd, 0, 0 },
	{ "Font", 0, 0, 0, FontTranTab },
	{ "AFrame", 0, 0, MtProcAframe, 0 },
	{ "Variable", 0, 0, 0, VarRefTranTab },
	{ "ATbl", 0, 0, MtProcAtbl, 0 },
	{ 0 }
};

MtSidTran ParaPgfTranTab[] = {
	{ "PgfTag", 0, MtProcPgfTag, 0, 0 },
	{ 0 }
};

MtSidTran ParaTranTab[] = {
	{ "Pgf", 0, 0, 0, ParaPgfTranTab },
	{ "PgfTag", 0, MtProcPgfTag, 0, 0 },
	{ "PgfNumString", 0, MtProcPgfNumString, 0, 0 },
	{ "ParaLine", 0, 0, 0, ParaLineTranTab },
	{ 0 }
};

MtSidTran TextFlowTranTab[] = {
	{ "Para", 0, MtProcParaPre, MtProcParaPost, ParaTranTab },
	{ "TFTag", 0, MtProcTFTag, 0, 0 },
	{ 0 }
};

MtSidTran ImportObjectTranTab[] = {
	{ "ImportObFile", 0, 0, MtProcImportObFile, 0 },
	{ 0 }
};

MtSidTran FrameTranTab[] = {
	{ "ID", 0, 0, MtProcAframeId, 0 },
	{ "ImportObject", 0, 0, 0, ImportObjectTranTab },
	{ 0 }
};

MtSidTran AFramesTranTab[] = {
	{ "Frame", 0, 0, 0, FrameTranTab },
	{ 0 }
};

MtSidTran TblCellContTab[] = {
	{ "Para", 0, MtProcParaPre, MtProcParaPost, ParaTranTab },
	{ 0 }
};

MtSidTran TblCellTab[] = {
	{ "CellContent", 0, 0, 0, TblCellContTab },
	{ 0 }
};

MtSidTran TblRowTab[] = {
	{ "Cell", 0, MtProcTblCell, 0, TblCellTab },
	{ 0 }
};

MtSidTran TblBodyTab[] = {
	{ "Row", 0, MtProcTblRow, 0, TblRowTab },
	{ 0 }
};

MtSidTran TblTranTab[] = {
	{ "TblID", 0, 0, MtProcTblId, 0 },
	{ "TblBody", 0, MtProcTblBegin, MtProcTblEnd, TblBodyTab },
	{ 0 }
};

MtSidTran TblsTranTab[] = {
	{ "Tbl", 0, 0, 0, TblTranTab },
	{ 0 }
};

MtSidTran VariableFormatTranTab[] = {
	{ "VariableName", 0, MtProcVariableName, 0, 0 },
	{ "VariableDef", 0, MtProcVariableDef, 0, 0 },
	{ 0 }
};

MtSidTran VariableFormatsTranTab[] = {
	{ "VariableFormat", 0, 0, 0, VariableFormatTranTab },
	{ 0 }
};

MtSidTran TopTranTab[] = {
	{ "TextFlow", 0, 0, 0, TextFlowTranTab },
	{ "AFrames", 0, 0, 0, AFramesTranTab },
	{ "VariableFormats", 0, 0, 0, VariableFormatsTranTab },
	{ "Tbls", 0, 0, 0, TblsTranTab },
	{ 0 }
};

int
MtProcTop(mti)
MtInfo *mti;
{
	int t;

	MtPrepareTranTab(TopTranTab);
	MtSubSid(mti,MT_O_STARTFILE,(MtSid)0,mti->ifi->ifilename);
	t =  MtTran(mti,TopTranTab);
	if (t) return t;
	if (mti->pgftag) {
		SwitchPgf(mti,mti->pgftag,0);
		mti->pgftag = 0;
	}
	MtSubSid(mti,MT_O_ENDFILE,(MtSid)0,mti->ifi->ifilename);
	return 0;
}

/* end */
