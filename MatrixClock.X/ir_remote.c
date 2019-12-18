#include "ir_remote.h"

#include "config.h"
#include "pt_cornell_1_2.h"
#include "rgb_matrix.h"
#include "matrix_gfx.h"

/************************* Variable declarations ******************************/

// String names for opcodes
static const char * const adafruit_remote_mapping[];
static const char * const apple_remote_mapping[];

// The pulse ring buffer for montoring the pulses
// Even indexs are low pulses, odds are high
#define PULSES_BUFFER_SIZE 5
unsigned short pulses_buffer[PULSES_BUFFER_SIZE][200];
volatile char pulses_start_idx; // index to read pulse array
volatile char pulses_end_idx;   // index to write pulse array
volatile unsigned short curr_pulse_idx = 0; // current pulse to record
// For reseting the pulses on timer overflow (20ms)
volatile BOOL end_pulse = FALSE;

/********************* Forward declarations for helpers ***********************/

inline BOOL is_pulse_buffer_full();
inline BOOL is_pulse_buffer_empty();
inline unsigned short *pulse_array_to_write();
inline void finish_writing_pulse();
inline unsigned short *pulse_array_to_read();
inline void finish_reading_pulse();

apple_remote_opcode_t int2opcode_apple(int opcode);
adafruit_remote_opcode_t int2opcode_adafruit(int opcode);
const char* opcode2str_apple(int opcode);
const char* opcode2str_adafruit(int opcode);

int ir_getAddr(signed char* decoded_pulses);
int ir_getOpcode(signed char* decoded_pulses);

ir_remote_type_t ir_addr2remotetype(int addr);

inline BOOL ir_isInitPulse(int a, int b);
inline BOOL ir_isRepeatPulse(int a, int b);
inline char ir_parseLogicPulse(int a);

void ir_decodepulses(signed char* decoded_pulses);


void ir_init() {
    // Configure Timer 3
    // Setup timer3 on, interrupts, internal clock, perscalar of 64, toggle rate
    // System CPU is configured to be 40 MHz
    // With Prescalar of 64, this means each timer tick is 1.6e-6 seconds
    // Timer Period is configured to trigger gap time at 20ms (12500 ticks)

    OpenTimer3(T3_ON | T3_SOURCE_INT | T3_PS_1_64, 0x30D4);
    ConfigIntTimer3(T3_INT_ON | T3_INT_PRIOR_2);
    mT3ClearIntFlag(); // clear int flag

    // Set up the input capture
    OpenCapture1(IC_EVERY_EDGE | IC_INT_1CAPTURE | IC_TIMER3_SRC | IC_ON);
    // turn on the interrupt so that every catpure can be recorded
    ConfigIntCapture1(IC_INT_ON | IC_INT_PRIOR_3 | IC_INT_SUB_PRIOR_3);
    INTClearFlag(INT_IC1);
    // connect PIN 15 to IC1 Capture
    PPSInput(3, IC1, RPB13);
    mPORTBSetPinsDigitalIn(BIT_13);  // set pin as input
}

// Return 1 if ir is not ready, -1 on error, and 0 on success
char ir_receive(ir_cmd_t* cmd_ptr) {
    static char decoded_pulses[50];
    
    if (!cmd_ptr) return -1;
    if (!ir_ready) return 1;
    
    ir_decodepulses(decoded_pulses);
    ir_ready = !is_pulse_buffer_empty();
    
    cmd_ptr->addr = ir_getAddr(decoded_pulses);
    cmd_ptr->remote_type = ir_addr2remotetype(cmd_ptr->addr);
    cmd_ptr->is_repeat = (decoded_pulses[1] == 99);
    cmd_ptr->opcode = ir_getOpcode(decoded_pulses);
    
    if (cmd_ptr->remote_type == ir_remote_type_apple) {
        // Divide the opcode by 16 to simplify calculations
        cmd_ptr->opcode = int2opcode_apple(cmd_ptr->opcode >> 4); 
        cmd_ptr->str = opcode2str_apple(cmd_ptr->opcode);
    }
    else if (cmd_ptr->remote_type == ir_remote_type_adafruit) {
        // Divide the opcode by 4 to simplify calculations
        cmd_ptr->opcode = int2opcode_adafruit(cmd_ptr->opcode >> 2);
        cmd_ptr->str = opcode2str_adafruit(cmd_ptr->opcode);
    }
    else {
        cmd_ptr->str = opcode2str_apple(apple_remote_opcode_unknown);
    }
    
    return 0;
}

