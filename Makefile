.PHONY: vshmenu arkmenu xmenu

PY = $(shell which python3)
PSPDEV = $(shell psp-config --pspdev-path)
BUILDTOOLS = $(PSPDEV)/share/psp-cfw-sdk/build-tools

all: vshmenu arkmenu xmenu

vshmenu:
	mkdir -p dist
	make -C vshMenu
	$(PY) $(BUILDTOOLS)/gz/pspgz.py dist/VSHMENU.PRX $(BUILDTOOLS)/gz/UserModule.hdr vshMenu/satelite.prx VshCtrlSatelite 0x0000

arkmenu:
	mkdir -p dist
	make -C arkMenu
	cp arkMenu/EBOOT.PBP dist/VBOOT.PBP
	$(PY) $(BUILDTOOLS)/pack/pkg-res.py arkMenu/themes THEME.ARK
	cp arkMenu/themes/ARK_Revamped/THEME.ARK dist/
	cp -r arkMenu/themes dist/
	find dist/themes/ -type d -name 'resources' -exec rm -rf {} \; 2>/dev/null || true

xmenu:
	mkdir -p dist
	make -C xMenu
	cp xMenu/EBOOT.PBP dist/XBOOT.PBP

clean:
	make -C vshMenu clean
	make -C arkMenu clean
	make -C xMenu clean
	rm -rf dist
	find -name 'THEME.ARK' -exec rm {} \;
