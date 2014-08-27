#Makefile for miftran
#Copyright 1993-1997 Globetrotter Software; see source files for full copyright.
#Jim McBeath (jimmc@globes.com)

PROGRAM = miftran
ROOTDESTDIR = /usr/local
BINDESTDIR = $(ROOTDESTDIR)/bin
LIBDESTDIR = $(ROOTDESTDIR)/lib
#DESTMACHINE is used in the remote_install tag to copy to another machine
DESTMACHINE = remotemachine

#name of the directory we are in, for use by 'make kit'
DIR = miftran

CP = cp

CDEFINES =

CCOPTS = -g

CFLAGS = $(INCLUDES) $(CDEFINES) $(CCOPTS) $(ARCH)
#For NextStep, which has multi-architecture binary files, you can uncomment
#the following line to compile for multiple architectures.
#ARCH = -arch m68k -arch i386 -arch hppa

LINKER = $(CC)
LINKFLAGS = $(CFLAGS)

LIBS =

KITFILES = README RelNotes Makefile TODO $(SRCS) *.h miftran.mif $(SCRIPTS) \
	html/Makefile html/html.makefile html/miftran.rc htmlref/*.htm \
	mtinc/*.rc

SCRIPTS  = fixchl fixindex fixref

CIFILES = $(KITFILES)

SRCS =	main.c \
	mtcmd.c \
	mtfmt.c \
	mtlex.c \
	mtop.c \
	mtout.c \
	mtproc.c \
	mtrc.c \
	mtsub.c \
	mttran.c \
	mtutil.c

OBJS =	main.o  \
	mtcmd.o \
	mtfmt.o \
	mtlex.o \
	mtop.o \
	mtout.o \
	mtproc.o \
	mtrc.o \
	mtsub.o \
	mttran.o \
	mtutil.o

default:	all

all:	prog

prog: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(LINKER) $(LINKFLAGS) -o $(PROGRAM) $(OBJS) $(LIBS)

install:	install_bin install_mtinc

install_bin:	prog
	$(CP) -p $(PROGRAM) $(BINDESTDIR)/$(PROGRAM)

install_mtinc:
	-mkdir $(LIBDESTDIR)/mtinc
	$(CP) -p mtinc/*.rc $(LIBDESTDIR)/mtinc/

remote_install: remote_install_bin remote_install_mtinc

remote_install_bin:
	$(MAKE) -e install_bin CP=rcp BINDESTDIR=$(DESTMACHINE):$(BINDESTDIR)

remote_install_mtinc:
	-rsh $(DESTMACHINE) mkdir $(LIBDESTDIR)/mtinc
	rcp -p mtinc/*.rc $(DESTMACHINE):$(LIBDESTDIR)/mtinc/

purify:;	make prog LINKER='purify $(LINKER)'

depend:;	makedepend $(SRCS)

refhtml:	htmlref/chap1.htm

htmlref/chap1.htm:	miftran.mif
	cd html; make
	cp -p html/*.htm htmlref/

kit:	$(DIR).tgz

$(DIR).tar:	$(KITFILES) refhtml
	rm -f kit.list
	for f in $(KITFILES); do echo $(DIR)/$$f >> kit.list; done
	cd ..; tar cvf $(DIR)/$(DIR).tar `cat $(DIR)/kit.list`
	ls -l $(DIR).tar

$(DIR).tgz:	$(DIR).tar
	gzip -9 $(DIR).tar
	mv $(DIR).tar.gz $(DIR).tgz
	ls -l $(DIR).tgz

lookcheck:;	ls -l $(CIFILES) | grep '^-rw' | cat

clean:;	rm -f $(PROGRAM) $(OBJS) core $(DIR).tar $(DIR).tar.gz $(DIR).tgz

#If you have makedepend (part of the X11 distribution) and you want
#dependencies on system files, you can run it to generate them below.
###
# DO NOT DELETE THIS LINE -- make depend depends on it.

main.o: mtutil.h mtinfo.h mtlex.h mtcmd.h version.h
mtcmd.o: mtutil.h mtinfo.h mtlex.h mtcmd.h
mtfmt.o: mtutil.h mtinfo.h 
mtlex.o: mtutil.h mtinfo.h mtlex.h
mtout.o: mtutil.h mtinfo.h 
mtproc.o: mtutil.h mtinfo.h mttran.h
mtrc.o: mtutil.h mtinfo.h mttran.h
mtsub.o: mtutil.h mtinfo.h 
mttran.o: mtutil.h mtinfo.h mtcmd.h mttran.h
mtutil.o: mtinfo.h
