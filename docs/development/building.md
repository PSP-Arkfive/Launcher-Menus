# Build Notes

## Targets

| Command | Output |
| --- | --- |
| `make vshmenu` | `dist/VSHMENU.PRX` |
| `make arkmenu` | `dist/VBOOT.PBP`, `dist/THEME.ARK` |
| `make xmenu` | `dist/XBOOT.PBP` |
| `make` | all targets |
| `make clean` | remove generated files |

## Local PSPDEV

```sh
export PSPDEV="$HOME/pspdev"
export PSPSDK="$PSPDEV/psp/sdk"
export PATH="$PSPDEV/bin:$PATH"
psp-pacman -Syu ya2d
make vshmenu
```

## Docker

Use Docker when the host toolchain fails, for example with `GLIBC_2.38 not found`.

```sh
docker run --rm \
  -v "$PWD:/work" \
  -w /work \
  pspdev/pspdev:latest \
  sh -lc '
    set -e
    apk add --no-cache python3
    export PATH=/usr/local/pspdev/bin:$PATH
    export PSPDEV=/usr/local/pspdev
    export PSPSDK=/usr/local/pspdev/psp/sdk
    psp-pacman -Syu --noconfirm ya2d
    make vshmenu
  '
```

Docker may leave root-owned files in the repo. Run `make clean` or fix ownership with `chown`.

## Common Failures

| Error | Fix |
| --- | --- |
| `ya2d.h: No such file or directory` | `psp-pacman -Syu ya2d` |
| `python3: No such file or directory` | `apk add --no-cache python3` |
| `GLIBC_2.38 not found` | Use Docker or rebuild/update local PSPDEV. |
