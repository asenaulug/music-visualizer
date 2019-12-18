/* 
 * File:   ir_remote.h
 * Author: smiller
 *
 * Created on December 5, 2015, 2:33 AM
 */

#ifndef IR_REMOTE_H
#define	IR_REMOTE_H

#define APPLE_REMOTE_ADDR 0b11101110
#define ADAFRUIT_REMOTE_ADDR 0

typedef enum ir_remote_type {
    ir_remote_type_unknown,
    ir_remote_type_apple,
    ir_remote_type_adafruit
} ir_remote_type_t;

typedef enum apple_remote_opcode {
    apple_remote_opcode_unknown = -1,
    apple_remote_opcode_back = 1,
    apple_remote_opcode_enter = 2,
    apple_remote_opcode_menu = 4,
    apple_remote_opcode_minus = 11,
    apple_remote_opcode_plus = 13,
    apple_remote_opcode_forw = 14
} apple_remote_opcode_t;

typedef enum adafruit_remote_opcode {
    adafruit_remote_opcode_unknown = -1,
    adafruit_remote_opcode_vol_minus = 0,
    adafruit_remote_opcode_1 = 2,
    adafruit_remote_opcode_ch_minus = 4,
    adafruit_remote_opcode_7 = 6,
    adafruit_remote_opcode_setup = 8,
    adafruit_remote_opcode_4 = 10,
    adafruit_remote_opcode_10_plus = 12,
    adafruit_remote_opcode_vol_plus = 16,
    adafruit_remote_opcode_3 = 18,
    adafruit_remote_opcode_ch_plus = 20,
    adafruit_remote_opcode_9 = 22,
    adafruit_remote_opcode_stop = 24,
    adafruit_remote_opcode_6 = 26,
    adafruit_remote_opcode_replay = 28,
    adafruit_remote_opcode_play = 32,
    adafruit_remote_opcode_2 = 34,
    adafruit_remote_opcode_enter = 36,
    adafruit_remote_opcode_8 = 38,
    adafruit_remote_opcode_prev = 40,
    adafruit_remote_opcode_5 = 42,
    adafruit_remote_opcode_next = 44
} adafruit_remote_opcode_t;

typedef struct {
    int opcode;
    int addr;
    unsigned char is_repeat;
    ir_remote_type_t remote_type;
    const char *str;
} ir_cmd_t;

volatile unsigned char ir_ready;

void ir_init();

char ir_receive(ir_cmd_t* cmd_ptr);

#endif	/* IR_REMOTE_H */

