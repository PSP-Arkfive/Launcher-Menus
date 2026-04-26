all: vshmenu arkmenu xmenu

vshmenu:
	$(Q)mkdir -p dist
	$(Q)make -C vshMenu
	$(Q)cp vshMenu/satelite.prx dist/VSHMENU.PRX

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
