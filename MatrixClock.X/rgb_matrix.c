#include "rgb_matrix.h"

#define _SUPPRESS_PLIB_WARNING 1
//#include <plib.h>
//#include <stdlib.h>
#include <xc.h>

#include "gamma.h"


/************************* Variable declarations ******************************/

UINT16 *matrixbuff[2];

volatile BOOL swapflag;
volatile UINT8 backindex;
volatile INT16 *buffptr;
volatile int row, plane;

// Expects that the System is configured at 40MHz with pbclk also at 40MHz
void matrix_init(BOOL dualbuffers) {
    int buffsize = MATRIX_WIDTH * MATRIX_NROWS * 3;
    int allocsize = (dualbuffers) ? buffsize * 2 : buffsize;
    if (NULL == (matrixbuff[0] = (INT16 *)malloc(allocsize * 2))) return;
    memset(matrixbuff[0], 0, allocsize);
    matrixbuff[1] = (dualbuffers) ? &matrixbuff[0][buffsize] : matrixbuff[0];
    
    backindex = 0;
    buffptr = matrixbuff[1-backindex];
    
    plane = MATRIX_NPLANES - 1;
    row = MATRIX_NROWS - 1;
    _matrix_height = MATRIX_HEIGHT;
    _matrix_width = MATRIX_WIDTH;
    
    mPORTASetPinsDigitalOut(MATRIX_CLK_PORTA_BIT
                            |MATRIX_OE_PORTA_BIT
                            |MATRIX_LAT_PORTA_BIT);
    
    mPORTBClearBits(MATRIX_A_PORTB_BIT|MATRIX_B_PORTB_BIT
                   |MATRIX_C_PORTB_BIT);
    mPORTBSetPinsDigitalOut(MATRIX_A_PORTB_BIT|MATRIX_B_PORTB_BIT
                           |MATRIX_C_PORTB_BIT);
    
    //   9,8,7  &  2,1,0
    // (0011|1000|0111)
    mPORTBClearBits(0x387);
    mPORTBSetPinsDigitalOut(0x387);
                    
    // Open timer 2 with prescalar 1 and max value
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_1, 100);
    // Setup interrupts to handle overflow
    ConfigIntTimer2( T2_INT_ON | T2_INT_PRIOR_5 );
    mT2ClearIntFlag();
}

void matrix_swapBuffers(BOOL copy) {
    if (matrixbuff[0] != matrixbuff[1]) {
        // To avoid 'tearing' display, actual swap takes place in the interrupt
        // handler, at the end of a complete screen refresh cycle.
        swapflag = 1;                  // Set flag here, then...
        while(swapflag) _nop(); // wait for interrupt to clear it
        if(copy) {
            memcpy(matrixbuff[backindex], 
                   matrixbuff[1-backindex], 
                   MATRIX_WIDTH * MATRIX_NROWS * 3 * 2);
        }
    }
}

UINT16 *matrix_backBuffer() {
    return matrixbuff[backindex];
}

// @ 40MHz SYSCLK and PBCLK
// 137 us (per line, roughly 450 Hz frame rate)
// 4 interrupts (4*0.675us = 2.7us)
// 11.6 + 8.8 + 8.0 + 8.0 = 36.4us
// Total = 39.1us
// CPU load = 39.1 / 137 = 0.285  
#define CALLOVERHEAD 30 // actual measured 27
#define LOOPTIME 360 // actual measured 292

