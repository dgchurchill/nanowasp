INCLUDES = -DPKGDATADIR=\"@datadir@/@PACKAGE@\"

EXTRA_DIST = nanowasp.h crtc.h fdc.h gui.h keyb.h memmap.h mz80.h  vdu.h z80.h \
             disk.h
DISTCLEANFILES = mz80.asm

bin_PROGRAMS = nanowasp
noinst_PROGRAMS = makez80
nanowasp_SOURCES = nanowasp.c crtc.c fdc.c gui.c keyb.c memmap.c vdu.c z80.c \
                   disk.c
nanowasp_LDADD = mz80.o
SUFFIXES = .asm

mz80.asm: makez80
	./makez80 mz80.asm -l -s


%.o: %.asm
	$(NASM) -f elf $*.asm
