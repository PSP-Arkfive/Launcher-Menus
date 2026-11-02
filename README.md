# Launcher-Menus
Custom launcher menus for ARK. Very useful when the XMB is not available or misbehaving.

## Building
- Requires `ya2d` package: `psp-pacman -Syu ya2d`

## Extra features (arkMenu)

### ISO folder categories (folder mode)
- Every subfolder of `ms0:/ISO/` (except `VIDEO`) becomes a category, the same way Game Categories Lite does it for the XMB.
- With the Games column focused, press **R** (RT) to pick a category: `All games`, every folder with its game count, and `* Favorites`.
- Requires `Scan category entries` to be enabled in the launcher settings (same option that makes the flat scan recursive).
- The active category is shown in the header as `[folder]`; folders are sorted alphabetically, so `00GTA`, `01Akcja`, ... keep your order.

### Favorites
- **L** (LT) on a game -> `Add to Favorites` / `Remove from Favorites`.
- Favorites are marked with a `*` and get their own `* Favorites` category.
- Persisted in `ms0:/SEPLUGINS/arkmenu_fav.txt` (one absolute path per line).

### gclite_filter.txt (XMB <-> launcher sync)
- `ms0:/SEPLUGINS/gclite_filter.txt` is honoured: `===HIDDEN CATEGORIES===` entries are not offered as categories, `===HIDDEN APPS===` and legacy entries are removed from the game lists.
