
BASEI  = include/sfq.h

BASEL  = lib/libsfq.so lib/libsfq.so.0 lib/libsfq.so.0.17.2
BASEL += lib/libsfqc.so lib/libsfqc.so.0 lib/libsfqc.so.0.17.2

BASEX  = bin/sfqc-init bin/sfqc-info bin/sfqc-list bin/sfqc-pusht bin/sfqc-pushb 
BASEX += bin/sfqc-pop bin/sfqc-shift bin/sfqc-clear bin/sfqc-sets bin/sfqc-disable
BASEX += bin/sfqc-pushx bin/sfqc-popx bin/sfqc-shiftx bin/sfqc-reset-procs
BASEX += bin/sfqc-read

TARGET_BASE = $(BASEI) $(BASEL) $(BASEX)
TARGET_EXTN = $(EXTNL)


all: $(TARGET_BASE)


include/sfq.h: src/inc/sfq.h
	cp -p $^ include

#
lib/libsfq.so: src/libsfq/libsfq.so
	rm -f $@
	cp -P -p $^ $@

lib/libsfq.so.0: src/libsfq/libsfq.so.0
	rm -f $@
	cp -P -p $^ $@

lib/libsfq.so.0.17.2: src/libsfq/libsfq.so.0.17.2
	rm -f $@
	cp -P -p $^ $@

#
lib/libsfqc.so: src/sfqc/libsfqc.so
	rm -f $@
	cp -P -p $^ $@

lib/libsfqc.so.0: src/sfqc/libsfqc.so.0
	rm -f $@
	cp -P -p $^ $@

lib/libsfqc.so.0.17.2: src/sfqc/libsfqc.so.0.17.2
	rm -f $@
	cp -P -p $^ $@

#
# bin
#
src/libsfq/libsfq.so:
	cd src/libsfq/; make $(MAKEOPT); cp -p libsfq.so ../../lib/libsfq.so

src/sfqc/libsfqc.so:
	cd src/sfqc/; make $(MAKEOPT); cp -p libsfqc.so ../../lib/libsfqc.so

bin/sfqc-init: src/sfqc/sfqc-init
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-init ../../bin/sfqc-init

bin/sfqc-info: src/sfqc/sfqc-info
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-info ../../bin/sfqc-info

bin/sfqc-list: src/sfqc/sfqc-list
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-list ../../bin/sfqc-list

bin/sfqc-disable: src/sfqc/sfqc-disable
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-disable ../../bin/sfqc-disable

bin/sfqc-clear: src/sfqc/sfqc-clear
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-clear ../../bin/sfqc-clear

bin/sfqc-reset-procs: src/sfqc/sfqc-reset-procs
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-reset-procs ../../bin/sfqc-reset-procs

bin/sfqc-sets: src/sfqc/sfqc-sets
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-sets ../../bin/sfqc-sets

#
bin/sfqc-pusht: src/sfqc/sfqc-pusht
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-pusht ../../bin/sfqc-pusht

bin/sfqc-pushb: src/sfqc/sfqc-pushb
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-pushb ../../bin/sfqc-pushb

bin/sfqc-pop: src/sfqc/sfqc-pop
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-pop ../../bin/sfqc-pop

bin/sfqc-shift: src/sfqc/sfqc-shift
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-shift ../../bin/sfqc-shift

#
bin/sfqc-pushx: src/sfqc/sfqc-pushx
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-pushx ../../bin/sfqc-pushx

bin/sfqc-popx: src/sfqc/sfqc-popx
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-popx ../../bin/sfqc-popx

bin/sfqc-shiftx: src/sfqc/sfqc-shiftx
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-shiftx ../../bin/sfqc-shiftx

bin/sfqc-read: src/sfqc/sfqc-read
	cd src/sfqc/; make $(MAKEOPT); cp -p sfqc-read ../../bin/sfqc-read

#
debug:
	make clean
	make "MAKEOPT=debug"

clean:
	rm -f $(TARGET_BASE) $(TARGET_EXTN)
	cd src/sfqc/; make clean
	cd src/libsfq/; make clean

rebuild:
	make clean
	make

#

