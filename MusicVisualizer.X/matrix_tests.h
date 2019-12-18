/* 
 * File:   matrix_tests.h
 * Author: smiller
 * 
 * Adapted from corresponding examples: 
 * https://github.com/adafruit/RGB-matrix-Panel/tree/master/examples
 *
 * Created on November 19, 2015, 1:34 PM
 */

#ifndef MATRIX_TESTS_H
#define	MATRIX_TESTS_H

#include "rgb_matrix.h"

// Plasma demo is contained in separate file to keep things neat
#include "matrix_plasma.h"

/* Fill screen with a color wheel */
void draw_colorwheel();

/* Fill screen with various brightness levels of each color */
void draw_levels();

/* Scroll demo text while also animating balls in the background
 * This loops infinitely and should replace the main loop */
void scroll_test_loop();

void draw_scroll_test_frame();

/* Infinite loop that iterates through each of the possible graphics that
 * can be drawn on the matrix */
void shapes_test_loop();

void draw_shapes_test_frame();

#endif	/* MATRIX_TESTS_H */

