##
## Makefile for the OptiPNG manual page
##


# System directories
prefix=/usr/local
mandir=${prefix}/man
man1dir=${mandir}/man1


# Targets
all: optipng.1.txt optipng.1.ps optipng.1.pdf optipng.1.html

optipng.1.txt: optipng.1
	nroff -man -c $< | col -b -x | uniq > $@

optipng.1.ps: optipng.1
	#troff -man $< > $@
	groff -man -Tps -P-g $< > $@

optipng.1.pdf: optipng.1.ps
	ps2pdf $< $@

optipng.1.html: optipng.1
	#man2html $< > $@
	groff -man -Thtml -P-l $< > $@

install: optipng.1
	mkdir -p ${man1dir}
	cp -f -p optipng.1 ${man1dir}

uninstall:
	rm -f ${man1dir}/optipng.1

dist: optipng.1.txt optipng.1.pdf optipng.1.html
	cp -f -p optipng.1.txt ../doc/manual.txt
	cp -f -p optipng.1.pdf ../doc/manual.pdf
	cp -f -p optipng.1.html ../doc/manual.html

clean:
	rm -f optipng.1.txt
	rm -f optipng.1.ps
	rm -f optipng.1.pdf
	rm -f optipng.1.html

distclean: dist clean