//=== Capture 1 ISR ================================
void __ISR(_INPUT_CAPTURE_1_VECTOR, IPL3AUTO) C1Handler(void){
    static capture;

    if (!is_pulse_buffer_full()) {
        capture = mIC1ReadCapture();   // Capture time
        WriteTimer3(0);            // Reset Timer to 0
        pulse_array_to_write()[curr_pulse_idx++] = capture;
        
        end_pulse = TRUE;
    }
    
    mIC1ClearIntFlag();
}

//=== Timer 2 Overflow Handler
// Will clear the pulses variable to reset for next capture
void __ISR(_TIMER_3_VECTOR, IPL2AUTO) Timer3Handler(void){
    if (end_pulse){
        end_pulse = FALSE;
        curr_pulse_idx = 0;
        ir_ready = TRUE;
        
        finish_writing_pulse();
    }
    mT3ClearIntFlag();
}


void ir_decodepulses(signed char* decoded_pulses) {
    static char i;
    static BOOL in_pulse;
    static unsigned short * pulse_array;
        
    pulse_array = pulse_array_to_read();
    for(i = 0; i < 40; i++){
        if(ir_isRepeatPulse(pulse_array[(i<<1) + 1], pulse_array[(i+1) << 1])){
            decoded_pulses[i+1] = 99; // 99 signifies repeat pulse
            break;
        }
        if(ir_isInitPulse(pulse_array[(i<<1) + 1],pulse_array[((i+1) << 1)])){ // First find a begining pulse
            decoded_pulses[i+1] = 52; // 52 signifies init pulse
            in_pulse = 1;
            i++;
            continue;
        }
        if(in_pulse = 1){ // Once we are in the pulse, then decode the pulses via gaps
            decoded_pulses[i] = ir_parseLogicPulse(pulse_array[i<<1]);
        }
    }

    finish_reading_pulse();
}

inline BOOL is_pulse_buffer_full() {
    return ((pulses_end_idx+1) % PULSES_BUFFER_SIZE) == pulses_start_idx;
}

inline BOOL is_pulse_buffer_empty() {
    return (pulses_end_idx == pulses_start_idx);
}

inline unsigned short *pulse_array_to_write() {
    return pulses_buffer[pulses_end_idx];
}

inline void finish_writing_pulse() {
    pulses_end_idx = (pulses_end_idx+1) % PULSES_BUFFER_SIZE;
    if (is_pulse_buffer_empty()) {
        pulses_start_idx = (pulses_start_idx+1) % PULSES_BUFFER_SIZE;
    }
}

inline unsigned short *pulse_array_to_read() {
    return pulses_buffer[pulses_start_idx];
}

inline void finish_reading_pulse() {
    memset(pulses_buffer[pulses_start_idx], 0, 200);
    pulses_start_idx = (pulses_start_idx+1) % PULSES_BUFFER_SIZE;
}

ir_remote_type_t ir_addr2remotetype(int addr) {
    switch(addr) {
        case APPLE_REMOTE_ADDR:
            return ir_remote_type_apple;
        case ADAFRUIT_REMOTE_ADDR:
            return ir_remote_type_adafruit;
        default:
            return ir_remote_type_unknown;
    }
}

int ir_getAddr(signed char* decoded_pulses) {
    static int addr, i;
    addr = 0;
    for(i = 2; i <= 9; i++){  // First check to see if the address is 0
        addr |= (decoded_pulses[i] << (i-2));
    }
    return addr;
}

int ir_getOpcode(signed char* decoded_pulses) {
    static int opcode, i;
    opcode = 0;
    for(i = 18; i < 26; i++){
        opcode = decoded_pulses[i] + (opcode << 1);
    }
    return opcode;
}

