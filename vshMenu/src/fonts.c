#include <string.h>
#include <pspsdk.h>
#include <pspsysmem.h>
#include <psputility.h>
#include <pspiofilemgr.h>

#include <cfwmacros.h>
#include <systemctrl_ark.h>
#include <tinyfont.h> // msx (built-in font)


extern SceOff findPkgOffset(const char* filename, unsigned* size, const char* pkgpath);

static SceUID mem_id = -1;

// A bitmap font is 256 glyphs of 8x8 pixels, 8 bytes each (one byte per row, MSB = leftmost pixel).
#define FONT_SIZE 2048
// The built-in msx table only has 255 glyphs (2040 bytes): the last one (0xFF) does not exist.
#define MSX_SIZE 2040

// Copy of the built-in font in RAM, so its glyphs can be corrected without touching the library's table.
static u8 msx_buf[FONT_SIZE];

const char* available_fonts[] = {
    "msx",
    "8X8!FONT.pf",
    "8X8#FONT.pf",
    "8X8@FONT.pf",
    "8X8ITAL.pf",
    "SMEGA88.pf",
    "APEAUS.pf",
    "SMVGA88.pf",
    "APLS.pf",
    "SPACE8.pf",
    "Standard.pf",
    "TINYTYPE.pf",
    "FANTASY.pf",
    "THIN8X8.pf",
    "THIN_SS.pf",
    "CP111.pf",
    "CP112.pf",
    "CP113.pf",
    "CP437old.pf",
    "CP437.pf",
    "CP850.pf",
    "CP851.pf",
    "CP852.pf",
    "CP853.pf",
    "CP860.pf",
    "CP861.pf",
    "CP862.pf",
    "CP863.pf",
    "CP864.pf",
    "CP865.pf",
    "CP866.pf",
    "CP880.pf",
    "CP881.pf",
    "CP882.pf",
    "CP883.pf",
    "CP884.pf",
    "CP885.pf",
    "CRAZY8.pf",
    "DEF_8X8.pf",
    "VGA-ROM.pf",
    "EVGA-ALT.pf",
    "FE_8X8.pf",
    "GRCKSSRF.pf",
    "HERCITAL.pf",
    "HERCULES.pf",
    "MAC.pf",
    "MARCIO08.pf",
    "READABLE.pf",
    "ROM8PIX.pf",
    "RUSSIAN.pf",
    "CYRILL1.pf",
    "CYRILL2.pf",
    "CYRILL3.pf",
    "CYRIL_B.pf",
    "ARMENIAN.pf",
    "GREEK.pf",
    "CHS.pf",
    "CHT.pf",
    "JP.pf",
    "KR.pf",
};

const char** font_list(int* nfonts) {
    if (nfonts) *nfonts = NELEMS(available_fonts);
    return available_fonts;
}


/*
 * Accented letters for CP850 text.
 *
 * The western-European translations (satelite_pt.txt, satelite_fr.txt, ...) are single-byte CP850 text,
 * and tinyfont uses each byte directly as the glyph index. Only CP850.pf has its glyphs laid out in CP850
 * order; every other font (msx, the style fonts, the Asian fonts, ...) has a different layout, so bytes
 * like 0xC6 (a-tilde) or 0xE4 (o-tilde) draw bars, sigma, blocks or nothing.
 *
 * To make every font usable, after loading a font we rebuild, in RAM, the glyphs for the accented letters
 * at their CP850 positions. Each one is composed from the SAME font's plain letter (always at its ASCII
 * position) plus a small accent mark, so the font keeps its own style.
 */
enum {
    ACC_ACUTE,
    ACC_GRAVE,
    ACC_CIRC,
    ACC_TILDE,
    ACC_DIAER,
    ACC_RING,
    ACC_CEDIL,
};

typedef struct {
    u8 code;   // CP850 byte
    char base; // plain letter
    u8 acc;    // accent mark
} AccentedLetter;

