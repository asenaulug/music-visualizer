/*
 * File:   config_1_3_2.h
 * Author: Modifed by: Bruce Land 
 * Created on Dec 10, 2018
 * 
 */

#ifndef CONFIG_H
#define	CONFIG_H
#define _SUPPRESS_PLIB_WARNING 
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include <plib.h>
// serial stuff
#include <stdio.h>

//=============================================================
// 40 MHz sys_clock with 40 MHz peripherial clock
#pragma config FNOSC = FRCPLL, POSCMOD = OFF
#pragma config FPLLIDIV = DIV_2, FPLLMUL = MUL_20, FPLLODIV = DIV_2  //40 MHz
#pragma config FPBDIV = DIV_1 // PB 40 MHz
#pragma config FWDTEN = OFF,  JTAGEN = OFF
#pragma config FSOSCEN = OFF  //PINS 11 and 12 to secondary oscillator!
#pragma config DEBUG = OFF   // RB4 and RB5
//////////////////////////
// set up clock parameters
// system cpu clock REFER to ABOVE settings
#define sys_clock 40000000
// sys_clock/FPBDIV
#define pb_clock sys_clock/1 // divide by one in this case

//==============================================================
// Protothreads configure
//=== use Vref debug ===========================================
// IF use_vref_debug IS defined, pin 25 is Vref output
//#define use_vref_debug
//===use UART===================================================
// IF use_uart_serial IS defined, two pins are used by the uart
// Go to pt_cornell_1_3_1.h and search for "SET UART i/o PINS"
//#define use_uart_serial
// BAUDRATE must match PC terminal emulator setting
#define BAUDRATE 38400 
//==============================================================

#endif	/* CONFIG_H */