inline BOOL ir_isInitPulse(int a, int b) {
    return (a >= 5650 && a <= 5750) && (b >= 2750 && b <= 2850);
}

// Detects a Repeat Pulse
inline BOOL ir_isRepeatPulse(int a, int b) {
    return (a >= 5650 && a <= 5750) && (b >= 1350 && b <= 1450);
}

// Decodes Logical Pulses
inline char ir_parseLogicPulse(int a) {
    if(a >= 300 && a <= 400){ // 0 logical pulse
        return 0;
    }
    if(a >= 1000 && a <= 1100){ // 1 logical pulse
        return 1;
    }
    return 2;
}

apple_remote_opcode_t int2opcode_apple(int opcode) {
    switch (opcode) {
        case apple_remote_opcode_back:
            return apple_remote_opcode_back;
            break;
        case apple_remote_opcode_enter:
            return apple_remote_opcode_enter;
            break;
        case apple_remote_opcode_menu:
            return apple_remote_opcode_menu;
            break;
        case apple_remote_opcode_minus:
            return apple_remote_opcode_minus;
            break;
        case apple_remote_opcode_plus:
            return apple_remote_opcode_plus;
            break;
        case apple_remote_opcode_forw:
            return apple_remote_opcode_forw;
            break;
        default:
            return apple_remote_opcode_unknown;
    }  
}

const char* opcode2str_apple(int opcode) {
    switch (opcode) {
        case apple_remote_opcode_back:
            return apple_remote_mapping[1];
            break;
        case apple_remote_opcode_enter:
            return apple_remote_mapping[5];
            break;
        case apple_remote_opcode_menu:
            return apple_remote_mapping[6];
            break;
        case apple_remote_opcode_minus:
            return apple_remote_mapping[3];
            break;
        case apple_remote_opcode_plus:
            return apple_remote_mapping[4];
            break;
        case apple_remote_opcode_forw:
            return apple_remote_mapping[2];
            break;
        default:
            return apple_remote_mapping[0];
    }
}

adafruit_remote_opcode_t int2opcode_adafruit(int opcode) {
    switch (opcode) {
        case adafruit_remote_opcode_vol_minus:
            return adafruit_remote_opcode_vol_minus;  
        case adafruit_remote_opcode_1:
            return adafruit_remote_opcode_1;  
        case adafruit_remote_opcode_ch_minus:
            return adafruit_remote_opcode_ch_minus;  
        case adafruit_remote_opcode_7:
            return adafruit_remote_opcode_7;  
        case adafruit_remote_opcode_setup:
            return adafruit_remote_opcode_setup;  
        case adafruit_remote_opcode_4:
            return adafruit_remote_opcode_4;  
        case adafruit_remote_opcode_10_plus:
            return adafruit_remote_opcode_10_plus;  
        case adafruit_remote_opcode_vol_plus:
            return adafruit_remote_opcode_vol_plus;  
        case adafruit_remote_opcode_3:
            return adafruit_remote_opcode_3;  
        case adafruit_remote_opcode_ch_plus:
            return adafruit_remote_opcode_ch_plus;  
        case adafruit_remote_opcode_9:
            return adafruit_remote_opcode_9;  
        case adafruit_remote_opcode_stop:
            return adafruit_remote_opcode_stop;  
        case adafruit_remote_opcode_6:
            return adafruit_remote_opcode_6;  
        case adafruit_remote_opcode_replay:
            return adafruit_remote_opcode_replay;  
        case adafruit_remote_opcode_play:
            return adafruit_remote_opcode_play;  
        case adafruit_remote_opcode_2:
            return adafruit_remote_opcode_2;  
        case adafruit_remote_opcode_enter:
            return adafruit_remote_opcode_enter;  
        case adafruit_remote_opcode_8:
            return adafruit_remote_opcode_8;  
        case adafruit_remote_opcode_prev:
            return adafruit_remote_opcode_prev;  
        case adafruit_remote_opcode_5:
            return adafruit_remote_opcode_5; 
        case adafruit_remote_opcode_next:
            return adafruit_remote_opcode_next; 
        
        default:
            return adafruit_remote_opcode_unknown;
    }
}