void __ISR(_TIMER_2_VECTOR, IPL5AUTO) matrix_updateDisplay(void) {
    UINT16 *ptr;
    UINT16 *end_ptr;
    UINT16 i, duration;
    
    mPORTASetBits(MATRIX_OE_PORTA_BIT|MATRIX_LAT_PORTA_BIT);  
    
    duration = ((LOOPTIME + (CALLOVERHEAD * 2)) << plane) - CALLOVERHEAD;
    
    
    if (++plane >= MATRIX_NPLANES) {
        plane = 0;
        
        if (++row >= MATRIX_NROWS) {
            row = 0;
            if (swapflag) {
                backindex = 1 - backindex;
                swapflag = 0;
            }
            buffptr = matrixbuff[1-backindex];
        }
    }
    else if (plane == 1) {
        mPORTBClearBits(MATRIX_A_PORTB_BIT|MATRIX_B_PORTB_BIT|MATRIX_C_PORTB_BIT);
        if (row & BIT_0) mPORTBSetBits(MATRIX_A_PORTB_BIT);
        if (row & BIT_1) mPORTBSetBits(MATRIX_B_PORTB_BIT);
        if (row & BIT_2) mPORTBSetBits(MATRIX_C_PORTB_BIT);
        
    }
    
    ptr = (UINT16 *)buffptr;
    end_ptr = ptr + 32;
    
    WritePeriod2(duration);
    WriteTimer2(0);
    mPORTAClearBits(MATRIX_OE_PORTA_BIT|MATRIX_LAT_PORTA_BIT);
    
    if (plane > 0) {
        for (; ptr< end_ptr; ptr++) {
            mPORTBClearBits(0x387);
            mPORTBSetBits( *ptr & 0x387 );
            _nop(); _nop();
            mPORTASetBits(MATRIX_CLK_PORTA_BIT);
            mPORTAClearBits(MATRIX_CLK_PORTA_BIT);
        }
        
        buffptr = ptr;
    }
    else {
        for (i=0; i < MATRIX_WIDTH; i++) {
            mPORTBClearBits(0x387);
            mPORTBSetBits( ((ptr[i] >> 6) & 0x380) | ((ptr[i] >> 4) & 0x7) );
            _nop(); _nop();
            mPORTASetBits(MATRIX_CLK_PORTA_BIT);
            mPORTAClearBits(MATRIX_CLK_PORTA_BIT);
        }
    }
     
    mT2ClearIntFlag();
}

void matrix_setRotation(unsigned char x) {
/* Set display matrix_rotation in 90 degree steps
 * Parameters:
 *      x: dictate direction of matrix_rotation
 *          0 = no matrix_rotation (0 degree matrix_rotation)
 *          1 = rotate 90 degree clockwise
 *          2 = rotate 180 degree
 *          3 = rotate 90 degree anticlockwise
 * Returns: Nothing
 */
    matrix_rotation = (x & 3);
    switch(matrix_rotation) {
        case 0:
        case 2:
            _matrix_width  = MATRIX_HEIGHT;
            _matrix_height = MATRIX_WIDTH;
            break;
        case 1:
        case 3:
            _matrix_width  = MATRIX_HEIGHT;
            _matrix_height = MATRIX_WIDTH;
            break;
    }
}

