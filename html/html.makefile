#Makefile to create HTML files from miftran.mif

MIFTRAN = ../miftran
FIXCHL = ../fixchl
FIXREF = ../fixref
FIXINDEX = ../fixindex

BODY = ../miftran.mif

default:	chap1.html

chap1.html:	miftran.rc $(BODY)
	$(MIFTRAN)
	$(FIXREF)
	$(FIXCHL)
	mv d1.txt TOC.html
	$(FIXINDEX) <d3.txt >IX.html

diff:;	for i in *.html; do echo $$i; diff $$i ../htmlref/$$i; done

clean:;	rm -f chap?.html TOC.html IX.html d?.txt href.sed