const char* opcode2str_adafruit(int opcode) {
    switch (opcode) {
        case adafruit_remote_opcode_vol_minus:
            return apple_remote_mapping[1];  
        case adafruit_remote_opcode_1:
            return apple_remote_mapping[2];  
        case adafruit_remote_opcode_ch_minus:
            return apple_remote_mapping[3];  
        case adafruit_remote_opcode_7:
            return apple_remote_mapping[4];  
        case adafruit_remote_opcode_setup:
            return apple_remote_mapping[5];  
        case adafruit_remote_opcode_4:
            return apple_remote_mapping[6];  
        case adafruit_remote_opcode_10_plus:
            return apple_remote_mapping[7];  
        case adafruit_remote_opcode_vol_plus:
            return apple_remote_mapping[8];  
        case adafruit_remote_opcode_3:
            return apple_remote_mapping[9];  
        case adafruit_remote_opcode_ch_plus:
            return apple_remote_mapping[10];  
        case adafruit_remote_opcode_9:
            return apple_remote_mapping[11];  
        case adafruit_remote_opcode_stop:
            return apple_remote_mapping[12];  
        case adafruit_remote_opcode_6:
            return apple_remote_mapping[13];  
        case adafruit_remote_opcode_replay:
            return apple_remote_mapping[14];  
        case adafruit_remote_opcode_play:
            return apple_remote_mapping[15];  
        case adafruit_remote_opcode_2:
            return apple_remote_mapping[16];  
        case adafruit_remote_opcode_enter:
            return apple_remote_mapping[17];  
        case adafruit_remote_opcode_8:
            return apple_remote_mapping[18];  
        case adafruit_remote_opcode_prev:
            return apple_remote_mapping[19];  
        case adafruit_remote_opcode_5:
            return apple_remote_mapping[20]; 
        case adafruit_remote_opcode_next:
            return apple_remote_mapping[21]; 
        
        default:
            return adafruit_remote_mapping[0];
    }
}

void matrix_drawIRPulses(signed char *decoded_pulses) {
    static unsigned char i;
    static const char num_in_row = 20;
    
    for(i = 0; i < 40; i++) {
        matrix_drawPixel(i % num_in_row, 10 + (i/num_in_row), COLOR565_BLACK);
        
        if (decoded_pulses[i] == 99) {
            matrix_drawPixel(i % num_in_row, 10 + (i / num_in_row), COLOR565_WHITE);
        } else if (decoded_pulses[i] == 52) {
            matrix_drawPixel(i % num_in_row, 10 + (i / num_in_row), COLOR565_BLUE);
        } else if (decoded_pulses[i] == 0) {
            matrix_drawPixel(i % num_in_row, 10 + (i / num_in_row), COLOR565_RED);
        } else if (decoded_pulses[i] == 1) {
            matrix_drawPixel(i % num_in_row, 10 + (i / num_in_row), COLOR565_GREEN);
        } else if (decoded_pulses[i] == 2) {
            matrix_drawPixel(i % num_in_row, 10 + (i / num_in_row), COLOR565_YELLOW);
        }
    }
}

static const char * const adafruit_remote_mapping[] = {
    "UNK",
    "Vol-",   //0x00 Vol-
    "1",         //0x08 1
    "CH-",     //0x10 CH-
    "7",         //0x18 7
    "Setup", //0x20 Setup
    "4",         //0x28 4
    "0.10+", //0x30 0.10+
    "Vol+",   //0x40 Vol+
    "3",         //0x48 3
    "CH+",     //0x50 CH+
    "9",         //0x58 9
    "Stop",   //0x60 Stop
    "6",         //0x68 6
    "U-turn", //0x70 U-turn
    "Play",   //0x80 Play
    "2",         //0x88 2
    "Enter",  //0x90 Enter
    "8",         //0x98 8
    "Prev",   //0xA0 Prev
    "5",         //0xA8 5
    "Next",   //0xB0 Next
};

static const char * const apple_remote_mapping[] = {
    "UNK",
    "BACK",
    "FORW",
    "-",
    "+",
    "ENTER",
    "MENU",
};