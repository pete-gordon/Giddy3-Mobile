
ifeq ($(PALMPDK),)
PALMPDK := C:\Program Files (x86)\HP webOS\PDK
endif

CFLAGS = -Wall -O3 "-I$(PALMPDK)/include" "-I$(PALMPDK)/include/SDL"
LFLAGS = "-L$(PALMPDK)/device/lib" --allow-shlib-undefined

LIBS = -lSDL -lGLES_CM -lpdl -lz

CC = arm-none-linux-gnueabi-gcc
CXX = arm-none-linux-gnueabi-g++
AR = arm-none-linux-gnueabi-ar
RANLIB = arm-none-linux-gnueabi-ranlib
STRIP = arm-none-linux-gnueabi-strip
DEBUGLIB =
TARGET = giddy3

OBJECTS = \
	enemies.o \
	giddy3.o \
	ptplay.o \
	render.o \
	samples.o \
	specials.o \
	tables.o \
	titles.o

all: webos-package/$(TARGET)/$(TARGET)

run: $(TARGET)
	cmd "/C pdk-run $(TARGET) "

webos-package/$(TARGET)/$(TARGET): $(TARGET)
	copy $(TARGET) webos-package\$(TARGET)\$(TARGET)
	$(STRIP) webos-package\$(TARGET)\$(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(LFLAGS) $(LIBS)


-include $(OBJECTS:.o=.d)

# Rules based build for standard *.c to *.o compilation
$(OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
	@$(CC) -MM $(CFLAGS) $< > $*.d

clean:
	del $(TARGET) *.bak *.o *.d
