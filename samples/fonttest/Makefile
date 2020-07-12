CC = x86_64-w64-mingw32-gcc

TARGET_EXE = fonttest.exe
OBJS = fonttest.o

INCDIR = ../../include
CFLAGS = -Os -Wall -s -I$(INCDIR)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = ../../lib
LDFLAGS = -L$(LIBDIR)
LIBS = -lintrafont -lglfw3 -lopengl32 -lglu32 -luser32 -lgdi32

all: $(TARGET_EXE)

$(TARGET_EXE): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	del $(OBJS) $(TARGET_EXE)