#Makefile to create HTML files using miftran

MIF_SOURCE      = ../miftran.mif
MIFTRAN_RC      = miftran.rc
MIFTRAN_DIR     = ..
MIFTRAN         = $(MIFTRAN_DIR)/miftran
MIFTRAN_ARGS    = -I $(MIFTRAN_DIR)/mtinc -rc $(MIFTRAN_RC) $(MIF_SOURCE)
FIXCHL          = $(MIFTRAN_DIR)/fixchl
FIXREF          = $(MIFTRAN_DIR)/fixref
FIXINDEX        = $(MIFTRAN_DIR)/fixindex
SHELL           = /bin/sh

default:	chap1.html

chap1.html:	$(MIFTRAN_RC) $(MIF_SOURCE)
	$(MIFTRAN) $(MIFTRAN_ARGS)
	$(FIXREF)
	[ -s chap0.html ] || rm -f chap0.html    #rm if zero size
	$(FIXCHL)
	mv d1.txt TOC.html
	$(FIXINDEX) <d3.txt >IX.html

pgf.list:	$(MIF_SOURCE)
	$(MIFTRAN) -tran $(MIFTRAN_ARGS) \
		| grep "^startpgf" | sort | uniq > pgf.list

font.list:	$(MIF_SOURCE)
	$(MIFTRAN) -tran $(MIFTRAN_ARGS) \
		| grep "^startfont" | sort | uniq > font.list

diff:;	for i in *.html; do echo $$i; diff $$i ../htmlref/$$i; done

clean:;	rm -f chap?.html TOC.html IX.html d?.txt href.sed pgf.list font.list
