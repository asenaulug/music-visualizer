#include "matrix_tests.h"

#include <math.h>

#include "rgb_matrix.h"
#include "matrix_gfx.h"
#include "blocking_delay.h"

/* Fill screen with a color wheel */
void draw_colorwheel() {
    int x, y, hue;
    float dy, dx, d;
    UINT16 c;
    UINT8 sat, val;

    for (y=0; y < _matrix_width; y++) {
        dy = 15.5 - (float)y;
        for (x=0; x < _matrix_height; x++) {
            dx = 15.5 - (float)x;
            d  = dx * dx + dy * dy;
            if (d <= (16.5 * 16.5)) { // Inside the circle(ish)?
                hue = (int)((atan2(-dy, dx) + M_PI) * 1536.0 / (M_PI * 2.0));
                d = sqrt(d);
                if (d > 15.5) {
                    // Do a little pseudo anti-aliasing along perimeter
                    sat = 255;
                    val = (int)((1.0 - (d - 15.5)) * 255.0 + 0.5);
                }
                else {
                    // White at center
                    sat = (int)(d / 15.5 * 255.0 + 0.5);
                    val = 255;
                }
                c = matrix_colorHSV(hue, sat, val, true);
            }
            else {
                c = 0;
            }
            matrix_drawPixel(x, y, c);
        }
    }
}

/* Fill screen with various brightness levels of each color */
void draw_levels() {
    UINT16 i, j, r, b;

    for (i=0; i<_matrix_width; i++) {
        for (j=0; j<_matrix_height; j++) {
            r = (i % 16);
            if (j%4 == 0) {
                b = r << 12; // Red
            } else if (j%4 == 1) {
                b = r << 7; // Green
            } else if (j%4 == 2) {
                b = r << 1; // Blue
            } else {
                //b = (r << 12) | (r << 7) | (r << 1); // White
                b = (r << 12) | (r << 7) | (r << 1); // White
            }
            matrix_drawPixel(i,j,b);
        }
    }
}

const char str[]  = "Pic32 with Adafruit 32x32 RGB LED Matrix";

INT8 ball[3][4] = {
      {  3,  0,  1,  1 }, // Initial X,Y pos & velocity for 3 bouncy balls
      { 17, 15,  1, -1 },
      { 27,  4, -1,  1 }
    };

static const UINT16 ballcolor[3] = {
      0x0080, // Green=1
      0x0002, // Blue=1
      0x1000  // Red=1
    };

void scroll_test_loop() {
    while(true) {
        // Clear background
        matrix_fillScreen(0);

        draw_scroll_test_frame();

        // Update display
        matrix_swapBuffers(false);

        delay_ms(20);
    }
}


void draw_scroll_test_frame() {
    static int    size    = 4;
    static int    textX   = MATRIX_WIDTH,
                  textMin,
                  hue     = 0;
    static char i;
    
    textMin = strlen(str) * -6 * size;
    
    matrix_setTextWrap(false); // Allow text to run off right edge
    matrix_setTextSize(size);
    
        // Bounce three balls around
    for (i=0; i<3; i++) {
        // Draw 'ball'
        matrix_fillCircle(ball[i][0], ball[i][1], 5, pgm_read_word(&ballcolor[i]));
        // Update X, Y position
        ball[i][0] += ball[i][2];
        ball[i][1] += ball[i][3];
        // Bounce off edges
        if ((ball[i][0] == 0) || (ball[i][0] == (_matrix_width - 1))) {
            ball[i][2] *= -1;
        }
        if ((ball[i][1] == 0) || (ball[i][1] == (_matrix_height - 1))) {
            ball[i][3] *= -1;
        }
    }

    // Draw big scrolly text on top
    matrix_setTextColor(matrix_colorHSV(hue, 255, 255, true));
    matrix_setCursor(textX, 2);
    matrix_writeString(str);

    // Move text left (w/wrap), increase hue
    if ((--textX) < textMin) textX = _matrix_width;
    hue += 7;
    if (hue >= 1536) hue -= 1536;
}

void shapes_test_loop() {
    
    matrix_setTextSize(1);

    while (true) {
        // draw a pixel in solid white
        matrix_drawPixel(0, 0, matrix_color333(7, 7, 7));
        matrix_swapBuffers(false);
        matrix_fillScreen(COLOR565_BLACK);
        delay_ms(1000);

        // fix the screen with green
        matrix_fillRect(0, 0, 32, 32, matrix_color333(0, 7, 0));
        matrix_swapBuffers(false);
        matrix_fillScreen(COLOR565_BLACK);
        delay_ms(1000);

        // draw a box in yellow
        matrix_drawRect(0, 0, 32, 32, matrix_color333(7, 7, 0));
        matrix_swapBuffers(false);
        matrix_fillScreen(COLOR565_BLACK);
        delay_ms(1000);

        // draw an 'X' in red
        matrix_drawLine(0, 0, 31, 31, matrix_color333(7, 0, 0));
        matrix_drawLine(31, 0, 0, 31, matrix_color333(7, 0, 0));
        matrix_swapBuffers(false);
        matrix_fillScreen(COLOR565_BLACK);
        delay_ms(1000);

        // draw a blue circle
        matrix_drawCircle(10, 10, 10, matrix_color333(0, 0, 7));
        matrix_swapBuffers(false);
        matrix_fillScreen(COLOR565_BLACK);
        delay_ms(1000);

        // fill a violet circle
        matrix_fillCircle(21, 21, 10, matrix_color333(7, 0, 7));
        matrix_swapBuffers(false);
        matrix_fillScreen(COLOR565_BLACK);
        delay_ms(1000);

        // fill the screen with 'black'
        matrix_fillScreen(COLOR565_BLACK);
        matrix_swapBuffers(false);
        matrix_fillScreen(COLOR565_BLACK);

        // draw some text!
        matrix_setCursor(1, 0);    // start at top left, with one pixel of spacing
        matrix_setTextSize(1);     // size 1 == 8 pixels high
        matrix_setTextWrap(false); // Don't wrap at end of line - will do ourselves

        matrix_setTextColor(matrix_color333(7,7,7));
        matrix_writeString(" Ada\n");
        matrix_writeString("fruit\n");

        // print each letter with a rainbow color
        matrix_setTextColor(matrix_color333(7,0,0));
        matrix_write('3');
        matrix_setTextColor(matrix_color333(7,4,0));
        matrix_write('2');
        matrix_setTextColor(matrix_color333(7,7,0));
        matrix_write('x');
        matrix_setTextColor(matrix_color333(4,7,0));
        matrix_write('3');
        matrix_setTextColor(matrix_color333(0,7,0));
        matrix_write('2');
        matrix_write('\n');

        matrix_setTextColor(matrix_color333(0,7,7));
        matrix_write('*');
        matrix_setTextColor(matrix_color333(0,4,7));
        matrix_write('R');
        matrix_setTextColor(matrix_color333(0,0,7));
        matrix_write('G');
        matrix_setTextColor(matrix_color333(4,0,7));
        matrix_write('B');
        matrix_setTextColor(matrix_color333(7,0,4));
        matrix_write('*');
        matrix_swapBuffers(false);
        matrix_fillScreen(COLOR565_BLACK);
        delay_ms(1000);
    }
}