static const AccentedLetter cp850_letters[] = {
    {0x80, 'C', ACC_CEDIL}, {0x81, 'u', ACC_DIAER}, {0x82, 'e', ACC_ACUTE}, {0x83, 'a', ACC_CIRC},
    {0x84, 'a', ACC_DIAER}, {0x85, 'a', ACC_GRAVE}, {0x86, 'a', ACC_RING},  {0x87, 'c', ACC_CEDIL},
    {0x88, 'e', ACC_CIRC},  {0x89, 'e', ACC_DIAER}, {0x8A, 'e', ACC_GRAVE}, {0x8B, 'i', ACC_DIAER},
    {0x8C, 'i', ACC_CIRC},  {0x8D, 'i', ACC_GRAVE}, {0x8E, 'A', ACC_DIAER}, {0x8F, 'A', ACC_RING},
    {0x90, 'E', ACC_ACUTE}, {0x93, 'o', ACC_CIRC},  {0x94, 'o', ACC_DIAER}, {0x95, 'o', ACC_GRAVE},
    {0x96, 'u', ACC_CIRC},  {0x97, 'u', ACC_GRAVE}, {0x98, 'y', ACC_DIAER}, {0x99, 'O', ACC_DIAER},
    {0x9A, 'U', ACC_DIAER}, {0xA0, 'a', ACC_ACUTE}, {0xA1, 'i', ACC_ACUTE}, {0xA2, 'o', ACC_ACUTE},
    {0xA3, 'u', ACC_ACUTE}, {0xA4, 'n', ACC_TILDE}, {0xA5, 'N', ACC_TILDE}, {0xB5, 'A', ACC_ACUTE},
    {0xB6, 'A', ACC_CIRC},  {0xB7, 'A', ACC_GRAVE}, {0xC6, 'a', ACC_TILDE}, {0xC7, 'A', ACC_TILDE},
    {0xD2, 'E', ACC_CIRC},  {0xD3, 'E', ACC_DIAER}, {0xD4, 'E', ACC_GRAVE}, {0xD6, 'I', ACC_ACUTE},
    {0xD7, 'I', ACC_CIRC},  {0xD8, 'I', ACC_DIAER}, {0xE0, 'O', ACC_ACUTE}, {0xE2, 'O', ACC_CIRC},
    {0xE3, 'O', ACC_GRAVE}, {0xE4, 'o', ACC_TILDE}, {0xE5, 'O', ACC_TILDE}, {0xE9, 'U', ACC_ACUTE},
    {0xEA, 'U', ACC_CIRC},  {0xEB, 'U', ACC_GRAVE}, {0xEC, 'y', ACC_ACUTE}, {0xED, 'Y', ACC_ACUTE},
};

// Accent marks, 2 rows each, centred on column 3 (bit 0x10); shifted horizontally to sit over the letter.
static const u8 accent_shape[][2] = {
    [ACC_ACUTE] = {0x08, 0x10},
    [ACC_GRAVE] = {0x10, 0x08},
    [ACC_CIRC]  = {0x10, 0x28},
    [ACC_TILDE] = {0x28, 0x50},
    [ACC_DIAER] = {0x00, 0x28},
    [ACC_RING]  = {0x30, 0x30},
};

#define ACCENT_ROWS 2

// Builds the glyph for 'base' + 'acc' using the glyphs of 'font'. Returns 0 if the font has no such letter.
static int compose_accent(const u8* font, u8* out, char base, int acc) {
    u8 g[8];
    int top = -1, bot = -1;
    int c0 = -1, c1 = -1;
    u8 cols = 0;
    int i, k;

    memcpy(g, &font[((u8)base) * 8], 8);

    for (i = 0; i < 8; i++) {
        if (g[i]) {
            if (top < 0) top = i;
            bot = i;
            cols |= g[i];
        }
    }
    if (top < 0) return 0; // the font has no glyph for this letter: leave the original one

    for (k = 0; k < 8; k++) {
        if (cols & (0x80 >> k)) {
            if (c0 < 0) c0 = k;
            c1 = k;
        }
    }
    int cx = (c0 + c1) / 2; // horizontal centre of the letter

    if (acc == ACC_CEDIL) {
        // one pixel under the letter (or on its last row if there is no room)
        if (bot < 7) g[bot + 1] |= (0x80 >> cx);
        else g[7] |= (0x80 >> cx);
    }
    else {
        if (top < ACCENT_ROWS) {
            // no room above the letter: squeeze it (dropping middle rows) so it fits in the bottom rows
            u8 body[8];
            int n = bot - top + 1;
            memcpy(body, &g[top], n);
            while (n > 8 - ACCENT_ROWS) {
                int m = n / 2;
                memmove(&body[m], &body[m + 1], n - m - 1);
                n--;
            }
            memset(g, 0, 8);
            memcpy(&g[8 - n], body, n);
            top = 8 - n;
        }
        int shift = cx - 3;
        for (k = 0; k < ACCENT_ROWS; k++) {
            u8 v = accent_shape[acc][k];
            v = (shift >= 0) ? (u8)(v >> shift) : (u8)(v << (-shift));
            g[top - ACCENT_ROWS + k] |= v;
        }
    }

    memcpy(out, g, 8);
    return 1;
}

// Rebuilds the CP850 accented letters of a 256-glyph font table, in place.
// Only glyphs at 0x80-0xFF are written; the plain letters they are made from (0x20-0x7E) are never touched.
static void fix_cp850_glyphs(u8* font) {
    unsigned i;
    for (i = 0; i < NELEMS(cp850_letters); i++) {
        u8 glyph[8];
        if (compose_accent(font, glyph, cp850_letters[i].base, cp850_letters[i].acc)) {
            memcpy(&font[cp850_letters[i].code * 8], glyph, 8);
        }
    }
    // 0xFF is a no-break space in CP850: make sure it is blank
    memset(&font[0xFF * 8], 0, 8);
}

