#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SYSTEM_VGA_MEMORY                     0xB8000
#define SYSTEM_VIDEO_MEMORY                   0xA0000
#define SYSTEM_VESA_INFO_BUFFER               0x8800
#define SYSTEM_VESA_MODE_BUFFER               0x8A00

#define abs(a) (((a) < 0) ? -(a) : (a))

#define UCHAR8A(value) ((unsigned char)(value))
#define UCHAR8B(value) ((unsigned char)((value)>> 8))
#define UCHAR8C(value) ((unsigned char)((value)>>16))
#define UCHAR8D(value) ((unsigned char)((value)>>24))
#define UINT32(a,b,c,d) ((unsigned long)((unsigned char)(a)|((unsigned char)(b)<<8)|((unsigned char)(c)<<16)|((unsigned char)(d)<<24)))

#define RGB24(r,g,b)  ((uint32_t)(((uint8_t)(r) << 16)|((uint8_t)(g) << 8)|((uint8_t)(b))))

//#define RGB(r,g,b)  ((uint32_t)(((uint8_t)(r) << 16)|((uint8_t)(g) << 8)|((uint8_t)(b))))

/*
#define RGB(r,g,b)  ((uint32_t)(rgb(r,g,b)))

#define GetRValue(rgb)  ((uint8_t)(rgb))
#define GetGValue(rgb)  ((uint8_t)(((uint16_t)(rgb)) >> 8))
#define GetBValue(rgb)  ((uint8_t)((rgb)>>16))
*/

#define GetRValue(rgb)      ((uint8_t)(rgb))
#define GetGValue(rgb)      ((uint8_t)(((uint16_t)(rgb)) >> 8))
#define GetBValue(rgb)      ((uint8_t)((rgb) >> 16))

//#define RGB(r,g,b)          ((uint32_t)(((uint8_t)(r)|((uint16_t)((uint8_t)(g))<<8))|(((uint16_t)(uint8_t)(b))<<16)))

#define RGB(r,g,b) ((uint32_t)UINT32(r,g,b,0))

#define offsetpixel(_x, _y, _h) ((_h * _x) + _y)

#define xyoffset(_x,_y,_w) ((_w*_y) + _x)
#define xyoffset16(_x,_y,_w)  ((_w*_y) + (_x * 2))
#define xyoffset24(_x,_y,_w)  ((_w*_y) + (_x * 3))
#define xyoffset32(_x,_y,_w)  ((_w*_y) + (_x * 4))


#define clBlack                                      0x0
#define clMaroon                                     0x80
#define clGreen                                      0x8000
#define clOlive                                      0x8080
#define clNavy                                       0x800000
#define clPurple                                     0x800080
#define clTeal                                       0x808000
#define clGray                                       0x808080
#define clSilver                                     0xc0c0c0
#define clRed                                        0xff
#define clLime                                       0xff00
#define clYellow                                     0xffff
#define clBlue                                       0xff0000
#define clFuchsia                                    0xff00ff
#define clAqua                                       0xffff00
#define clLtGray                                     0xc0c0c0
#define clDkGray                                     0x808080
#define clWhite                                      0xffffff
#define clMoneyGreen                                 0xc0dcc0
#define clSkyBlue                                    0xf0caa6
#define clCream                                      0xf0fbff
#define clMedGray                                    0xa4a0a0
#define clDarkGray                                   0x606060
#define clDarkBlue                                   0x6a240a

#define clBackground RGB(0,0,0)

#define clBtnFace RGB(192,192,192)

#define clBtnHighlight RGB(255,255,255)
#define clBtnShadow  RGB(128,128,128)
#define cl3DDkShadow  RGB(64,64,64)
#define clActiveCaptionIn RGB(0,0,0)
#define clActiveCaptionOut RGB(255,0,0)
#define clInactiveCaptionIn RGB(16,16,16)
#define clInactiveCaptionOut RGB(164,164,164)
#define clHighlight RGB(0,96,192)
#define clHighlightText RGB(255,255,255)
#define clInfo RGB(255,255,128)
#define clInfoText RGB(0,0,0)
#define clWindow RGB(255,255,255)
#define clWindowText RGB(0,0,0)



#pragma pack (push, 1)

typedef struct
{
	char signature[4];
	unsigned short version;
	unsigned long oem;
	unsigned long capabilities;
	unsigned long mode_list;
	unsigned short video_memory_size;
	char reserved_0[236];
	char reserved_1[256];
} vesa_info_t;

typedef struct
{
	unsigned short mode_attributes;
	unsigned char window_a_attributes;
	unsigned char window_b_attributes;
	unsigned short window_granularity;
	unsigned short window_size;
	unsigned short window_a_segment;
	unsigned short window_b_segment;
	unsigned long window_far_ptr;
	unsigned short scan_line_size;
	unsigned short width;
	unsigned short height;
	unsigned char char_width;
	unsigned char char_height;
	unsigned char planes;
	unsigned char depth;
	unsigned char banks;
	unsigned char memory_model;
	unsigned char bank_size;
	unsigned char pages;
	char reserved_0;
	unsigned char red_width;
	unsigned char red_shift;
	unsigned char green_width;
	unsigned char green_shift;
	unsigned char blue_width;
	unsigned char blue_shift;
	char reserved_1[3];
	unsigned long lfb_address;
	char reserved_2[212];
} vesa_mode_t;

#pragma pack (pop)

typedef uint32_t COLORREF;

uint32_t rgb(int r, int g, int b);

COLORREF clLightifDarkR(COLORREF c);
COLORREF clLightifDarkG(COLORREF c);
COLORREF clLightifDarkB(COLORREF c);



vesa_info_t *v_info;
vesa_mode_t *v_mode;

unsigned char *frame_buffer;
unsigned char *double_buffer;

int screen_lock = 0;

unsigned long get_edx(void)
{
    unsigned long edx_reg;
    asm("mov %%edx, %0" : "=r" (edx_reg));
	return edx_reg;
}

