#include "fire.h"
#include "string.h"
#include "animtypes.h"
//#include <pspdisplay.h>

static u16 packFireColor(int red, int green, int blue){
    return (red >> 3) | ((green >> 3) << 5) | ((blue >> 3) << 10) | 0x8000;
}

Fire :: Fire()
{
    int i,v;
    u16 *rawdata1;

    memset(fireMemY,0,fireMaxX*fireMaxY);    //clear fire buffer
    this->firetex = ya2d_create_texture (fireMaxX,fireMaxY,GU_PSM_5551,YA2D_PLACE_RAM);
    //Palette fill (OSLib / YA2D)
    this->firePal[0]=0;
    for (i=1;i<16;i++) this->firePal[i] = packFireColor(0, 0, (i+1)*4);
    for (i=16;i<80;i++) this->firePal[i] = packFireColor((i-15)*4-1, 0, 79-i);
    for (i=80;i<208;i++) this->firePal[i] = packFireColor(255, (i-79)*2-1, 0);
    for (i=208;i<256;i++) this->firePal[i] = packFireColor(255, 255, (i-207)*5);

    /*Palette fill (direct mem)
    for (i=0;i<16;i++) firePal[i]=((i+1)*4)*0x10000L;
    for (i=16;i<80;i++) firePal[i]=((79-i)*0x10000L)+((i-15)*4-1);
    for (i=80;i<208;i++) firePal[i]=0x000000ffL+((((i-79)*2)-1)*0x100L);
    for (i=208;i<256;i++) firePal[i]=(((i-207)*5)*0x10000L)+0x0000ffffL;
    */
    // bottom line draw, this makes the fire look better
    rawdata1=(u16*)firetex->data+firetex->pow2_w*(fireMaxY-1);
    for (v=0;v!=(fireMaxX);v++){
        this->fireMemY[fireMaxY-1].fireMemX[v]=fireBaseColor;
        *rawdata1=this->firePal[fireBaseColor];
        rawdata1++;
    }

};

Fire :: ~Fire()
{
    ya2d_free_texture(this->firetex);
};

int Fire::getId(){
    return ANIM_FIRE;
}

void Fire :: draw()
{
    int i,v,fx,fy;
    u16 *rawdata1;        //Pointer to write into texture
    static int fireFrameCount=0;
    //     for (v=0;v!=256;v++)drawPixel (v,10,1,10,firePal[v]); //Draw palette, debug only (direct mem)
    //     for (v=0;v!=256;v++)ya2d_draw_rect (v,10,1,10,firePal[v],1); //Draw palette, debug only (YA2D)
         if (fireFrameCount==0) {
             for (v=0;v!=fireMaxX;v++) this->fireMemY[fireMaxY-2].fireMemX[v]=(rand()%2*fireBaseColor);//Baseline new fire generator
             for (fy=1;fy!=(fireMaxY-1);fy++){
                 rawdata1=(u16*)this->firetex->data + this->firetex->pow2_w*fy+1;
                 for (fx=1;fx!=(fireMaxX-1);fx++){
                     v=(this->fireMemY[fy].fireMemX[fx] + this->fireMemY[fy].fireMemX[fx-1] + this->fireMemY[fy].fireMemX[fx+1]
                        + this->fireMemY[fy+1].fireMemX[fx])/4;
                     v=v-fireDecay;
                     if (v<0) v=0;
                     this->fireMemY[fy-1].fireMemX[fx]=v;
                     *rawdata1=this->firePal[v];
                     rawdata1++;
                 }
             }
         }
         if (fireScale==1)
             ya2d_draw_texture(this->firetex, firePosX, firePosY);
         else
             ya2d_draw_texture_scale(this->firetex, firePosX, firePosY,fireScale,fireScale);

         //tinyfont_draw_stringf(10, 10,  GU_RGBA(0,0,255,255), "x:%d px:%d y:%d py:%d", fireMaxX, firetex->pow2_w,fireMaxY,firetex->pow2_h);

         fireFrameCount++;
         if (fireFrameCount==fireFrameSkip)fireFrameCount=0;
};