// Languages whose satelite_xx.txt is written in CP850 (the others use their own fonts, see font_load)
static int language_uses_cp850(int lang) {
    switch (lang) {
        case PSP_SYSTEMPARAM_LANGUAGE_FRENCH:
        case PSP_SYSTEMPARAM_LANGUAGE_SPANISH:
        case PSP_SYSTEMPARAM_LANGUAGE_GERMAN:
        case PSP_SYSTEMPARAM_LANGUAGE_ITALIAN:
        case PSP_SYSTEMPARAM_LANGUAGE_DUTCH:
        case PSP_SYSTEMPARAM_LANGUAGE_PORTUGUESE:
            return 1;
        default:
            return 0;
    }
}


static void* load_external_font(const char *file, unsigned int* out_size) {
    SceUID fd;
    int ret;
    void *buf;
    unsigned int size = 0;
    ARKConfig ark_config;

    if (out_size) *out_size = 0;

    if (file == NULL || file[0] == 0) return NULL;

    sctrlArkGetConfig(&ark_config);

    char pkgpath[ARK_PATH_SIZE];
    strcpy(pkgpath, ark_config.arkpath);
    strcat(pkgpath, "LANG.ARK");

    SceOff offset = findPkgOffset(file, &size, pkgpath);

    if (offset == 0) return NULL;

    fd = sceIoOpen(pkgpath, PSP_O_RDONLY, 0777);

    if(fd < 0) {
        return NULL;
    }

    mem_id = sceKernelAllocPartitionMemory(2, "proDebugScreenFontBuffer", PSP_SMEM_High, size, NULL);

    if(mem_id < 0) {
        mem_id = -1;
        sceIoClose(fd);
        return NULL;
    }

    buf = sceKernelGetBlockHeadAddr(mem_id);

    if (buf == NULL) {
        sceKernelFreePartitionMemory(mem_id);
        mem_id = -1;
        sceIoClose(fd);
        return NULL;
    }

    sceIoLseek(fd, offset, PSP_SEEK_SET);
    ret = sceIoRead(fd, buf, size);

    if(ret != size) {
        sceKernelFreePartitionMemory(mem_id);
        mem_id = -1;
        sceIoClose(fd);
        return NULL;
    }

    sceIoClose(fd);
    if (out_size) *out_size = size;
    return buf;
}

void* font_load(ArkMenuConf* conf) {

    int ret, value;
    // get device language
    ret = sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &value);
    
    // if language not found, default to english
    if (ret < 0)
        value = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;

    switch (value) {
        case PSP_SYSTEMPARAM_LANGUAGE_RUSSIAN:
        	// make sure we use a russian font
        	if (conf->vsh_font < 49 || conf->vsh_font > 53){
        		conf->vsh_font = 49;
        	}
        	break;
        case PSP_SYSTEMPARAM_LANGUAGE_CHINESE_SIMPLIFIED:
        	// make sure we use a specific chinese font
        	conf->vsh_font = 56;
        	break;
        case PSP_SYSTEMPARAM_LANGUAGE_CHINESE_TRADITIONAL:
        	// make sure we use a specific chinese font
        	conf->vsh_font = 57;
        	break;
        case PSP_SYSTEMPARAM_LANGUAGE_JAPANESE:
        	// make sure we use a specific chinese font
        	conf->vsh_font = 58;
        	break;

        case PSP_SYSTEMPARAM_LANGUAGE_KOREAN:
        	// make sure we use a specific chinese font
        	conf->vsh_font = 59;
        	break;

        default:
        	break;
    }

    // Western-European translations are CP850 text: accented letters must be rebuilt for the chosen font
    int fix_accents = language_uses_cp850(value);

    // vsh_font is stored as an index into available_fonts.
    // index 0 is the built-in msx font, so external fonts must not be shifted.
    if (conf->vsh_font && conf->vsh_font < NELEMS(available_fonts)) {
        // load external font
        const char* name = available_fonts[conf->vsh_font];
        unsigned int size = 0;
        u8* buf = load_external_font(name, &size);

        if (buf) {
            // CP850.pf already has its glyphs in CP850 order: leave it exactly as it is
            if (fix_accents && size >= FONT_SIZE && strcmp(name, "CP850.pf") != 0) {
                fix_cp850_glyphs(buf);
            }
            return buf;
        }

        // the font could not be loaded: fall back to the built-in font (vsh_font is kept as is)
    }
    else {
        conf->vsh_font = 0;
    }

    // built-in msx font, copied to RAM so that it can be corrected as well
    memset(msx_buf, 0, sizeof(msx_buf));
    memcpy(msx_buf, msx, MSX_SIZE);
    if (fix_accents) {
        fix_cp850_glyphs(msx_buf);
    }

    return msx_buf;
}

void release_font(void) {
    if (mem_id >= 0) {
        sceKernelFreePartitionMemory(mem_id);
        mem_id = -1;
    }
}
