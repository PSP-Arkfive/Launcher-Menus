# Launcher-Menus

Launcher-Menus builds the menu files ARK uses outside the normal XMB flow.
That includes the small VSH overlay, the main ARK launcher, and a lighter alternate launcher.

Build outputs:

| File               | Built from        | Used for                         |
| ------------------ | ----------------- | -------------------------------- |
| `dist/VSHMENU.PRX` | `vshMenu/`        | SELECT-menu overlay in XMB.      |
| `dist/VBOOT.PBP`   | `arkMenu/`        | Main launcher and recovery menu. |
| `dist/XBOOT.PBP`   | `xMenu/`          | Smaller alternate launcher.      |
| `dist/THEME.ARK`   | `arkMenu/themes/` | Default `arkMenu` theme package. |

## Docs

- [Module Notes](docs/architecture/modules.md)
- [Build Notes](docs/development/building.md)
- [Test Notes](docs/development/testing.md)

## Requirements

Required toolchain:

- `psp-config` and `psp-gcc`
- PSPSDK
- `psp-cfw-sdk` build tools
- Python 3
- menu libraries such as `ya2d`, `tinyfont`, `intraFont`, `libpng`, `cJSON`, and ARK control libraries

In the PSPDEV Docker image, install Python 3 and `ya2d` before running `make`:

```sh
apk add --no-cache python3
export PATH=/usr/local/pspdev/bin:$PATH
export PSPDEV=/usr/local/pspdev
export PSPSDK=/usr/local/pspdev/psp/sdk
psp-pacman -Syu --noconfirm ya2d
```

## Build

```sh
make vshmenu   # dist/VSHMENU.PRX
make arkmenu   # dist/VBOOT.PBP and dist/THEME.ARK
make xmenu     # dist/XBOOT.PBP
make           # all of the above
make clean
```

## Quick Test

For a VSH menu test, back up the existing file on the memory stick and replace it with the new one:

```text
dist/VSHMENU.PRX -> PSP/SAVEDATA/ARK_01234/VSHMENU.PRX
```

ARK tries this file before falling back to `flash0:/vsh/module/ark_satelite.prx`.

For launcher tests, copy `VBOOT.PBP`, `XBOOT.PBP`, or `THEME.ARK` into the same ARK folder as needed.

## Contributing

- Do not commit build artifacts unless the release process needs them.
- `vshMenu` reads satellite language files and `.pf` fonts from ARK's `LANG.ARK`.
- CJK satellite strings are byte-indexed font data at runtime. They are not plain UTF-8 or GBK.
- `arkMenu` and `xMenu` use JSON language files with `intraFont` UTF-8 rendering.
- Test hardware paths that touch VSH hooks, memory allocation, font loading, or XMB state.
