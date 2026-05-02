#include <string.h>
#include <pspsdk.h>
#include <pspsysmem.h>
#include <psputility.h>
#include <pspiofilemgr.h>

#include <cfwmacros.h>
#include <systemctrl_ark.h>


extern SceOff findPkgOffset(const char* filename, unsigned* size, const char* pkgpath);

static SceUID mem_id = -1;

const char* available_fonts[] = {
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

static void* load_external_font(const char *file) {
    SceUID fd;
    int ret;
    void *buf;
    unsigned int size = 0;
    ARKConfig ark_config;

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
        sceIoClose(fd);
        return NULL;
    }

    buf = sceKernelGetBlockHeadAddr(mem_id);

    if (buf == NULL) {
        sceKernelFreePartitionMemory(mem_id);
        sceIoClose(fd);
        return NULL;
    }

    sceIoLseek(fd, offset, PSP_SEEK_SET);
    ret = sceIoRead(fd, buf, size);

    if(ret != size) {
        sceKernelFreePartitionMemory(mem_id);
        sceIoClose(fd);
        return NULL;
    }

    sceIoClose(fd);
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
        	if (conf->vsh_font != 49){
        		conf->vsh_font = 49;
        	}
        	break;
        case PSP_SYSTEMPARAM_LANGUAGE_CHINESE_SIMPLIFIED:
        	// make sure we use a specific chinese font
        	if (conf->vsh_font != 56){
        		conf->vsh_font = 56;
        	}
        	break;
        case PSP_SYSTEMPARAM_LANGUAGE_CHINESE_TRADITIONAL:
        	// make sure we use a specific chinese font
        	if (conf->vsh_font != 57){
        		conf->vsh_font = 57;
        	}
        	break;
        case PSP_SYSTEMPARAM_LANGUAGE_JAPANESE:
        	// make sure we use a specific chinese font
        	if (conf->vsh_font != 58){
        		conf->vsh_font = 58;
        	}
        	break;

        case PSP_SYSTEMPARAM_LANGUAGE_KOREAN:
        	// make sure we use a specific chinese font
        	if (conf->vsh_font != 59){
        		conf->vsh_font = 59;
        	}
        	break;

        /*
        // use CP881 font for French
        case PSP_SYSTEMPARAM_LANGUAGE_FRENCH:
        	load_external_font("CP881.pf");
        	vsh->config.ark_menu.vsh_font = 32;
        	break;
        */
        default:
        	break;
    }

    // if a font is needed (ie not 0)
    if (conf->vsh_font) {
        // load external font
        return load_external_font(available_fonts[conf->vsh_font - 1]);
    }

    return NULL;
}

void release_font(void) {
    if (mem_id >= 0) {
        sceKernelFreePartitionMemory(mem_id);
        mem_id = -1;
    }
}
