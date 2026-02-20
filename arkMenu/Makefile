PSPSDK = $(shell psp-config --pspsdk-path)
PSPDEV = $(shell psp-config --pspdev-path)
PYTHON = $(shell which python3)
CFWSDK = $(PSPDEV)/share/psp-cfw-sdk

TARGET = ARKMENU
CFLAGS = -Os -G0 -Wall -Wno-unused -Wno-write-strings -Wno-sign-compare
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif
LIBDIR = 
INCDIR = \
	include \
	include/anim \

LDFLAGS =
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = arkMenu
PSP_EBOOT_ICON = ICON0.PNG
PSP_FW_VERSION=660
BUILD_PRX = 1

OBJS = \
	main.o \
	imports.o \
	src/debug.o \
	src/common.o \
	src/lang.o \
	src/system_mgr.o \
	src/gamemgr.o \
	src/net_mgr.o \
	src/entry.o \
	src/iso.o \
	src/umd.o \
	src/eboot.o \
	src/menu.o \
	src/controller.o \
	src/ya2d++.o \
	src/browser.o \
	src/browser_entries.o \
	src/osk.o \
	src/usb.o \
	src/network.o \
	src/settingsmenu.o \
	src/optionsmenu.o \
	src/texteditor.o \
	src/image_viewer.o \
	src/music_player.o \
	src/mp3.o \
	src/ftp_driver.o \
	src/pspav_wrapper.o \
	src/anim/anim.o \
	src/anim/pixel.o \
	src/anim/wave.o \
	src/anim/sprites.o \
	src/anim/fire.o \
	src/anim/tetris.o \
	src/anim/matrix.o \
	src/anim/hacker.o \
	src/anim/bsod.o \
	src/anim/snow.o \
	src/anim/gol.o \
	src/anim/noanim.o \

PRX_EXPORTS = exports.exp

LIBS = \
	-lstdc++ \
	-lya2d \
	-lpng \
	-lcjson \
	-lintrafont \
	-lz \
	-lm \
	-lpspav \
	-lpspftp \
	-lpspvfpu \
	-lpspjpeg \
	-lpspmpeg \
	-lpspaudio \
	-lpsppower \
	-lpspgum \
	-lpspgu \
	-lpsprtc \
	-lpspvram \
	-lpspmath \
	-lpspumd \
	-lpspmp3 \
	-lpspatrac3 \
	-lpspwlan \
	-lpspnet \
	-lpspnet_inet \
	-lpsphttp \
	-lpspusb \
	-lpspdisplay \
	-lpspusbdevice \
	-lpspsystemctrl_user \
	-lpspkubridge \
	-larkctrl

all: $(TARGET).prx
	$(Q)$(PYTHON) $(CFWSDK)/build-tools/pack/pkg-res.py $(CURDIR)/themes THEME.ARK
	$(Q)cp themes/ARK_Revamped/THEME.ARK ./

distclean:
	make clean
	$(Q)-rm -f THEME.ARK

include $(PSPSDK)/lib/build.mak
