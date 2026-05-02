#include <string.h>
#include <pspsdk.h>
#include <pspiofilemgr.h>

SceOff findPkgOffset(const char* filename, unsigned* size, const char* pkgpath) {

    int pkg = sceIoOpen(pkgpath, PSP_O_RDONLY, 0777);
    if (pkg < 0)
        return 0;
     
    unsigned pkgsize = sceIoLseek32(pkg, 0, PSP_SEEK_END);
    unsigned size2 = 0;
     
    sceIoLseek32(pkg, 0, PSP_SEEK_SET);

    if (size != NULL)
        *size = 0;

    unsigned offset = 0;
    char name[64];
           
    while (offset != 0xFFFFFFFF) {
        sceIoRead(pkg, &offset, 4);
        if (offset == 0xFFFFFFFF) {
        	sceIoClose(pkg);
        	return 0;
        }
        unsigned namelength;
        sceIoRead(pkg, &namelength, 4);
        sceIoRead(pkg, name, namelength+1);
        		   
        if (!strncmp(name, filename, namelength)){
        	sceIoRead(pkg, &size2, 4);
    
        	if (size2 == 0xFFFFFFFF)
        		size2 = pkgsize;

        	if (size != NULL)
        		*size = size2 - offset;
     
        	sceIoClose(pkg);
        	return offset;
        }
    }
    return 0;
}
