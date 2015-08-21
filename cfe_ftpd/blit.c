#include <pspdisplay.h>
#include <pspdebug.h>
extern u8 msx[];

static u32 fcolor = 0x00ffffff;
static u32 bcolor = 0xff000000;

static u32 adjust_alpha(u32 col)
{
	u32 alpha = col>>24;
	u8 mul;
	u32 c1,c2;

	if(alpha==0)    return col;
	if(alpha==0xff) return col;

	c1 = col & 0x00ff00ff;
	c2 = col & 0x0000ff00;
	mul = (u8)(255-alpha);
	c1 = ((c1*mul)>>8)&0x00ff00ff;
	c2 = ((c2*mul)>>8)&0x0000ff00;
	return (alpha<<24)|c1|c2;
}

/////////////////////////////////////////////////////////////////////////////
// blit text
/////////////////////////////////////////////////////////////////////////////
int blit_string(int sx,int sy,const char *msg,int fg_col,int bg_col)
{
	int x,y,p;
	int offset;
	char code;
	unsigned char font;
	
	u32 col,c1,c2;
	u32 alpha;

	fg_col = adjust_alpha(fcolor);
	bg_col = adjust_alpha(bcolor);

	int pwidth, pheight, bufferwidth, pixelformat, unk, ret;
	unsigned int* vram32;
   	sceDisplayGetMode(&unk, &pwidth, &pheight);
   	sceDisplayGetFrameBuf((void*)&vram32, &bufferwidth, &pixelformat, &unk);

    if(bufferwidth == 0)
        return;
    if(pixelformat != 3)//this has problems when pixel format == 3 (in gta vcs anyway)
    {
        pspDebugScreenSetInit();//set init = 1;
		
        pspDebugScreenSetColorMode(pixelformat);
        pspDebugScreenSetOffset(0);
        pspDebugScreenSetBase(0x44000000);

        pspDebugScreenSetBackColor(bg_col);
        pspDebugScreenSetTextColor(fg_col);
        pspDebugScreenSetXY(sx,sy);
		
        return pspDebugScreenPuts(msg);
     }
     else
     {
        sy*=8;
    	for(x=0;msg[x] && x<(pwidth/8);x++)
    	{
    		code = msg[x] & 0x7f; // 7bit ANK
    		for(y=0;y<7;y++)
    		{
    			offset = (sy+y)*bufferwidth + (sx+x)*8;
    			font = msx[ code*8 + y ];
    			for(p=0;p<8;p++)
    			{
				
					col = (font & 0x80) ? fg_col : bg_col;
					alpha = col>>24;
					if(alpha==0) vram32[offset] = col;
					else if(alpha!=0xff)
					{
						c2 = vram32[offset];
						c1 = c2 & 0x00ff00ff;
						c2 = c2 & 0x0000ff00;
						c1 = ((c1*alpha)>>8)&0x00ff00ff;
						c2 = ((c2*alpha)>>8)&0x0000ff00;
						vram32[offset] = (col&0xffffff) + c1 + c2;
					}
    				//vram32[offset] = (font & 0x80) ? fg_col : bg_col;
					
    				font <<= 1;
    				offset++;
    			}
    		}
    	}
    	return x;
    }
}
