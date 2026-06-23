PSPSDK=$(shell psp-config --pspsdk-path)

TARGET = XMENU
CFLAGS = -O3 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
INCDIR = include
LIBDIR =
LIBS = -lstdc++ -lpng -lya2d -lintrafont -lcjson -lz -lpspvram -lpspmath -lpspgu -lpspgum -lpspsystemctrl_user -larkctrl -lpsppower
LDFLAGS = -L libs
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = xMenu
PSP_EBOOT_ICON = resources/ICON0.PNG
PSP_EBOOT_PIC1 = resources/PIC1.PNG
PSP_LARGE_MEMORY = 0

OBJS = \
		main.o \
		src/debug.o \
		src/common.o \
		src/submenu.o \
		src/menu.o \
		src/entry.o \
		src/controller.o \
		src/text.o \
		src/lang.o \


include $(PSPSDK)/lib/build.mak
