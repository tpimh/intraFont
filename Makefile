PSPSDK = $(shell psp-config --pspsdk-path)
PSPDIR = $(shell psp-config --psp-prefix)

CFLAGS = -O2 -G0 -ffast-math -fomit-frame-pointer -Wall -I include 

OBJS = src/intraFont.o src/libccc.o
TARGET_LIB = lib/libintrafont.a

include $(PSPSDK)/lib/build.mak

release: $(TARGET_LIB)
	tar -zcvf libintraFont.tar.gz lib include

install: all
	mkdir -p $(PSPDIR)/include $(PSPDIR)/lib
	cp include/*.h $(PSPDIR)/include
	cp lib/*.a $(PSPDIR)/lib
