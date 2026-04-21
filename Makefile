all: arkmenu xmenu


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
	$(Q)make -C arkMenu distclean
	$(Q)make -C xMenu clean
	$(Q)rm -rf dist
