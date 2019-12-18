/*
 * blocking_delay.h
 *
 * Adapted from tft_master.h/c to pull out delays to be used by other
 * components
 */
#ifndef BLOCKING_DELAY_H
#define BLOCKING_DELAY_H

void delay_ms(unsigned long milliseconds);
void delay_us(unsigned long microseconds);

#endif /* BLOCKING_DELAY_H */
