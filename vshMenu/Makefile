PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = satelite
OBJS = main.o fonts.o pkg.o stub.o

CFLAGS = -std=c99 -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

INCDIR = include
LIBDIR =

USE_PSPSDK_LIBS = 1
USE_PSPSDK_LIBC = 1

LIBS = -lya2d -ltinyfont -lpspmath -lpspvram -lguglue -larkctrl -lpspsystemctrl_user -lpspkubridge -lpspvshctrl -lpspreg -lpsppower
LDFLAGS = -nostartfiles

include $(PSPSDK)/lib/build.mak