void matrix_drawPixel(UINT16 x, UINT16 y, UINT16 c) {
    UINT8 r, g, b, curr_bit, limit;
    UINT16 *ptr;
    
    if ((x < 0) || (x >= _matrix_width) || (y < 0) || (y >= _matrix_height)) {
        return;
    }
    
    switch (matrix_rotation) {
        case 1:
            swap(x, y);
            x = MATRIX_WIDTH  - 1 - x;
            break;
        case 2:
            x = MATRIX_WIDTH  - 1 - x;
            y = MATRIX_HEIGHT - 1 - y;
            break;
        case 3:
            swap(x, y);
            y = MATRIX_HEIGHT - 1 - y;
            break;
   }
    
    // Adafruit_GFX uses 16-bit color in 5/6/5 format, while matrix needs
    // 4/4/4.  Pluck out relevant bits while separating into R,G,B:
    r =  c >> 12;        // RRRRrggggggbbbbb
    g = (c >>  7) & 0xF; // rrrrrGGGGggbbbbb
    b = (c >>  1) & 0xF; // rrrrrggggggBBBBb

    // Loop counter stuff
    curr_bit   = 2;
    limit = 1 << MATRIX_NPLANES;

    if (y < MATRIX_NROWS) {
        // Data for the upper half of the display is stored in the lower
        // bits of each byte.
        ptr = &matrixbuff[backindex][y * MATRIX_WIDTH * (MATRIX_NPLANES - 1) + x]; // Base addr
        // Plane 0 is a tricky case -- its data is spread about,
        // stored in least two bits not used by the other planes.
        *ptr &= ~0x70;           // Plane 0 R,G mask out in one op
        if (r & 1) *ptr |=  0x10; // Plane 0 R: 64 bytes ahead, bit 0
        if (g & 1) *ptr |=  0x20; // Plane 0 G: 64 bytes ahead, bit 1
        if (b & 1) *ptr |=  0x40; // Plane 0 B: 32 bytes ahead, bit 0
        // The remaining three image planes are more normal-ish.
        // Data is stored in the high 6 bits so it can be quickly
        // copied to the DATAPORT register w/6 output lines.
        for (; curr_bit < limit; curr_bit <<= 1) {
            *ptr &= ~0x7;            // Mask out R,G,B in one op
            if(r & curr_bit) *ptr |= 0x1; // Plane N R: bit 2
            if(g & curr_bit) *ptr |= 0x2; // Plane N G: bit 3
            if(b & curr_bit) *ptr |= 0x4; // Plane N B: bit 4
            ptr  += MATRIX_WIDTH;                 // Advance to next bit plane
        }
    }
    else {
        // Data for the lower half of the display is stored in the upper
        // bits, except for the plane 0 stuff, using 2 least bits.
        ptr = &matrixbuff[backindex][(y - MATRIX_NROWS) * MATRIX_WIDTH * (MATRIX_NPLANES - 1) + x];
        *ptr &= ~0xe000;           // Plane 0 R,G,B mask out in one op
        if(r & 1) *ptr |=  0x2000; // Plane 0 R: 
        if(g & 1) *ptr |=  0x4000; // Plane 0 G:
        if(b & 1) *ptr |=  0x8000; // Plane 0 B:
        for(; curr_bit < limit; curr_bit <<= 1) {
            *ptr &= ~0x380;            // Mask out R,G,B in one op
            if(r & curr_bit) *ptr |= 0x080; // Plane N R: bit 5
            if(g & curr_bit) *ptr |= 0x100; // Plane N G: bit 6
            if(b & curr_bit) *ptr |= 0x200; // Plane N B: bit 7
            ptr  += MATRIX_WIDTH;                 // Advance to next bit plane
        }
    }  
}

// Promote 3/3/3 RGB to Adafruit_GFX 5/6/5
UINT16 matrix_color333(UINT8 r, UINT8 g, UINT8 b) {
  // RRRrrGGGgggBBBbb
  return ((r & 0x7) << 13) | ((r & 0x6) << 10) |
         ((g & 0x7) <<  8) | ((g & 0x7) <<  5) |
         ((b & 0x7) <<  2) | ((b & 0x6) >>  1);
}

// Promote 4/4/4 RGB to Adafruit_GFX 5/6/5
UINT16 matrix_color444(UINT8 r, UINT8 g, UINT8 b) {
  // RRRRrGGGGggBBBBb
  return ((r & 0xF) << 12) | ((r & 0x8) << 8) |
         ((g & 0xF) <<  7) | ((g & 0xC) << 3) |
         ((b & 0xF) <<  1) | ((b & 0x8) >> 3);
}

// Demote 8/8/8 to Adafruit_GFX 5/6/5
UINT16 matrix_color888(UINT8 r, UINT8 g, UINT8 b, BOOL gflag) {
  if(gflag) { // Gamma-corrected color?
    r = gamma[r]; // Gamma correction table maps
    g = gamma[g]; // 8-bit input to 4-bit output
    b = gamma[b];
    return ((UINT16)r << 12) | ((UINT16)(r & 0x8) << 8) | // 4/4/4->5/6/5
           ((UINT16)g <<  7) | ((UINT16)(g & 0xC) << 3) |
           (          b <<  1) | (           b        >> 3);
  } // else linear (uncorrected) color
  return ((UINT16)(r & 0xF8) << 8) | ((UINT16)(g & 0xFC) << 3) | (b >> 3);
}