unsigned long get_eax(void)
{
    unsigned long eax_reg;
    asm("mov %%eax, %0" : "=r" (eax_reg));
	return eax_reg;
}

uint32_t videoint(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
	uint32_t r_a=0, r_d=0;
	__asm__ volatile ( "int $0x4f" :: "a" (a), "b" (b), "c" (c), "d" (d) );	
	__asm__ volatile ( "mov %%eax, %0" : "=r" (r_a) );
	__asm__ volatile ( "mov %%edx, %0" : "=r" (r_d) );
	switch(a)
	{
		case 3:
		{
			if (r_a == 1)
			{
				if (r_d != 0)
				{
					return SYSTEM_VESA_MODE_BUFFER;
				}
				else 
				{
					return 0;
				}
			}
		}
		break;
		case 5:
		{
			if (r_a == 1)
			{
				if (r_d != 0)
				{
					return SYSTEM_VESA_INFO_BUFFER;
				}
				else 
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}
		break;
	};
	return r_a;
}

unsigned long be2le(unsigned long bigEndian) {
    return ((bigEndian >> 24) & 0x000000FF) | 
           ((bigEndian >> 8) & 0x0000FF00) | 
           ((bigEndian << 8) & 0x00FF0000) | 
           ((bigEndian << 24) & 0xFF000000);
}

unsigned long get_vesa_pixel(int x, int y)
{
	unsigned long c = 0;	
	if (x < 0) return 0;
	if (y < 0) return 0;
	if (x >= v_mode->width) return 0;
	if (y >= v_mode->height) return 0;
	unsigned char *video_memory = (unsigned char*)(v_mode->lfb_address);

	switch (v_mode->depth)
	{
		case 1:
		{
			c = (unsigned char)(video_memory[xyoffset(x,y,v_mode->scan_line_size)] & 0x01);
		};
		break;
		case 2:
		{
			c = (unsigned char)(video_memory[xyoffset(x,y,v_mode->scan_line_size)] & 0x03);
		};
		break;
		case 4:
		{
			c = (unsigned char)(video_memory[xyoffset(x,y,v_mode->scan_line_size)] & 0x0F);
		};
		break;
		case 8:
		{
			c = (unsigned char)(video_memory[xyoffset(x,y,v_mode->scan_line_size)] & 0xFF);
		};
		break;
		case 16:
		{
			c = (unsigned short)(*(unsigned long *)(video_memory + xyoffset16(x, y, v_mode->scan_line_size)) & 0xFFFF);
		};
		break;
		case 24:
		{
			//c = (unsigned long)(*(unsigned long *)(video_memory + xyoffset24(x, y, v_mode->scan_line_size)) & 0xFFFFFF);
			uint32_t p = (unsigned long)(*(unsigned long *)(video_memory + xyoffset24(x, y, v_mode->scan_line_size)) & 0xFFFFFFFF);
			c = UINT32(UCHAR8C(p), UCHAR8B(p), UCHAR8D(p), 0);
		};
		break;
		case 32:
		{
			c = (unsigned long)(*(unsigned long *)(video_memory + xyoffset32(x, y, v_mode->scan_line_size)));
		};
		break;
		default:
		{
			return 0;
		}
		break;
	};
	return c;
}
uint32_t video_function(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
	if (a == 0)
	{
		v_info = (vesa_info_t*)SYSTEM_VESA_INFO_BUFFER;
		v_mode = (vesa_mode_t*)SYSTEM_VESA_MODE_BUFFER;
		return 1;
	}	
	else if (a == 8)
	{
		if (v_mode == NULL) return 0;
		else
		{
			return get_vesa_pixel(b,c);
		}
	}
	else
	{
		return videoint(a, b, c, d);
	}
}

void initvideo(void)
{
	video_function(0, 0, 0, 0);
}

uint16_t getvideomode()
{
	uint16_t mode;
	initvideo();
	mode = video_function(1, 0, 0, 0);
	video_function(3, mode, 0, 0);
	return mode;
}

void setvideomode(uint16_t mode)
{
	initvideo();
	video_function(4, mode, 0, 0);
	int vsize = v_mode->width*v_mode->height*(v_mode->depth/8);
	for(int i=0;i<vsize;i++)
	{
		unsigned char *v_lfb = (unsigned char*)v_mode->lfb_address;
		v_lfb[i] = 0;
	}
}

uint32_t getvideopixel(int x, int y)
{
	if (v_mode == NULL) return 0;
	if (x > v_mode->width) return 0;
	if (y > v_mode->height) return 0;
	uint32_t p = video_function(8, x, y, 0);
	return p;
}

void setvideopixel(int x, int y, uint32_t c)
{
	video_function(9, x, y, c);
}

uint32_t getpixel32(int x, int y)
{
	uint32_t p = getvideopixel(x, y);
	uint32_t c = UINT32(GetRValue(p), GetGValue(p), GetBValue(p), 255);
	return c;
}

uint32_t getpixel(int x, int y)
{
	unsigned char *video_memory = (unsigned char*)(frame_buffer);
	if (screen_lock) video_memory = (unsigned char*)(double_buffer);
	uint32_t p = (*(unsigned long *)(video_memory + xyoffset24(x, y, v_mode->scan_line_size)));//getvideopixel(x, y);
	return p;
}

void drawpixel(int x, int y, uint32_t c)
{
	//setvideopixel(x, y, c);
	unsigned char *video_memory;
	if (screen_lock) video_memory = (unsigned char*)(double_buffer);
	else video_memory = (unsigned char*)(frame_buffer);
	(*(unsigned long *)(video_memory + xyoffset24(x, y, v_mode->scan_line_size))) = RGB(GetRValue(c),GetGValue(c),GetBValue(c));
}

void restorebuffer(void)
{
	unsigned long screen_size = v_mode->width*v_mode->height*4;
	memcpy(double_buffer, frame_buffer, screen_size);
	screen_lock = 1;
}

void swapbuffers(void)
{
	unsigned long screen_size = v_mode->width*v_mode->height*4;
	unsigned char *video_memory = (unsigned char*)(v_mode->lfb_address);
	memcpy(video_memory, double_buffer, screen_size);
	screen_lock = 0;
	/*
	for(int y=0;y<v_mode->height;y++)
	{
		for(int x=0;x<v_mode->width;x++)
		{
			uint32_t c = (unsigned long)(*(unsigned long *)(frame_buffer + xyoffset24(x, y, v_mode->scan_line_size)));
			setvideopixel(x, y, c);
		}
	}
	*/
}

uint32_t rgb(int r, int g, int b)
{
	uint32_t c = video_function(10, r, g, b);
	return c;
}




uint32_t foregroundcolor = 0;

void setcolor(uint32_t c)
{
  foregroundcolor = c;
}

uint32_t getcolor(void)
{
  return foregroundcolor;
}

uint32_t backgroundcolor = 0;

void setbkcolor(uint32_t c)
{
 backgroundcolor = c;
}

uint32_t getbkcolor(void)
{
 return backgroundcolor;
}

uint8_t foregroundalpha=255;

uint8_t getalpha(void)
{
  return foregroundalpha;
}

void setalpha(uint8_t a)
{
  foregroundalpha = a;
}

uint8_t backgroundalpha=255;

uint8_t getbkalpha(void)
{
  return backgroundalpha;
}

void setbkalpha(uint8_t a)
{
  backgroundalpha = a;
}


void setpixelrgb(int x,int y,uint32_t c)
{
  if (v_mode == NULL) return;
  if (x < 0) return;
  if (y < 0) return;
  if (x > v_mode->width) return;
  if (y > v_mode->height) return;
  drawpixel(x,y,c);
}

int intsgn8(int x)
{
	int r = x;
	if (r < 0) r = 0;
	if (r > 255) r = 255;
	return r;
}

void setpixel(int x,int y,uint32_t color,int alpha)
{
  if (v_mode == NULL) return;
  if (x < 0) return;
  if (y < 0) return;
  if (x > v_mode->width) return;
  if (y > v_mode->height) return;
  uint32_t c=0;
  uint8_t sr=0,sg=0,sb=0;
  uint8_t dr=0,dg=0,db=0;
  uint8_t r=0,g=0,b=0;
  uint8_t a=0;
  a = 255-alpha;
  c = getpixel(x,y);
  sb = GetRValue(c);
  sg = GetGValue(c);
  sr = GetBValue(c);
  dr = GetRValue(color);
  dg = GetGValue(color);
  db = GetBValue(color);
  a = intsgn8(a);
  sb = intsgn8(sb);
  sg = intsgn8(sg);
  sr = intsgn8(sr);
  db = intsgn8(db);
  dg = intsgn8(dg);
  dr = intsgn8(dr);
  dr = sr*(a/255.0)+dr*(1.0-(a/255.0));
  dg = sg*(a/255.0)+dg*(1.0-(a/255.0));
  db = sb*(a/255.0)+db*(1.0-(a/255.0));
  db = intsgn8(db);
  dg = intsgn8(dg);
  dr = intsgn8(dr);
  r = db;
  g = dg;
  b = dr;
  b = intsgn8(b);
  g = intsgn8(g);
  r = intsgn8(r);
  c = RGB(r,g,b);
  drawpixel(x,y,c);
}

void drawline(int x1,int y1,int x2,int y2,uint32_t c,int alpha)
{
 int x = x1;
 int sx = 1;
 if ((x2-x1)<0)
 {
  sx = -1;
  x -= 1;
 }
 int y = y1;
 int sy = 1;
 if ((y2-y1)<0)
 {
  sy = -1;
  y -= 1;
 }
 int dx = abs(x2-x1);
 int dy = abs(y2-y1);
 int ic = 0;
 if (dy > dx)
 {
  int t = dx;
  dx = dy;
  dy = t;
  ic = 1;
 }
 int er = 2 * dy - dx;
 int i=0;
 for (i=0;i<dx;i++)
 {
   setpixel(x,y,c,alpha);
   while (er >= 0)
   {
     if (ic)
      x = x+sx;
     else
      y = y+sy;
     er = er-2*dx;
   }
   if (ic)
    y = y+sy;
   else
    x = x+sx;
   er = er+2*dy;
 }
}

int line_ox=0,line_oy=0;

void moveto(int x,int y)
{
 line_ox = x;
 line_oy = y;
}

int line_x=0,line_y=0;

void lineto(int x,int y)
{
  line_x = x;
  line_y = y;
  drawline(line_ox,line_oy,line_x,line_y,getcolor(),getalpha());
}

void rectangle(int x0,int y0,int x1,int y1,uint32_t c,int alpha)
{
  drawline(x0,y0,x1,y0,c,alpha);
  drawline(x1,y0,x1,y1,c,alpha);
  drawline(x0,y1,x1,y1,c,alpha);
  drawline(x0,y0,x0,y1,c,alpha);
}

void fillrect(int x0,int y0,int x1,int y1,uint32_t c,int alpha)
{
int x=0,y=0;
  for (y=y0;y<y1;y++)
  {
    for (x=x0;x<x1;x++)
    {
      setpixel(x,y,c,alpha);
    }
  }
}

void drawcontroledgew(int x,int y,int w,int h, int state, int alpha);


void drawcontroledge(int x,int y,int w,int h, int state, int alpha)
{
int sx=0,sy=0,sw=0,sh=0;
//begindrawing();

if (state==0)
{
fillrect(x,y,x+w,y+h,clBtnFace,alpha);

drawline(x,y,x+w,y,clBtnHighlight,alpha);
drawline(x,y,x,y+h,clBtnHighlight,alpha);

drawline(x+1,y+h-1,x+w-1,y+h-1,clBtnShadow,alpha);
drawline(x+w-1,y+1,x+w-1,y+h,clBtnShadow,alpha);

drawline(x,y+h,x+w,y+h,cl3DDkShadow,alpha);
drawline(x+w,y,x+w,y+h+1,cl3DDkShadow,alpha);

} else if (state==1)
{
fillrect(x,y,x+w,y+h,clBtnFace,alpha);

drawline(x,y,x+w,y,cl3DDkShadow,alpha);
drawline(x,y,x,y+h,cl3DDkShadow,alpha);

drawline(x+1,y+1,x+w-1,y+1,clBtnShadow,alpha);
drawline(x+1,y+1,x+1,y+h-1,clBtnShadow,alpha);

drawline(x,y+h,x+w,y+h,clBtnHighlight,alpha);
drawline(x+w,y,x+w,y+h+1,clBtnHighlight,alpha);

} else if (state==2)
{
fillrect(x,y,x+w,y+h,clBtnFace,alpha);

drawline(x-1,y+1,x+w-1,y+1,clBtnHighlight,alpha);
drawline(x+1,y-1,x+1,y+h-1,clBtnHighlight,alpha);

drawline(x+1,y+h-2,x+w-1,y+h-2,clBtnShadow,alpha);
drawline(x+w-2,y+1,x+w-2,y+h-1,clBtnShadow,alpha);

drawline(x-1,y+h-1,x+w-1,y+h-1,cl3DDkShadow,alpha);
drawline(x+w-2,y-1,x+w-1,y+h-1,cl3DDkShadow,alpha);

rectangle(x,y,x+w,y+h,clBlack,alpha);

} else if (state==3)
{
fillrect(x,y,x+w,y+h,clBtnFace,alpha);
rectangle(x,y,x+w,y+h,clBlack,alpha);
rectangle(x+1,y+1,x+w-1,y+h-1,clBtnShadow,alpha);
} else if (state==4)
{
fillrect(x,y,x+w,y+h,clBtnFace,alpha);

drawline(x-1,y,x+w,y,clBtnHighlight,alpha);
drawline(x,y-1,x,y+h,clBtnHighlight,alpha);

drawline(x,y+h-1,x+w,y+h-1,cl3DDkShadow,alpha);
drawline(x+w-1,y,x+w-1,y+h,cl3DDkShadow,alpha);

drawline(x-1,y+h,x+w,y+h,clBtnShadow,alpha);
drawline(x+w-1,y-1,x+w,y+h,clBtnShadow,alpha);
} else if (state==5)
{
fillrect(x,y,x+w,y+h,clBtnFace,alpha);

drawline(x-1,y,x+w-1,y,clBtnShadow,alpha);
drawline(x,y-1,x,y+h-1,clBtnShadow,alpha);

drawline(x+1,y+1,x+w-2,y+1,cl3DDkShadow,alpha);
drawline(x+1,y,x+1,y+h-2,cl3DDkShadow,alpha);

drawline(x-1,y+h,x+w-1,y+h-1,clBtnHighlight,alpha);
drawline(x+w-1,y-1,x+w,y+h,clBtnHighlight,alpha);

} else if (state==6)
{
fillrect(x,y,x+w,y+h,clBtnFace,alpha);

drawline(x,y,x+w,y,clBtnHighlight,alpha);
drawline(x,y,x,y+h,clBtnHighlight,alpha);

drawline(x,y+h,x+w,y+h,clBtnShadow,alpha);
drawline(x+w,y,x+w,y+h+1,clBtnShadow,alpha);
} else if (state==7)
{
fillrect(x,y,x+w,y+h,clBtnFace,alpha);

drawline(x,y,x+w,y,clBtnShadow,alpha);
drawline(x,y,x,y+h,clBtnShadow,alpha);

drawline(x,y+h,x+w,y+h,clBtnHighlight,alpha);
drawline(x+w,y,x+w,y+h+1,clBtnHighlight,alpha);

} else if (state==8)
{
fillrect(x,y,x+w,y+h,clBtnFace,alpha);

drawline(x-1,y,x+w,y,clBtnHighlight,alpha);
drawline(x,y,x,y+h,clBtnHighlight,alpha);

drawline(x,y+h-1,x+w,y+h-1,clBtnShadow,alpha);
drawline(x+w-1,y+1,x+w-1,y+h,clBtnShadow,alpha);

drawline(x-1,y+h,x+w,y+h,cl3DDkShadow,alpha);
drawline(x+w-1,y,x+w,y+h,cl3DDkShadow,alpha);

} else if (state==9)
{
fillrect(x,y,x+w,y+h,clBtnFace,alpha);

drawline(x-1,y,x+w-1,y,cl3DDkShadow,alpha);
drawline(x,y,x,y+h-1,cl3DDkShadow,alpha);

drawline(x+1,y+1,x+w-2,y+1,clBtnShadow,alpha);
drawline(x+1,y+1,x+1,y+h-2,clBtnShadow,alpha);

drawline(x-1,y+h,x+w-1,y+h-1,clBtnHighlight,alpha);
drawline(x+w-1,y,x+w,y+h,clBtnHighlight,alpha);

} else if (state==10)
{
fillrect(x,y,x+w,y+h,clBtnFace,alpha);
} else if (state==11)
{
sx = x;
sy = y;
sw = w-1;
sh = h+1;
drawcontroledgew(sx+1,sy+1,sw-1,sh-1,0,alpha);
}else if (state==12)
{
sx = x;
sy = y;
sw = w-1;
sh = h+1;
fillrect(sx,sy,sx+sw,sy+sh,clBtnFace,alpha);
rectangle(sx,sy,sx+sw,sy+sh,clBtnShadow,alpha);
} else if (state==13)
{
sx = x;
sy = y;
sw = w-1;
sh = h+1;
drawcontroledgew(sx+1,sy+1,sw-1,sh-1,0,alpha);
} else if (state==14)
{
sx = x;
sy = y;
sw = w-1;
sh = h+1;
drawcontroledgew(sx+1,sy+1,sw-1,sh-1,1,alpha);
}
//enddrawing();
}

void drawcontroledgew(int x,int y,int w,int h, int state, int alpha)
{
//begindrawing();
if (state==0)
{
fillrect(x,y,x+w,y+h,clBtnFace,alpha);

drawline(x+1,y+1,x+w-1,y+1,clBtnHighlight,alpha);
drawline(x+1,y+1,x+1,y+h-1,clBtnHighlight,alpha);

drawline(x+1,y+h-1,x+w-1,y+h-1,clBtnShadow,alpha);
drawline(x+w-1,y+1,x+w-1,y+h,clBtnShadow,alpha);

drawline(x,y+h,x+w,y+h,cl3DDkShadow,alpha);
drawline(x+w,y,x+w,y+h+1,cl3DDkShadow,alpha);
} else if (state==1)
{
fillrect(x-2,y-2,x+w,y+h,clBtnFace,alpha);

drawline(x-2,y-1,x+w-1,y-1,clBtnShadow,alpha);
drawline(x-1,y-2,x-1,y+h-1,clBtnShadow,alpha);

drawline(x-1,y-1,x+w,y-1,cl3DDkShadow,alpha);
drawline(x,y-1,x,y+h,cl3DDkShadow,alpha);

drawline(x-1,y+h-2,x+w,y+h-1,clBtnFace,alpha);
drawline(x+w-2,y-1,x+w-1,y+h,clBtnFace,alpha);

drawline(x-2,y+h-1,x+w,y+h,clBtnHighlight,alpha);
drawline(x+w-1,y-2,x+w,y+h,clBtnHighlight,alpha);
}
//enddrawing();
}


COLORREF clLightifDarkR(COLORREF c)
{
if (GetRValue(c)<64) return (GetRValue(c)+128); else
return (GetRValue(c)|(GetRValue(c)/2));
}

COLORREF clLightifDarkG(COLORREF c)
{
if (GetGValue(c)<64) return (GetGValue(c)+128); else
return (GetGValue(c)|(GetGValue(c)/2));
}

COLORREF clLightifDarkB(COLORREF c)
{
if (GetBValue(c)<64) return (GetBValue(c)+128); else
return (GetBValue(c)|(GetBValue(c)/2));
}


void drawgradient(int x0, int y0, int x1, int y1, int horizontal, uint32_t source, uint32_t dest, int alpha)
{
  int x,y,z,l,nc,f2,fc,m;
  float f=0,fx=0,f4=0;
  uint8_t b[2][3];
  uint8_t a[3];
  nc = 2;
  if (nc > 0) {
        if (horizontal) {
          m = x1 - x0;
        }
        else
        {
          m = y1 - y0;
        }
        m *= 2;
        b[0][0] = GetRValue(source);
        b[0][1] = GetGValue(source);
        b[0][2] = GetBValue(source);
        b[1][0] = GetRValue(dest);
        b[1][1] = GetGValue(dest);
        b[1][2] = GetBValue(dest);
        fc =((float)m/(float)nc);
        for (y = 0; y < nc-1; y++) {
           if (y == nc-1) {
                 f2 =  m - y * fc - 1;
           }
           else
           {
                 f2 = fc;
           }

        for (x = 0; x < f2; x++) {
           l = x+y*fc;
           fx = x;
           f4 = f2;
           f = (fx/f4);
           for (z = 0; z < 3; z++) {
                 a[z] = (int)((b[y][z]+ (b[y + 1][z] - b[y][z])*f));
           }
           if (horizontal) {
                  drawline(x0+l,y0,x0+l,y1,RGB(a[0],a[1],a[2]),alpha);
           }
           else
           {
                  drawline(x0, y0+l, x1, y0+l,RGB(a[0],a[1],a[2]),alpha);
           }
        }
   }
  }
}

/*
unsigned char kbhit(void)
{
	unsigned char hit = 0;
	outb(0x60, 0xA7);
	unsigned char key = inb(0x60);
	unsigned char keyt = inb(0x61);
	outb(0x61, keyt|0x80);
	outb(0x61, keyt&0x7F);
	if ((key & 0x80) == 0)
	{
		hit = 1;
	}
	outb(0x60, 0xA8);
	outb(0x20, 0x20);
	return (hit);
}
*/

typedef struct tagBITMAP {
        long    bmType;
        long    bmWidth;
        long    bmHeight;
        long    bmWidthBytes;
        uint16_t        bmPlanes;
        uint16_t        bmBitsPixel;
        void*   bmBits;
} BITMAP,*PBITMAP,*LPBITMAP;
typedef struct tagBITMAPCOREHEADER {
        uint32_t        bcSize;
        uint16_t        bcWidth;
        uint16_t        bcHeight;
        uint16_t        bcPlanes;
        uint16_t        bcBitCount;
} BITMAPCOREHEADER,*LPBITMAPCOREHEADER,*PBITMAPCOREHEADER;
#pragma pack(push,1)
typedef struct tagRGBTRIPLE {
        uint8_t rgbtBlue;
        uint8_t rgbtGreen;
        uint8_t rgbtRed;
} RGBTRIPLE,*LPRGBTRIPLE;
#pragma pack(pop)
#pragma pack(push,2)
typedef struct tagBITMAPFILEHEADER {
        uint16_t        bfType;
        uint32_t        bfSize;
        uint16_t        bfReserved1;
        uint16_t        bfReserved2;
        uint32_t        bfOffBits;
} BITMAPFILEHEADER,*LPBITMAPFILEHEADER,*PBITMAPFILEHEADER;
#pragma pack(pop)
typedef struct _BITMAPCOREINFO {
        BITMAPCOREHEADER        bmciHeader;
        RGBTRIPLE       bmciColors[1];
} BITMAPCOREINFO,*LPBITMAPCOREINFO,*PBITMAPCOREINFO;
typedef struct tagBITMAPINFOHEADER{
        uint32_t        biSize;
        long    biWidth;
        long    biHeight;
        uint16_t        biPlanes;
        uint16_t        biBitCount;
        uint32_t        biCompression;
        uint32_t        biSizeImage;
        long    biXPelsPerMeter;
        long    biYPelsPerMeter;
        uint32_t        biClrUsed;
        uint32_t        biClrImportant;
} BITMAPINFOHEADER,*LPBITMAPINFOHEADER,*PBITMAPINFOHEADER;
typedef struct tagRGBQUAD {
        uint8_t rgbBlue;
        uint8_t rgbGreen;
        uint8_t rgbRed;
        uint8_t rgbReserved;
} RGBQUAD,*LPRGBQUAD;
typedef struct tagBITMAPINFO {
        BITMAPINFOHEADER bmiHeader;
        RGBQUAD bmiColors[1];
} BITMAPINFO,*LPBITMAPINFO,*PBITMAPINFO;



typedef struct bmp
{
 BITMAPFILEHEADER       head;
 BITMAPINFO             info;
 void*                  data;
 int                    width;
 int                    height;
 int                    size;
 unsigned char*         bgr;
}BMP;


/*
int             savebmp(char* filename, BMP* bmp);
BMP*            loadbmp(char* filename);
void            freebmp(BMP* bmp);



int savebmp(char* filename, BMP* bmp)
{
 int    infosize;
 FILE*  file = NULL;
 if(!(file = fopen(filename, "wb"))) return 1;
 fwrite(&(bmp->head), sizeof(BITMAPFILEHEADER), 1, file);
 infosize = (bmp->head).bfOffBits - sizeof(BITMAPFILEHEADER);
 fwrite(&(bmp->info), 1, infosize, file);
 fwrite((unsigned char *)(bmp->data), 1, bmp->size, file);
 fclose(file);
 return 0;
}

BMP* loadbmp(char* filename)
{
 FILE*  file            = NULL;
 BMP*   bmp             = NULL;
 long   infosize;
 long   bitsize;
 if(!(file = fopen(filename, "rb"))) return NULL;
 bmp = (BMP *)malloc(sizeof(BMP));
 fread(&(bmp->head), sizeof(BITMAPFILEHEADER), 1, file);
 if((bmp->head).bfType!= 0x4d42) { fclose(file); free(bmp); return NULL; }
 infosize = (bmp->head).bfOffBits - sizeof(BITMAPFILEHEADER);
 fread(&(bmp->info), 1, infosize, file);
 if((bitsize = (bmp->info).bmiHeader.biSizeImage)==0)
  bitsize = ((bmp->info).bmiHeader.biWidth*((bmp->info).bmiHeader.biBitCount+7)/8) * abs((bmp->info).bmiHeader.biHeight);
 (bmp->data) = (void *)malloc(sizeof(char)*bitsize); 
 fread((unsigned char *)(bmp->data), 1, bitsize, file);
 fclose(file);
 bmp->size      = bitsize;
 bmp->width     = (bmp->info).bmiHeader.biWidth;
 bmp->height = (bmp->info).bmiHeader.biHeight;
 bmp->bgr = (unsigned char*)bmp->data;
 return bmp;
}

void freebmp(BMP* bmp)
{
 if(!bmp) return;
 if(bmp->data) free(bmp->data);
 free(bmp);
}
*/

void drawimagemask(int x, int y,BMP* imagefile, int alpha, COLORREF color, int transparent, COLORREF transparentcolor)
{
int cx,cy,index=0,r,g,b;
COLORREF c;

  for(cy=0;cy<imagefile->height;cy++)
  {
  for(cx=0;cx<imagefile->width;cx++)
  {
    r = imagefile->bgr[index + 2];
    g = imagefile->bgr[index + 1];
    b = imagefile->bgr[index + 0];
    c = RGB(b,g,r);
    if (transparent)
    {
      if (c!=transparentcolor) setpixel(x+cx,y+(imagefile->height-cy),color,alpha);
    }
    else
    {
      setpixel(x+cx,y+(imagefile->height-cy),color,alpha);    
    }
    index += 3;
  }
  }
  
//  enddrawing();

}

void drawimage(int x, int y,BMP* imagefile, int alpha, int transparent, COLORREF transparentcolor)
{
int cx,cy,index=0,r,g,b,rgb;
COLORREF c;

  for(cy=0;cy<imagefile->height;cy++)
  {
  for(cx=0;cx<imagefile->width;cx++)
  {
    r = imagefile->bgr[index + 2];
    g = imagefile->bgr[index + 1];
    b = imagefile->bgr[index + 0];
    c = RGB(b,g,r);
    rgb = RGB(r,g,b);
    if (transparent)
    {
      if (c!=transparentcolor) setpixel(x+cx,y+(imagefile->height-cy),rgb,alpha);
    }
    else
    {
      setpixel(x+cx,y+(imagefile->height-cy),rgb,alpha);    
    }
    index += 3;
  }
  }
  
//  enddrawing();

}

#define ARROW_HEIGHT 32
#define ARROW_WIDTH 32

// array size is 1024
static const uint8_t arrow[]  = {
  0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0x00, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0x00, 0xff, 0x03, 0x03, 0xff, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0xff, 0x03, 0x03, 0x03, 0xff, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0x00, 0x00, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03
};

void drawcursor(int x, int y)
{
	for(int yy=0;yy<ARROW_HEIGHT;yy++)
	{
		for(int xx=0;xx<ARROW_WIDTH;xx++)
		{
			uint32_t p = arrow[xyoffset(xx,yy,ARROW_WIDTH)];
			uint8_t r = 0;
			uint8_t g = 0;
			uint8_t b = 0;			
			uint8_t t = 0;
			switch(p)
			{
				case 0:
				{
					r = g = b = 0;
				}
				break;
				case 3:
				{
					r = 0; g = 0; b = 255;
					t = 1;
				}
				break;
				case 255:
				{
					r = g = b = 255;
				}
				break;
			}
			uint32_t c = UINT32(r,g,b,0);
			if (t == 0) setpixel(x+xx, y+yy, c, 255);
		}
	}
}

void getmouse(int *x, int *y, int *b)
{
	int m_x = 0;
	int m_y = 0;
	int m_b = 0;
	__asm__ volatile ( "int $0x80" : "=c"(m_x),"=d"(m_y),"=b"(m_b) : "a" (512), "b" (0), "c" (0), "d" (0) );
	*x = m_x;
	*y = m_y;
	*b = m_b;
}

int initmouse()
{
	int r=0;
	//mouse_install
	//__asm__ volatile ( "int $0x80" : "=a"(r) : "a" (508), "b" (0), "c" (0), "d" (0) );
	__asm__ volatile ( "int $0x80" : "=a"(r) : "a" (513), "b" (0), "c" (0), "d" (0) );
	return r;
}

void uninitmouse()
{
	//mouse_uninstall
	__asm__ volatile ( "int $0x80" : : "a" (514), "b" (0), "c" (0), "d" (0) );
}

void setmouseresrange(int screen_width, int screen_height)
{
	//mouse_uninstall
	__asm__ volatile ( "int $0x80" : : "a" (515), "b" (0), "c" (screen_width), "d" (screen_height) );
}

bool in_area(int px, int py, int x, int y, int w, int h)
{
	if (((px >= x) && (py >= y)) && ((px <= x+w) && (py <= y+h)))
	{
		return true;
	}
	return false;
}

uint8_t kbhitkey(uint8_t key)
{
	uint8_t status=0;
	__asm__ volatile ( "int $0x80" : "=a"(status) : "a" (516), "b" (0), "c" (0), "d" (key) );
	return status;
}

uint8_t kbhit(void)
{
	for(int i=0;i<256;i++)
	{
		uint8_t kb_status = kbhitkey(i);
		if (kb_status == 1)
		{
			return i;
		}
	}
	return 0;
}

char pow2char[8] = {1,2,4,8,16,32,64,128};

unsigned char textfont[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x7E, 0x81, 0xA5, 0x81, 0xBD, 0x99, 0x81, 0x7E,
  0x7E, 0xFF, 0xDB, 0xFF, 0xC3, 0xE7, 0xFF, 0x7E, 
  0x6C, 0xFE, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00,
  0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10, 0x00,
  0x38, 0x7C, 0x38, 0xFE, 0xFE, 0x92, 0x10, 0x7C,
  0x00, 0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x7C,
  0x00, 0x00, 0x18, 0x3C, 0x3C, 0x18, 0x00, 0x00, 
  0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF, 
  0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00, 
  0xFF, 0xC3, 0x99, 0xBD, 0xBD, 0x99, 0xC3, 0xFF,
  0x0F, 0x07, 0x0F, 0x7D, 0xCC, 0xCC, 0xCC, 0x78,
  0x3C, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x7E, 0x18, 
  0x3F, 0x33, 0x3F, 0x30, 0x30, 0x70, 0xF0, 0xE0, 
  0x7F, 0x63, 0x7F, 0x63, 0x63, 0x67, 0xE6, 0xC0,
  0x99, 0x5A, 0x3C, 0xE7, 0xE7, 0x3C, 0x5A, 0x99,
  0x80, 0xE0, 0xF8, 0xFE, 0xF8, 0xE0, 0x80, 0x00,
  0x02, 0x0E, 0x3E, 0xFE, 0x3E, 0x0E, 0x02, 0x00,
  0x18, 0x3C, 0x7E, 0x18, 0x18, 0x7E, 0x3C, 0x18, 
  0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00,
  0x7F, 0xDB, 0xDB, 0x7B, 0x1B, 0x1B, 0x1B, 0x00,
  0x3E, 0x63, 0x38, 0x6C, 0x6C, 0x38, 0x86, 0xFC, 
  0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E, 0x7E, 0x00,
  0x18, 0x3C, 0x7E, 0x18, 0x7E, 0x3C, 0x18, 0xFF,
  0x18, 0x3C, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x00, 
  0x18, 0x18, 0x18, 0x18, 0x7E, 0x3C, 0x18, 0x00, 
  0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00,
  0x00, 0x30, 0x60, 0xFE, 0x60, 0x30, 0x00, 0x00, 
  0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xFE, 0x00, 0x00, 
  0x00, 0x24, 0x66, 0xFF, 0x66, 0x24, 0x00, 0x00, 
  0x00, 0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x00, 0x00,
  0x00, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00,
  0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00,
  0x18, 0x7E, 0xC0, 0x7C, 0x06, 0xFC, 0x18, 0x00,
  0x00, 0xC6, 0xCC, 0x18, 0x30, 0x66, 0xC6, 0x00, 
  0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00,
  0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00,
  0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00, 
  0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00,
  0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30, 
  0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00,
  0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00,
  0x7C, 0xCE, 0xDE, 0xF6, 0xE6, 0xC6, 0x7C, 0x00,
  0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xFC, 0x00,
  0x78, 0xCC, 0x0C, 0x38, 0x60, 0xCC, 0xFC, 0x00, 
  0x78, 0xCC, 0x0C, 0x38, 0x0C, 0xCC, 0x78, 0x00,
  0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x1E, 0x00,
  0xFC, 0xC0, 0xF8, 0x0C, 0x0C, 0xCC, 0x78, 0x00, 
  0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00, 
  0xFC, 0xCC, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00,
  0x78, 0xCC, 0xCC, 0x78, 0xCC, 0xCC, 0x78, 0x00,
  0x78, 0xCC, 0xCC, 0x7C, 0x0C, 0x18, 0x70, 0x00,
  0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00,
  0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30, 
  0x18, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x00, 
  0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00, 
  0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00,
  0x3C, 0x66, 0x0C, 0x18, 0x18, 0x00, 0x18, 0x00, 
  0x7C, 0xC6, 0xDE, 0xDE, 0xDC, 0xC0, 0x7C, 0x00,
  0x30, 0x78, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00,
  0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00,
  0x3C, 0x66, 0xC0, 0xC0, 0xC0, 0x66, 0x3C, 0x00,
  0xF8, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00,
  0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00,
  0xFE, 0x62, 0x68, 0x78, 0x68, 0x60, 0xF0, 0x00, 
  0x3C, 0x66, 0xC0, 0xC0, 0xCE, 0x66, 0x3A, 0x00,
  0xCC, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0xCC, 0x00, 
  0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
  0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00,
  0xE6, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0xE6, 0x00, 
  0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00, 
  0xC6, 0xEE, 0xFE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00,
  0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00,
  0x38, 0x6C, 0xC6, 0xC6, 0xC6, 0x6C, 0x38, 0x00, 
  0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00,
  0x7C, 0xC6, 0xC6, 0xC6, 0xD6, 0x7C, 0x0E, 0x00,
  0xFC, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0xE6, 0x00, 
  0x7C, 0xC6, 0xE0, 0x78, 0x0E, 0xC6, 0x7C, 0x00,
  0xFC, 0xB4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
  0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xFC, 0x00, 
  0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00, 
  0xC6, 0xC6, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00,
  0xC6, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0xC6, 0x00,
  0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x30, 0x78, 0x00, 
  0xFE, 0xC6, 0x8C, 0x18, 0x32, 0x66, 0xFE, 0x00,
  0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00, 
  0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00,
  0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00, 
  0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 
  0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00,
  0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66, 0xDC, 0x00,
  0x00, 0x00, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00,
  0x1C, 0x0C, 0x0C, 0x7C, 0xCC, 0xCC, 0x76, 0x00,
  0x00, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00, 
  0x38, 0x6C, 0x64, 0xF0, 0x60, 0x60, 0xF0, 0x00, 
  0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8,
  0xE0, 0x60, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00,
  0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00, 
  0x0C, 0x00, 0x1C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78,
  0xE0, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0xE6, 0x00,
  0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
  0x00, 0x00, 0xCC, 0xFE, 0xFE, 0xD6, 0xD6, 0x00, 
  0x00, 0x00, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0x00,
  0x00, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0x78, 0x00,
  0x00, 0x00, 0xDC, 0x66, 0x66, 0x7C, 0x60, 0xF0,
  0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0x1E,
  0x00, 0x00, 0xDC, 0x76, 0x62, 0x60, 0xF0, 0x00, 
  0x00, 0x00, 0x7C, 0xC0, 0x70, 0x1C, 0xF8, 0x00,
  0x10, 0x30, 0xFC, 0x30, 0x30, 0x34, 0x18, 0x00,
  0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 
  0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00,
  0x00, 0x00, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00,
  0x00, 0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00, 
  0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8, 
  0x00, 0x00, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00,
  0x1C, 0x30, 0x30, 0xE0, 0x30, 0x30, 0x1C, 0x00,
  0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00,
  0xE0, 0x30, 0x30, 0x1C, 0x30, 0x30, 0xE0, 0x00, 
  0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0x00
};

void printextrgba(long xpos, long ypos, COLORREF fontcolor, short fontalpha, COLORREF backcolor, short backalpha, char *name)
{
	long stx, i, x, y, charxsiz;
	char *fontptr, *letptr;

	stx = xpos;

	fontptr = (char*)textfont; charxsiz = 8;

	long xx, yy;
	for(i=0;name[i];i++) {
		letptr = &fontptr[name[i]<<3];
		xx = stx;
		yy = ypos+7 + 2; //+1 is hack!
		for(y=7;y>=0;y--) {
			for(x=charxsiz-1;x>=0;x--) {
				if (letptr[y]&pow2char[7-x])
					setpixel(xx+x,yy,fontcolor,fontalpha);
				else if (backcolor != 0)
					setpixel(xx+x,yy,backcolor,backalpha);
			}
			yy--;
		}
		stx += charxsiz;
	}

}

int main() 
{
	int x1 = 100;
	uint8_t pending_quit = 0;
	uint8_t quit = 0;
	int mouse_x = 0;
	int mouse_y = 0;
	int mouse_b = 0;
	uint8_t kb_hit = 0;
	uint16_t old_video_mode = getvideomode();
	setvideomode(0x112);
	initmouse();
	setmouseresrange(v_mode->width, v_mode->height);
	double_buffer = (unsigned char*)malloc(v_mode->width*v_mode->height*4);
	frame_buffer = (unsigned char*)malloc(v_mode->width*v_mode->height*4);
	fillrect(0, 0, v_mode->width, v_mode->height, RGB(0, 192, 255), 255);
	
	drawcontroledge(100,200,64,64,0,255);
	drawcontroledgew(280,200,64,64,0,255);
	printextrgba(128, 227, clBlack, 255, clBlack, 0, "X");
	//printextrgba(long xpos, long ypos, COLORREF fontcolor, short fontalpha, COLORREF backcolor, short backalpha, char *name);
	
	enable_interrupt();
	while(quit == 0)
	{
		restorebuffer();		
		
		if (in_area(mouse_x,mouse_y,100,200,64,64))
		{
			if (mouse_b & 1) 
			{
				drawcontroledge(100,200,64,64,1,255);
				printextrgba(129, 228, clBlack, 255, clBlack, 0, "X");
				pending_quit = 1;
			}
			else
			{
				drawcontroledge(100,200,64,64,0,255);
				printextrgba(128, 227, clBlack, 255, clBlack, 0, "X");
				if (pending_quit == 1) quit = 1;
			}
		}
		else 
		{
			drawcontroledge(100,200,64,64,0,255);
			printextrgba(128, 227, clBlack, 255, clBlack, 0, "X");
			pending_quit = 0;
		}
		
		fillrect(x1,300,x1+40,315,RGB(255,0,0),255);
		
		x1++;
		if (x1 > 300) x1 = 100;
		
		getmouse(&mouse_x, &mouse_y, &mouse_b);
		
		drawcursor(mouse_x, mouse_y);
		
		swapbuffers();
		kb_hit = kbhit();
		if (kb_hit) quit = 1;
	}
	setvideomode(old_video_mode);
	/*
	uint32_t video_mode = getvideomode();
	printf("Video Mode: 0x%X\n", video_mode);
	unsigned char ch1 = getch();
	printf("get char: 0x%X\n", ch1);
	char str[256];
	memset(str, 0, 256);
	gets(str);
	printf("get string: %s\n", str);
	*/
	
	free(frame_buffer);
	free(double_buffer);
	uninitmouse();
		
    return 0;
}


