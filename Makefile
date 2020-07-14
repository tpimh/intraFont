CC = x86_64-w64-mingw32-gcc
AR = x86_64-w64-mingw32-gcc-ar

CFLAGS = -Wall -I include 
LDFLAGS =

SRC_DIR = src

OBJS = src/intraFont.o src/libccc.o
TARGET_LIB = lib/libintrafont.a

all: release

debug: CFLAGS += -O0 -g -DDEBUG
debug: $(TARGET_LIB)

release: CFLAGS += -Os -ffast-math
release: $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	$(AR) crus $@ $^

dist: release
	tar -zcvf libintraFont_x86_64-w64-mingw32.tar.gz $(TARGET_LIB) include/intraFont.h include/libccc.h

clean:
	-del $(SRC_DIR)\\*.o