UINT16 matrix_colorHSV(long hue, UINT8 sat, UINT8 val, BOOL gflag) {
  UINT8  r, g, b, lo;
  UINT16 s1, v1;

  // Hue
  hue %= 1536;              // -1535 to +1535
  if (hue < 0) hue += 1536; //     0 to +1535
  lo = hue & 255;           // Low byte  = primary/secondary color mix
  switch (hue >> 8) {        // High byte = sextant of colorwheel
    case 0 : r = 255     ; g =  lo     ; b =   0     ; break; // R to Y
    case 1 : r = 255 - lo; g = 255     ; b =   0     ; break; // Y to G
    case 2 : r =   0     ; g = 255     ; b =  lo     ; break; // G to C
    case 3 : r =   0     ; g = 255 - lo; b = 255     ; break; // C to B
    case 4 : r =  lo     ; g =   0     ; b = 255     ; break; // B to M
    default: r = 255     ; g =   0     ; b = 255 - lo; break; // M to R
  }

  // Saturation: add 1 so range is 1 to 256, allowig a quick shift operation
  // on the result rather than a costly divide, while the type upgrade to int
  // avoids repeated type conversions in both directions.
  s1 = sat + 1;
  r  = 255 - (((255 - r) * s1) >> 8);
  g  = 255 - (((255 - g) * s1) >> 8);
  b  = 255 - (((255 - b) * s1) >> 8);

  // Value (brightness) & 16-bit color reduction: similar to above, add 1
  // to allow shifts, and upgrade to int makes other conversions implicit.
  v1 = val + 1;
  if (gflag) { // Gamma-corrected color?
    r = gamma[(r * v1) >> 8]; // Gamma correction table maps
    g = gamma[(g * v1) >> 8]; // 8-bit input to 4-bit output
    b = gamma[(b * v1) >> 8];
  }
  else { // linear (uncorrected) color
    r = (r * v1) >> 12; // 4-bit results
    g = (g * v1) >> 12;
    b = (b * v1) >> 12;
  }
  return (r << 12) | ((r & 0x8) << 8) | // 4/4/4 -> 5/6/5
         (g <<  7) | ((g & 0xC) << 3) |
         (b <<  1) | ( b        >> 3);
}

/******************************************************************************
 * Drawing
 ******************************************************************************/

void matrix_fillScreen(UINT16 c) {
  if((c == 0x0000) || (c == 0xffff)) {
    // For black or white, all bits in frame buffer will be identically
    // set or unset (regardless of weird bit packing), so it's OK to just
    // quickly memset the whole thing:
    memset(matrixbuff[backindex], c, MATRIX_WIDTH * MATRIX_NROWS * 3 * 2);
  }
  else {
    // Otherwise, need to handle it the long way:
    matrix_fillRect(0, 0, _matrix_width, _matrix_height, c);
  }
}

inline unsigned char matrix_getRotation(void) {
/* Returns current rotation of screen
 *          0 = no matrix_rotation (0 degree matrix_rotation)
 *          1 = rotate 90 degree clockwise
 *          2 = rotate 180 degree
 *          3 = rotate 90 degree anticlockwise
 */
  return matrix_rotation;
}

void matrix_drawFastVLine(INT16 x, INT16 y, INT16 h, UINT16 c) {
    if (x >= _matrix_width || y >= _matrix_height) return;
    
    if ((y+h-1) >= _matrix_width) {
        h = _matrix_height-y;
    }
        
    while (h--) {
        matrix_drawPixel(x, y+h, c);
    }
}

void matrix_drawFastHLine(INT16 x, INT16 y, INT16 w, UINT16 c) {
    if (x >= _matrix_width || y >= _matrix_height) return;
      
    if ((x+w-1) >= _matrix_width) {
        w = _matrix_width-x;
    }
    
    while (w--) {
        matrix_drawPixel(x+w, y, c);
    }
}
