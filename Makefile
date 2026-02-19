all:
	$(Q)make -C arkMenu
	$(Q)make -C xMenu
	$(Q)mkdir -p dist
	$(Q)cp arkMenu/EBOOT.PBP dist/VBOOT.PBP
	$(Q)cp xMenu/EBOOT.PBP dist/XBOOT.PBP
	$(Q)cp arkMenu/THEME.ARK dist/

clean:
	$(Q)make -C arkMenu distclean
	$(Q)make -C xMenu clean
	$(Q)rm -rf dist
