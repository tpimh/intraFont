TARGET = intraFontTest
OBJS = sinus.o\
       ../libraries/graphics.o ../libraries/framebuffer.o \

INCDIR = ../../include
CFLAGS = -O2 -G0 -Wall -g
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = ../../lib
LDFLAGS = 
LIBS = -lintrafont -lpspgum -lpspgu -lpng -lz -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = intraFont-G Sinusoid Test

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

