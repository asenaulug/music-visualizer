#include "clock_gfx.h"
#include "rgb_matrix.h"
#include "matrix_gfx.h"
#include "datetime.h"

#define swapc(a, b) { char t = a; a = b; b = t; }

// Min/sec/hour hand end point arrays
static const char min_end_pnts[];
static char hr_end_pnts[];

// Accessing analog clock end points for 60/12 position hands
void get_end_pnt60(unsigned char idx, char* ppoint);
void get_end_pnt12(unsigned char idx, char* ppoint);

// Generic index to end point conversion function
void get_end_point(int idx, const char* end_pnts, unsigned int num_pnts, char* point);

void draw_dtime(rtccTime dec_tm, rtccDate dec_dt) {
    char time_buf[10];
    
    const char* month_str = months_long[dec_dt.mon - 1];
    
    matrix_setTextSize(1);
    
    matrix_setCursor(((8-strlen(month_str))<<1),2);
    matrix_setTextColor(matrix_color444(5,5,5));
    matrix_write3x5String(months_long[dec_dt.mon - 1]);

    matrix_setTextColor(0xffff);
    sprintf(time_buf, "%d:%02d", twentyFour2TwelveHour(dec_tm.hour), dec_tm.min);
    matrix_setCursor(((5-strlen(time_buf))*3)+1,12);
    matrix_writeString(time_buf);

    if (dec_tm.sec < 30) {
        matrix_drawLine(1,20,dec_tm.sec+1,20,matrix_color444(1,0,0));
    }
    else {
        matrix_drawLine(1,20,30,20,matrix_color444(1,0,0));
        matrix_drawLine(1,21,dec_tm.sec-29,21, matrix_color444(1,0,0));
    }

    if (++dec_tm.sec > 59) {
        dec_tm.sec = 0;
    }

    matrix_setCursor(((dec_dt.mday > 9) ? 4:6),25);
    matrix_setTextColor(matrix_color444(5,5,5));
    matrix_printf3x5("%s %d", days_short[dec_dt.wday], dec_dt.mday);
}

inline void draw_aclock_face(UINT16 circle_c, UINT16 marker_c) {
    matrix_drawCircle(16,16,16,circle_c);
    matrix_drawCircle(16,16,15,circle_c);
    matrix_drawPixel(16,1,marker_c);
    matrix_drawPixel(24,3,marker_c);
    matrix_drawPixel(29,8,marker_c);
    matrix_drawPixel(31,16,marker_c);
    matrix_drawPixel(29,24,marker_c);
    matrix_drawPixel(24,29,marker_c);
    matrix_drawPixel(16,31,marker_c);
    matrix_drawPixel(8,29,marker_c);
    matrix_drawPixel(3,24,marker_c);
    matrix_drawPixel(1,16,marker_c);
    matrix_drawPixel(3,8,marker_c);
    matrix_drawPixel(8,3,marker_c); 
}


void draw_atime(rtccTime dec_tm, rtccDate dec_dt) {
    char point_sec[2];
    char point_min[2];
    char point_hr[2];
       
    draw_aclock_face(matrix_color444(0,0,5), COLOR565_CYAN);
    
    // Use helpers to get points to draw clock hands
    get_end_pnt60(dec_tm.sec, point_sec);
    get_end_pnt60(dec_tm.min, point_min);
    get_end_pnt12((dec_tm.hour % 12), point_hr);

    // Draw lines for each clock hand
    matrix_drawLine(16,16,16+point_sec[0],16+point_sec[1],matrix_color444(0,2,1));
    matrix_drawLine(16,16,16+point_min[0],16+point_min[1],COLOR565_MAGENTA);
    matrix_drawLine(16,16,16+point_hr[0],16+point_hr[1],COLOR565_YELLOW); 
}

/*******************************************************************************
 * Analog clock endpoint mappings
 ******************************************************************************/

static const char end_pnts60[] = {
    0, -12, //1
    1, -12, //2
    2, -12, //3
    3, -11, //4
    5, -11, //5
    6, -10, //6
    7, -10, //7
    8, -9,  //8
    9, -8,  //9
    10, -7, //10
    10, -6, //11
    11, -5, //12
    11, -3, //13
    12, -2, //14
    12, -1  //15
};

static const char end_pnts12[] = {
    0, -6, //1
    3, -5, //2
    5, -3, //3
};

void get_end_point(int idx, const char* end_pnts, unsigned int num_pnts, char* point) {
    int i = idx % num_pnts;
    int x = end_pnts[i*2];
    int y = end_pnts[(i*2)+1];
    
    const int num_pnts_2x = num_pnts << 1;
    const int num_pnts_3x = (num_pnts << 1) + num_pnts;
    
    if (idx >= num_pnts && idx < num_pnts_2x) {
        swapc(x,y);
        x = -x;
    }
    else if (idx >= num_pnts_2x && idx < num_pnts_3x) {
        x = -x;
        y = -y;
    }
    else if (idx >= num_pnts_3x) {
        swapc(x,y);
        y = -y;
    }
    
    point[0] = x;
    point[1] = y;
}

inline void get_end_pnt60(unsigned char idx, char* ppoint) {
    get_end_point(idx, end_pnts60, 15, ppoint);
}

inline void get_end_pnt12(unsigned char idx, char* ppoint) {
    get_end_point(idx, end_pnts12, 3, ppoint);
}