#
# Makefile for the OptiPNG manual page
#


# System directories
prefix=/usr/local
mandir=${prefix}/man
man1dir=${mandir}/man1


# Targets
all: optipng.txt optipng.ps optipng.html

optipng.txt: optipng.1
	nroff -man -c $< | col -bx | uniq > $@

optipng.ps: optipng.1
	#troff -man $< > $@
	groff -man -Tps $< > $@

optipng.html: optipng.1
	groff -man -Thtml $< > $@

install: optipng.1
	-@if [ ! -d ${man1dir} ]; then mkdir -p ${man1dir}; fi
	-@rm -f ${man1dir}/optipng.1
	cp -p optipng.1 ${man1dir}

uninstall:
	rm -f ${man1dir}/optipng.1

clean:
	#rm -f optipng.txt
	rm -f optipng.ps optipng.html
