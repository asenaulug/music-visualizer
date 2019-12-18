/* 
 * File:   clock_gfx.h
 * Author: smiller
 *
 * Created on December 6, 2015, 7:17 PM
 */

#ifndef CLOCK_GFX_H
#define	CLOCK_GFX_H

#include "config.h"

void draw_dtime(rtccTime dec_tm, rtccDate dec_dt);
void draw_atime(rtccTime dec_tm, rtccDate dec_dt);

inline void draw_aclock_face(UINT16 circle_c, UINT16 marker_c);

#endif	/* CLOCK_GFX_H */

