CC = x86_64-w64-mingw32-gcc

CFLAGS = -Os -ffast-math -Wall -I include 
LDFLAGS =

SRC_DIR = src

OBJS = src/intraFont.o src/libccc.o
TARGET_LIB = lib/libintrafont.a

all: $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	$(AR) crus $@ $^

release: $(TARGET_LIB)
	tar -zcvf libintraFont.tar.gz lib include

clean:
	del $(SRC_DIR)\\*.o