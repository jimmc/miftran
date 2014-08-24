#Makefile to create HTML files using miftran

MIF_SOURCE      = ../miftran.mif
MIFTRAN_DIR     = ..

MIFTRAN         = $(MIFTRAN_DIR)/miftran
FIXCHL          = $(MIFTRAN_DIR)/fixchl
FIXREF          = $(MIFTRAN_DIR)/fixref
FIXINDEX        = $(MIFTRAN_DIR)/fixindex


default:	chap1.html

chap1.html:	miftran.rc $(MIF_SOURCE)
	$(MIFTRAN)
	$(FIXREF)
	$(FIXCHL)
	mv d1.txt TOC.html
	$(FIXINDEX) <d3.txt >IX.html

pgf.list:	$(MIF_SOURCE)
	$(MIFTRAN) -tran | grep "^startpgf" | sort | uniq > pgf.list

font.list:	$(MIF_SOURCE)
	$(MIFTRAN) -tran | grep "^startfont" | sort | uniq > font.list

diff:;	for i in *.html; do echo $$i; diff $$i ../htmlref/$$i; done

clean:;	rm -f chap?.html TOC.html IX.html d?.txt href.sed pgf.list font.list
