#ifndef MATRIX_GFX_H
#define	MATRIX_GFX_H

#define _SUPPRESS_PLIB_WARNING 1
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING 1
#include "plib.h"

// Color definitions
#define COLOR565_BLACK   0x0000
#define COLOR565_BLUE    0x001F
#define COLOR565_ICE     (3 + (63 << 5) + (0 << 11))
#define COLOR565_RED     0xF800
#define COLOR565_ORANGE  (0 + (4 << 5) + (31 << 11))
#define COLOR565_GREEN   0x0CE0
#define COLOR565_LIGHT_GREEN  (0 + (63 << 5) + (4 << 11))
#define COLOR565_CYAN    0x07FF 
#define COLOR565_MAGENTA 0xF81F
#define COLOR565_LIPSTICK (10 + (5 << 5) + (31 << 11))
#define COLOR565_PINK     (3 + (0 << 5) + (15 << 11))
#define COLOR565_PURPLE  (31 + (0 << 5) + (4 << 11)) 
#define COLOR565_YELLOW  0xFFE0
#define COLOR565_MUSTARD (0 + (4 << 5) + (5 << 11))
#define COLOR565_NICE_BLUE (31 + (5 << 5) + (0 << 11))
#define COLOR565_PERIWINKLE (31 + (5 << 5) + (5 << 11))
#define COLOR565_WHITE   0xFFFF

extern unsigned short
  matrix_cursor_y, matrix_cursor_x,
  matrix_textsize, matrix_textcolor, matrix_textbgcolor,
  matrix_wrap;

void matrix_drawLine(short x0, short y0, short x1, short y1, unsigned short color);
void matrix_drawRect(short x, short y, short w, short h, unsigned short color);
void matrix_fillRect(short x, short y, short w, short h, unsigned short color);
void matrix_drawRoundRect(short x0, short y0, short w, short h,
      short radius, unsigned short color);
void matrix_fillRoundRect(short x0, short y0, short w, short h, short radius, unsigned short color);

void matrix_drawCircle(short x0, short y0, short r, unsigned short color);
void matrix_drawCircleHelper(short x0, short y0, short r, unsigned char cornername,
      unsigned short color);
void matrix_fillCircle(short x0, short y0, short r, unsigned short color);
void matrix_fillCircleHelper(short x0, short y0, short r, unsigned char cornername,
      short delta, unsigned short color);

void matrix_drawTriangle(short x0, short y0, short x1, short y1,
      short x2, short y2, unsigned short color);
void matrix_fillTriangle(short x0, short y0, short x1, short y1,
      short x2, short y2, unsigned short color);

void matrix_drawBitmap(short x, short y, const unsigned char *bitmap, short w, short h, unsigned short color);

void matrix_drawChar(short x, short y, unsigned char c, unsigned short color, unsigned short bg, unsigned char size);
void matrix_draw3x5Char(short x, short y, unsigned char c, unsigned short color, unsigned short bg, unsigned char size);
void matrix_setCursor(short x, short y);
void matrix_write(unsigned char c);
void matrix_writeString(const char* str);    // This is the function to use to write a string
void matrix_write3x5(unsigned char c);
void matrix_write3x5String(const char* str);



void matrix_setTextColor(unsigned short c);
void matrix_setTextColor2(unsigned short c, unsigned short bg);
void matrix_setTextSize(unsigned char s);
void matrix_setTextWrap(char w);

#endif /* MATRIX_GFX_H */
