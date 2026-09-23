# Module Notes

The top-level `Makefile` builds three menus.

| Directory | Output |
| --- | --- |
| `vshMenu/` | `dist/VSHMENU.PRX` |
| `arkMenu/` | `dist/VBOOT.PBP`, `dist/THEME.ARK` |
| `xMenu/` | `dist/XBOOT.PBP` |

Start with:

- `vshMenu/`: `main.c`, `fonts.c`, `lang.c`, `pkg.c`
- `arkMenu/`: `main.cpp`, `src/common.cpp`, `src/system_mgr.cpp`, `src/gamemgr.cpp`
- `xMenu/`: `main.cpp`, `src/common.cpp`, `src/menu.cpp`, `src/entry.cpp`

## Artifact Flow

```text
vshMenu/satelite.prx -> dist/VSHMENU.PRX
arkMenu/EBOOT.PBP -> dist/VBOOT.PBP
arkMenu/THEME.ARK -> dist/THEME.ARK
xMenu/EBOOT.PBP -> dist/XBOOT.PBP
```

## Runtime Names

- `ARKMENU.BIN`: shared menu settings.
- `LANG.ARK`: packed language and font resources.
- `ark_config.arkpath`: ARK install path from `sctrlArkGetConfig()`.

## Language Note

`vshMenu/lang.c` cannot treat every byte below `0x20` as a line ending.
ARK-packaged CJK satellite text uses low byte values as `.pf` glyph indexes.
Only CR and LF are safe terminators.
