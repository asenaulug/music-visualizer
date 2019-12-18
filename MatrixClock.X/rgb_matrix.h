/* 
 * File:   rgb_matrix.h
 * Author: smiller
 *
 * Created on November 19, 2015, 10:14 AM
 */

#ifndef RGB_MATRIX_H
#define	RGB_MATRIX_H

#define _SUPPRESS_PLIB_WARNING 1
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING 1
#include "plib.h"

#define MATRIX_HEIGHT 16
#define MATRIX_WIDTH 32

#define MATRIX_TABSPACE 4

#define MATRIX_NPLANES 3
#define MATRIX_NROWS 8
#define MATRIX_A_PORTB_BIT BIT_3
#define MATRIX_B_PORTB_BIT BIT_5
#define MATRIX_C_PORTB_BIT BIT_14
#define MATRIX_CLK_PORTA_BIT BIT_0
#define MATRIX_LAT_PORTA_BIT BIT_1
#define MATRIX_OE_PORTA_BIT BIT_1

UINT16 _matrix_width, _matrix_height;
UINT8 matrix_rotation;

#define swap(a, b) { short t = a; a = b; b = t; }
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_word(addr) (*(const unsigned short *)(addr))

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

void matrix_init(BOOL dualbuffers);
void matrix_drawPixel(UINT16 x, UINT16 y, UINT16 c);
void matrix_fillScreen(UINT16 c);
void matrix_swapBuffers(BOOL copy);
void matrix_dumpMatrix();

UINT16 *matrix_backBuffer();

UINT16 matrix_colorHSV(long hue, UINT8 sat, UINT8 val, BOOL gflag);
UINT16 matrix_color333(UINT8 r, UINT8 g, UINT8 b);
UINT16 matrix_color444(UINT8 r, UINT8 g, UINT8 b);
UINT16 matrix_color888(UINT8 r, UINT8 g, UINT8 b, BOOL gflag);

void matrix_setRotation(unsigned char x);
void matrix_drawFastVLine(short x, short y, short h, unsigned short color);
void matrix_drawFastHLine(short x, short y, short w, unsigned short color);

#endif	/* RGB_MATRIX_H */
