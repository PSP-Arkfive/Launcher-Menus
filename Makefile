PY = $(shell which python3)
PSPDEV = $(shell psp-config --pspdev-path)
BUILDTOOLS = $(PSPDEV)/share/psp-cfw-sdk/build-tools

all: vshmenu arkmenu xmenu

vshmenu:
	$(Q)mkdir -p dist
	$(Q)make -C vshMenu
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/VSHMENU.PRX $(BUILDTOOLS)/gz/UserModule.hdr vshMenu/satelite.prx VshCtrlSatelite 0x0000

arkmenu:
	$(Q)mkdir -p dist
	$(Q)make -C arkMenu
	$(Q)cp arkMenu/EBOOT.PBP dist/VBOOT.PBP
	$(Q)cp arkMenu/THEME.ARK dist/

xmenu:
	$(Q)mkdir -p dist
	$(Q)make -C xMenu
	$(Q)cp xMenu/EBOOT.PBP dist/XBOOT.PBP


clean:
	$(Q)make -C vshMenu clean
	$(Q)make -C arkMenu distclean
	$(Q)make -C xMenu clean
	$(Q)rm -rf dist
