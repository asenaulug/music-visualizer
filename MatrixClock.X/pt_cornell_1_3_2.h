/* 
 * File:   pt_cornell_1_3_2.h
 * Author: brl4 Briuce Land
 * Bruce R Land, Cornell University
 * Created on Dec 10, 2018
 */

/*
 * Copyright (c) 2004-2005, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: pt.h,v 1.7 2006/10/02 07:52:56 adam Exp $
 */
#include <plib.h>
/**
 * \addtogroup pt
 * @{
 */

/**
 * \file
 * Protothreads implementation.
 * \author
 * Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __PT_H__
#define __PT_H__

////////////////////////
//#include "lc.h"
////////////////////////
/**
 * \file lc.h
 * Local continuations
 * \author
 * Adam Dunkels <adam@sics.se>
 *
 */

#ifdef DOXYGEN
/**
 * Initialize a local continuation.
 *
 * This operation initializes the local continuation, thereby
 * unsetting any previously set continuation state.
 *
 * \hideinitializer
 */
#define LC_INIT(lc)

/**
 * Set a local continuation.
 *
 * The set operation saves the state of the function at the point
 * where the operation is executed. As far as the set operation is
 * concerned, the state of the function does <b>not</b> include the
 * call-stack or local (automatic) variables, but only the program
 * counter and such CPU registers that needs to be saved.
 *
 * \hideinitializer
 */
#define LC_SET(lc)

/**
 * Resume a local continuation.
 *
 * The resume operation resumes a previously set local continuation, thus
 * restoring the state in which the function was when the local
 * continuation was set. If the local continuation has not been
 * previously set, the resume operation does nothing.
 *
 * \hideinitializer
 */
#define LC_RESUME(lc)

/**
 * Mark the end of local continuation usage.
 *
 * The end operation signifies that local continuations should not be
 * used any more in the function. This operation is not needed for
 * most implementations of local continuation, but is required by a
 * few implementations.
 *
 * \hideinitializer 
 */
#define LC_END(lc)

/**
 * \var typedef lc_t;
 *
 * The local continuation type.
 *
 * \hideinitializer
 */
#endif /* DOXYGEN */

//#ifndef __LC_H__
//#define __LC_H__


//#ifdef LC_INCLUDE
//#include LC_INCLUDE
//#else

/////////////////////////////
//#include "lc-switch.h"
/////////////////////////////

//#ifndef __LC_SWITCH_H__
//#define __LC_SWITCH_H__

/* WARNING! lc implementation using switch() does not work if an
   LC_SET() is done within another switch() statement! */

/** \hideinitializer */
/*
typedef unsigned short lc_t;

#define LC_INIT(s) s = 0;

#define LC_RESUME(s) switch(s) { case 0:

#define LC_SET(s) s = __LINE__; case __LINE__:

#define LC_END(s) }

#endif /* __LC_SWITCH_H__ */

/** @} */

//#endif /* LC_INCLUDE */

//#endif /* __LC_H__ */

/** @} */
/** @} */

/////////////////////////////
//#include "lc-addrlabels.h"
/////////////////////////////

#ifndef __LC_ADDRLABELS_H__
#define __LC_ADDRLABELS_H__

/** \hideinitializer */
typedef void * lc_t;

#define LC_INIT(s) s = NULL

#define LC_RESUME(s)				\
  do {						\
    if(s != NULL) {				\
      goto *s;					\
    }						\
  } while(0)

#define LC_CONCAT2(s1, s2) s1##s2
#define LC_CONCAT(s1, s2) LC_CONCAT2(s1, s2)

#define LC_SET(s)				\
  do {						\
    LC_CONCAT(LC_LABEL, __LINE__):   	        \
    (s) = &&LC_CONCAT(LC_LABEL, __LINE__);	\
  } while(0)

#define LC_END(s)

#endif /* __LC_ADDRLABELS_H__ */

//////////////////////////////////////////
struct pt {
  lc_t lc;
};

#define PT_WAITING 0
#define PT_YIELDED 1
#define PT_EXITED  2
#define PT_ENDED   3

/**
 * \name Initialization
 * @{
 */

/**
 * Initialize a protothread.
 *
 * Initializes a protothread. Initialization must be done prior to
 * starting to execute the protothread.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \sa PT_SPAWN()
 *
 * \hideinitializer
 */
#define PT_INIT(pt)   LC_INIT((pt)->lc)

/** @} */

/**
 * \name Declaration and definition
 * @{
 */

/**
 * Declaration of a protothread.
 *
 * This macro is used to declare a protothread. All protothreads must
 * be declared with this macro.
 *
 * \param name_args The name and arguments of the C function
 * implementing the protothread.
 *
 * \hideinitializer
 */
#define PT_THREAD(name_args) char name_args

/**
 * Declare the start of a protothread inside the C function
 * implementing the protothread.
 *
 * This macro is used to declare the starting point of a
 * protothread. It should be placed at the start of the function in
 * which the protothread runs. All C statements above the PT_BEGIN()
 * invokation will be executed each time the protothread is scheduled.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \hideinitializer
 */
#define PT_BEGIN(pt) { char PT_YIELD_FLAG = 1; LC_RESUME((pt)->lc)

/**
 * Declare the end of a protothread.
 *
 * This macro is used for declaring that a protothread ends. It must
 * always be used together with a matching PT_BEGIN() macro.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \hideinitializer
 */
#define PT_END(pt) LC_END((pt)->lc); PT_YIELD_FLAG = 0; \
                   PT_INIT(pt); return PT_ENDED; }

/** @} */

/**
 * \name Blocked wait
 * @{
 */

/**
 * Block and wait until condition is true.
 *
 * This macro blocks the protothread until the specified condition is
 * true.
 *
 * \param pt A pointer to the protothread control structure.
 * \param condition The condition.
 *
 * \hideinitializer
 */
#define PT_WAIT_UNTIL(pt, condition)	        \
  do {						\
    LC_SET((pt)->lc);				\
    if(!(condition)) {				\
      return PT_WAITING;			\
    }						\
  } while(0)

/**
 * Block and wait while condition is true.
 *
 * This function blocks and waits while condition is true. See
 * PT_WAIT_UNTIL().
 *
 * \param pt A pointer to the protothread control structure.
 * \param cond The condition.
 *
 * \hideinitializer
 */
#define PT_WAIT_WHILE(pt, cond)  PT_WAIT_UNTIL((pt), !(cond))

/** @} */

/**
 * \name Hierarchical protothreads
 * @{
 */

/**
 * Block and wait until a child protothread completes.
 *
 * This macro schedules a child protothread. The current protothread
 * will block until the child protothread completes.
 *
 * \note The child protothread must be manually initialized with the
 * PT_INIT() function before this function is used.
 *
 * \param pt A pointer to the protothread control structure.
 * \param thread The child protothread with arguments
 *
 * \sa PT_SPAWN()
 *
 * \hideinitializer
 */
#define PT_WAIT_THREAD(pt, thread) PT_WAIT_WHILE((pt), PT_SCHEDULE(thread))

/**
 * Spawn a child protothread and wait until it exits.
 *
 * This macro spawns a child protothread and waits until it exits. The
 * macro can only be used within a protothread.
 *
 * \param pt A pointer to the protothread control structure.
 * \param child A pointer to the child protothread's control structure.
 * \param thread The child protothread with arguments
 *
 * \hideinitializer
 */
#define PT_SPAWN(pt, child, thread)		\
  do {						\
    PT_INIT((child));				\
    PT_WAIT_THREAD((pt), (thread));		\
  } while(0)

/** @} */

/**
 * \name Exiting and restarting
 * @{
 */

/**
 * Restart the protothread.
 *
 * This macro will block and cause the running protothread to restart
 * its execution at the place of the PT_BEGIN() call.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \hideinitializer
 */
#define PT_RESTART(pt)				\
  do {						\
    PT_INIT(pt);				\
    return PT_WAITING;			\
  } while(0)

/**
 * Exit the protothread.
 *
 * This macro causes the protothread to exit. If the protothread was
 * spawned by another protothread, the parent protothread will become
 * unblocked and can continue to run.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \hideinitializer
 */
#define PT_EXIT(pt)				\
  do {						\
    PT_INIT(pt);				\
    return PT_EXITED;			\
  } while(0)

/** @} */

/**
 * \name Calling a protothread
 * @{
 */

/**
 * Schedule a protothread.
 *
 * This function shedules a protothread. The return value of the
 * function is non-zero if the protothread is running or zero if the
 * protothread has exited.
 *
 * \param f The call to the C function implementing the protothread to
 * be scheduled
 *
 * \hideinitializer
 */
#define PT_SCHEDULE(f) ((f) < PT_EXITED)
//#define PT_SCHEDULE(f) ((f))

/** @} */

/**
 * \name Yielding from a protothread
 * @{
 */

/**
 * Yield from the current protothread.
 *
 * This function will yield the protothread, thereby allowing other
 * processing to take place in the system.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \hideinitializer
 */
#define PT_YIELD(pt)				\
  do {						\
    PT_YIELD_FLAG = 0;				\
    LC_SET((pt)->lc);				\
    if(PT_YIELD_FLAG == 0) {			\
      return PT_YIELDED;			\
    }						\
  } while(0)

/**
 * \brief      Yield from the protothread until a condition occurs.
 * \param pt   A pointer to the protothread control structure.
 * \param cond The condition.
 *
 *             This function will yield the protothread, until the
 *             specified condition evaluates to true.
 *
 *
 * \hideinitializer
 */
#define PT_YIELD_UNTIL(pt, cond)		\
  do {						\
    PT_YIELD_FLAG = 0;				\
    LC_SET((pt)->lc);				\
    if((PT_YIELD_FLAG == 0) || !(cond)) {	\
      return PT_YIELDED;                        \
    }						\
  } while(0)

/** @} */

#endif /* __PT_H__ */

#ifndef __PT_SEM_H__
#define __PT_SEM_H__

//#include "pt.h"

struct pt_sem {
  unsigned int count;
};

/**
 * Initialize a semaphore
 *
 * This macro initializes a semaphore with a value for the
 * counter. Internally, the semaphores use an "unsigned int" to
 * represent the counter, and therefore the "count" argument should be
 * within range of an unsigned int.
 *
 * \param s (struct pt_sem *) A pointer to the pt_sem struct
 * representing the semaphore
 *
 * \param c (unsigned int) The initial count of the semaphore.
 * \hide initializer
 */
#define PT_SEM_INIT(s, c) (s)->count = c

/**
 * Wait for a semaphore
 *
 * This macro carries out the "wait" operation on the semaphore. The
 * wait operation causes the protothread to block while the counter is
 * zero. When the counter reaches a value larger than zero, the
 * protothread will continue.
 *
 * \param pt (struct pt *) A pointer to the protothread (struct pt) in
 * which the operation is executed.
 *
 * \param s (struct pt_sem *) A pointer to the pt_sem struct
 * representing the semaphore
 *
 * \hideinitializer
 */
#define PT_SEM_WAIT(pt, s)	\
  do {						\
    PT_WAIT_UNTIL(pt, (s)->count > 0);		\
    --(s)->count;				\
  } while(0)

/**
 * Signal a semaphore
 *
 * This macro carries out the "signal" operation on the semaphore. The
 * signal operation increments the counter inside the semaphore, which
 * eventually will cause waiting protothreads to continue executing.
 *
 * \param pt (struct pt *) A pointer to the protothread (struct pt) in
 * which the operation is executed.
 *
 * \param s (struct pt_sem *) A pointer to the pt_sem struct
 * representing the semaphore
 *
 * \hideinitializer
 */
#define PT_SEM_SIGNAL(pt, s) ++(s)->count

#endif /* __PT_SEM_H__ */

//=====================================================================
//=== BRL4 additions for PIC 32 =======================================
//=====================================================================

// macro to time a thread execution interveal in millisec
// max time 4000 sec

#define PT_YIELD_TIME_msec(delay_time)  \
    do { static unsigned int time_thread ;\
    time_thread = time_tick_millsec + (unsigned int)delay_time ; \
    PT_YIELD_UNTIL(pt, (time_tick_millsec >= time_thread)); \
    } while(0);

// macro to return system time
#define PT_GET_TIME() (time_tick_millsec)

// init rate sehcduler
//#define PT_INIT(pt, priority)   LC_INIT((pt)->lc ; (pt)->pri = priority)
//PT_PRIORITY_INIT
#define PT_RATE_INIT() int pt_pri_count = 0;
// maitain proority frame count
//PT_PRIORITY_LOOP maitains a counter used to control execution
#define PT_RATE_LOOP() pt_pri_count = (pt_pri_count+1) & 0xf ;
// schecedule priority thread
//PT_PRIORITY_SCHEDULE
// 5 levels
// rate 0 is highest -- every time thru loop
// priority 1 -- every 2 times thru loop
// priority 2 -- every 4 times thru loop
//  3 is  -- every 8 times thru loop
#define PT_RATE_SCHEDULE(f,rate) \
    if((rate==0) | \
    (rate==1 && ((pt_pri_count & 0b1)==0) ) | \
    (rate==2 && ((pt_pri_count & 0b11)==0) ) | \
    (rate==3 && ((pt_pri_count & 0b111)==0)) | \
    (rate==4 && ((pt_pri_count & 0b1111)==0))) \
        PT_SCHEDULE(f);

// macro to use 4 bit DAC as debugger output
// level range 0-15; duration in approx microseconds
// -- with zero meaning HOLD it on forever
#define PT_DEBUG_VALUE(level, duration) \
do { static int i ; \
    CVRCON = CVRCON_setup | (level & 0xf); \
    if (duration>0){                   \
        for (i=0; i<duration*7; i++){};\
        CVRCON = CVRCON_setup; \
    } \
} while(0);

// macros to manipulate a semaphore without blocking
#define PT_SEM_SET(s) (s)->count=1
#define PT_SEM_CLEAR(s) (s)->count=0
#define PT_SEM_READ(s) (s)->count
#define PT_SEM_ACCEPT(s) \
  s->count; \
  if (s->count) s->count-- ; \

/*---PT interface ---------------------------------------------------------
// make ubasic_run A thread
static struct pt pt_basic_run ;
//void ubasic_run(void)
static PT_THREAD (protothread_basic_run(struct pt *pt))
{
  PT_BEGIN(pt);
  if(tokenizer_finished()) {
    //DEBUG_PRINTF("uBASIC program finished\n");
    //return;
    // kill this input thread, to allow spawning thread to execute
    PT_EXIT(pt);
  }
  statement(); //modified to eliminate line numbers
  //PT_YIELD(pt);
  //line_statement();
  PT_EXIT(pt);
  // and indicate the end of the thread
  PT_END(pt);
}*/
/*---------------------------------------------------------------------------*/

//====================================================================
// IMPROVED SCHEDULER

  
// === thread structures ===
// thread control structs

// A modified scheduler
static struct pt pt_sched ;

// count of defined tasks
int pt_task_count = 0 ;

// The task structure
struct ptx {
	struct pt pt;              // thread context
	int num;                    // thread number
	char (*pf)(struct pt *pt); // pointer to thread function
    int rate;
};

// === extended structure for scheduler ===============
// an array of task structures
#define MAX_THREADS 10
static struct ptx pt_thread_list[MAX_THREADS];
// see https://github.com/edartuz/c-ptx/tree/master/src
// and the license above
// add an entry to the thread list
//struct ptx *pt_add( char (*pf)(struct pt *pt), int rate) {
int pt_add( char (*pf)(struct pt *pt), int rate) {
	if (pt_task_count < (MAX_THREADS)) {
        // get the current thread table entry 
		struct ptx *ptx = &pt_thread_list[pt_task_count];
        // enter the tak data into the thread table
		ptx->num   = pt_task_count;
        // function pointer
		ptx->pf    = pf;
        // rate scheduler rate
        ptx->rate  = rate ; 
		PT_INIT( &ptx->pt );
        // count of number of defined threads
		pt_task_count++;
        // return current entry
        return pt_task_count-1;
	}
	return NULL;
}

/* Scheduler
Copyright (c) 2014 edartuz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
// === Scheduler Thread =================================================
// update a 1 second tick counter
// schedulser code was almost copied from
// https://github.com/edartuz/c-ptx
// see license above

// choose schedule method
#define SCHED_ROUND_ROBIN 0
#define SCHED_RATE 1
int pt_sched_method = SCHED_ROUND_ROBIN ;

#define PT_SET_RATE(thread_num, new_rate) pt_thread_list[thread_num].rate = new_rate
#define PT_GET_RATE(thread_num) pt_thread_list[thread_num].rate 

static PT_THREAD (protothread_sched(struct pt *pt))
{   
    PT_BEGIN(pt);
     // set up rate counter
    PT_RATE_INIT()
    
    static int i, rate;
    
    if (pt_sched_method==SCHED_ROUND_ROBIN){
        while(1) {
          // test stupid round-robin 
          // on all defined threads
          struct ptx *ptx = &pt_thread_list[0];
          // step thru all defined threads
          // -- loop can have more than one initialization or increment/decrement, 
          // -- separated using comma operator. But it can have only one condition.
          for (i=0; i<pt_task_count; i++, ptx++ ){
              // call thread function
              (pt_thread_list[i].pf)(&ptx->pt); 
          }
          // Never yields! 
          // NEVER exit while!
        } // END WHILE(1)
    } // end if(pt_sched_method==SCHED_ROUND_ROBIN)
    
    else if (pt_sched_method==SCHED_RATE){
        while(1) {
            PT_RATE_LOOP() ;
            // test stupid round-robin 
            // on all defined threads
            struct ptx *ptx = &pt_thread_list[0];
            // step thru all defined threads
            // -- loop can have more than one initialization or increment/decrement, 
            // -- separated using comma operator. But it can have only one condition.
            for (i=0; i<pt_task_count; i++, ptx++ ){
                // rate
                rate = pt_thread_list[i].rate ;
                if((rate==0) | 
                (rate==1 && ((pt_pri_count & 0b1)==0) ) | 
                (rate==2 && ((pt_pri_count & 0b11)==0) ) | 
                (rate==3 && ((pt_pri_count & 0b111)==0)) | 
                (rate==4 && ((pt_pri_count & 0b1111)==0))){
                // call thread function
                    (pt_thread_list[i].pf)(&ptx->pt); 
                }
            }
          // Never yields! 
          // NEVER exit while!
        } // END WHILE(1)
    } //end if (pt_sched_method==SCHED_RATE)       
     
    PT_END(pt);
} // scheduler thread

//====================================================================
//=== serial setup ===================================================
//#ifdef use_uart_serial
///////////////////////////
// UART parameters

#define PB_DIVISOR (1 << OSCCONbits.PBDIV) // read the peripheral bus divider, FPBDIV
#define PB_FREQ sys_clock/PB_DIVISOR // periperhal bus frequency
#define clrscr() printf( "\x1b[2J")
#define home()   printf( "\x1b[H")
#define pcr()    printf( '\r')
#define crlf     putchar(0x0a); putchar(0x0d);
#define backspace 0x7f // make sure your backspace matches this!
#define max_chars 64 // for input/output buffer 64
// PuTTY serial terminal control codes
// see 
// http://ascii-table.com/ansi-escape-sequences-vt-100.php
#define cursor_pos(line,column) printf("\x1b[%02d;%02dH",line,column)	
#define clr_right printf("\x1b[K")
// from http://www.comptechdoc.org/os/linux/howlinuxworks/linux_hlvt100.html
// and colors from http://www.termsys.demon.co.uk/vtansi.htm
#define green_text printf("\x1b[32m")
#define yellow_text printf("\x1b[33m")
#define red_text printf("\x1b[31m")
#define rev_text printf("\x1b[7m")
#define normal_text printf("\x1b[0m")
// The following thread definitions are necessary for UART control
static struct pt pt_input, pt_output, pt_DMA_output, pt_DMA_input ;

//====================================================================
// build a string from the UART2 /////////////
// assuming that a HUMAN is typing at a terminal!
//////////////////////////////////////////////
char PT_term_buffer[max_chars];
int PT_GetSerialBuffer(struct pt *pt)
{
    static char character;
    static int num_char;
    // mark the beginnning of the input thread
    PT_BEGIN(pt);

    num_char = 0;
    // clear buffer
    memset(PT_term_buffer, 0, max_chars);

    while(num_char < max_chars)
    {
        // get the character
        // yield until there is a valid character so that other
        // threads can execute
        PT_YIELD_UNTIL(pt, UARTReceivedDataIsAvailable(UART2));
       // while(!UARTReceivedDataIsAvailable(UART2)){};
        character = UARTGetDataByte(UART2);
        PT_YIELD_UNTIL(pt, UARTTransmitterIsReady(UART2));
        UARTSendDataByte(UART2, character);

        // unomment to check backspace character!!!
        //printf("--%x--",character );

        // end line
        if(character == '\r'){
            PT_term_buffer[num_char] = 0; // zero terminate the string
            //crlf; // send a new line
            PT_YIELD_UNTIL(pt, UARTTransmitterIsReady(UART2));
            UARTSendDataByte(UART2, '\n');
            break;
        }
        // backspace
        else if (character == backspace){
            PT_YIELD_UNTIL(pt, UARTTransmitterIsReady(UART2));
            UARTSendDataByte(UART2, ' ');
            PT_YIELD_UNTIL(pt, UARTTransmitterIsReady(UART2));
            UARTSendDataByte(UART2, backspace);
            num_char--;
            // check for buffer underflow
            if (num_char<0) {num_char = 0 ;}
        }
        else  {PT_term_buffer[num_char++] = character ;}
         //if (character == backspace)

    } //end while(num_char < max_size)
    
    // kill this input thread, to allow spawning thread to execute
    PT_EXIT(pt);
    // and indicate the end of the thread
    PT_END(pt);
}

//====================================================================
// build a string from the UART2 /////////////
// assuming MACHINE input
//////////////////////////////////////////////
// !!! you MUST specify EITHER a termination character or a count!!
//////////////////////////////////////////////
// -- terminator character could be <enter> '\r'
// or any other characcter, e.g. '#' or can be set to ZERO
// if there is no termination character
// -- Termination count will return after N characters, regardless of
// the terminator character
// Set to ZERO if there is no termination count.
// -- Termination time is the longest the routine will wait 
// for a terminator event in milliseconds
char PT_terminate_char='\r', PT_terminate_count=0 ;
// terminate time default million seconds
int PT_terminate_time = 0 ;
// timeout return value
int PT_timeout = 0; 

// system time updated in TIMER1 ISR below
volatile unsigned int time_tick_millsec ;

int PT_GetMachineBuffer(struct pt *pt)
{
    static char character;
    static unsigned int num_char, start_time;
    // mark the beginnning of the input thread
    PT_BEGIN(pt);
    
    // actual number received
    num_char = 0;
    //record milliseconds for timeout calculation
    start_time = time_tick_millsec ;
    // clear timeout flag
    PT_timeout = 0;
    // clear input buffer
    memset(PT_term_buffer, 0, max_chars);
    
    // === set up DMA for UART input ===================
    // configure the channel and enable end-on-match OR end-on-length
    // Three possible conditions:
    // PT_terminate_char = '%' ;
    // PT_terminate_count = 0 ;
    // PT_terminate_time = 0 ;
    // === open DMA for UART ============== 
    if (PT_terminate_char>0){
        // are we using a terminate CHARACTER
        DmaChnOpen(DMA_CHANNEL0, DMA_CHN_PRI2, DMA_OPEN_MATCH);
        DmaChnSetMatchPattern(DMA_CHANNEL0, PT_terminate_char);
    }
    else {
        DmaChnOpen(DMA_CHANNEL0, DMA_CHN_PRI2, DMA_OPEN_DEFAULT);
    }
    // ==================
    // set up count and source and destination addresses
    // are we using a terminate COUNT
    if (PT_terminate_count>0 && PT_terminate_count<=max_chars){
        DmaChnSetTxfer(DMA_CHANNEL0, (void*)&U2RXREG, PT_term_buffer, 1, PT_terminate_count, 1);
    }
    else {
        DmaChnSetTxfer(DMA_CHANNEL0, (void*)&U2RXREG, PT_term_buffer, 1, max_chars, 1);
    }
    // === DMA event control ===============
    // trigger a byte  the UART  has data
    DmaChnSetEventControl(DMA_CHANNEL0, DMA_EV_START_IRQ_EN|DMA_EV_MATCH_EN|DMA_EV_START_IRQ(_UART2_RX_IRQ));
    // signal when done
    DmaChnSetEvEnableFlags(DMA_CHANNEL0, DMA_EV_BLOCK_DONE);
    // enable the channel
    DmaChnEnable(DMA_CHANNEL0);
  
    // === Wait for DMA receive channel ==================
    // yield until DMA done OR times out
    // are we using a terminate TIME
    PT_YIELD_UNTIL(pt, (DmaChnGetEvFlags(DMA_CHANNEL0) & DMA_EV_BLOCK_DONE) ||
                       ((PT_terminate_time>0) && (time_tick_millsec >= PT_terminate_time+start_time)));
    
    DmaChnDisable(DMA_CHANNEL0);
    
    // === check for timeout ========================
    if((PT_terminate_time>0) && (time_tick_millsec >= PT_terminate_time+start_time)) {
        // took too long so set the timeout flag
        PT_timeout = 1;
    }
    
    // kill this input thread, to allow spawning thread to execute
    PT_EXIT(pt);
    // and indicate the end of the thread
    PT_END(pt);
}

//====================================================================
// === send a string to the UART2 ====================================
char PT_send_buffer[max_chars];
int num_send_chars ;
int PutSerialBuffer(struct pt *pt)
{
    PT_BEGIN(pt);
    num_send_chars = 0;
    while (PT_send_buffer[num_send_chars] != 0){
        PT_YIELD_UNTIL(pt, UARTTransmitterIsReady(UART2));
        UARTSendDataByte(UART2, PT_send_buffer[num_send_chars]);
        num_send_chars++;
    }
    // kill this output thread, to allow spawning thread to execute
    PT_EXIT(pt);
    // and indicate the end of the thread
    PT_END(pt);
}

//====================================================================
// === DMA send string to the UART2 ==================================
int PT_DMA_PutSerialBuffer(struct pt *pt)
{
    PT_BEGIN(pt);
    //mPORTBSetBits(BIT_0);
    // check for null string
    if (PT_send_buffer[0]==0)PT_EXIT(pt);
    // sent the first character
    PT_YIELD_UNTIL(pt, UARTTransmitterIsReady(UART2));
    UARTSendDataByte(UART2, PT_send_buffer[0]);
    //DmaChnStartTxfer(DMA_CHANNEL1, DMA_WAIT_NOT, 0);
    // start the DMA
    DmaChnEnable(DMA_CHANNEL1);
    // wait for DMA done
    //mPORTBClearBits(BIT_0);
    PT_YIELD_UNTIL(pt, DmaChnGetEvFlags(DMA_CHANNEL1) & DMA_EV_BLOCK_DONE);
    //wait until the transmit buffer is empty
    PT_YIELD_UNTIL(pt, U2STA&0x100);
    
    // kill this output thread, to allow spawning thread to execute
    PT_EXIT(pt);
    // and indicate the end of the thread
    PT_END(pt);
}
//#endif //#ifdef use_uart_serial

//======================================================================
// vref confing (if used)
int CVRCON_setup ;


// force full context save
//int w;
//void waste(void){w=1;};
// Timer 1 interrupt handler ///////
// ipl2 means "interrupt priority level 2"
void __ISR(_TIMER_1_VECTOR, IPL2AUTO) Timer1Handler(void) //_TIMER_1_VECTOR
{
    // clear the interrupt flag
    mT1ClearIntFlag();
    //count milliseconds
    time_tick_millsec++ ;
    //waste();
}

void PT_setup (void)
{
  // Configure the device for maximum performance but do not change the PBDIV
    // Given the options, this function will change the flash wait states, RAM
    // wait state and enable prefetch cache but will not change the PBDIV.
    // The PBDIV value is already set via the pragma FPBDIV option above..
    SYSTEMConfig(sys_clock, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

  ANSELA =0; //make sure analog is cleared
  ANSELB =0;
  
#ifdef use_uart_serial
  // === init the uart2 ===================
 // SET UART i/o PINS
 // The RX pin must be one of the Group 2 input pins
 // RPA1, RPB1, RPB5, RPB8, RPB11
 PPSInput (2, U2RX, RPA1); //Assign U2RX to pin RPA1 -- 
 // The TX pin must be one of the Group 4 output pins
 // RPA3, RPB0, RPB9, RPB10, RPB14 
 PPSOutput(4, RPB10, U2TX); //Assign U2TX to pin RPB10 -- 
 
  UARTConfigure(UART2, UART_ENABLE_PINS_TX_RX_ONLY);
  UARTSetLineControl(UART2, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
  UARTSetDataRate(UART2, pb_clock, BAUDRATE);
  UARTEnable(UART2, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));
  
  // Feel free to comment this out
  clrscr();
  home();
  // reverse video control codes
  normal_text;
  rev_text ;
  printf("...protothreads 1_3_2 07/18/18...");
  normal_text ;
  
  // === set up DMA for UART output ==================
  // configure the channel and enable end-on-match
  DmaChnOpen(DMA_CHANNEL1, DMA_CHN_PRI2, DMA_OPEN_MATCH);
  // trigger a byte everytime the UART is empty
  DmaChnSetEventControl(DMA_CHANNEL1, DMA_EV_START_IRQ_EN|DMA_EV_MATCH_EN|DMA_EV_START_IRQ(_UART2_TX_IRQ));
  // source and destination
  DmaChnSetTxfer(DMA_CHANNEL1, PT_send_buffer+1, (void*)&U2TXREG, max_chars, 1, 1);
  // signal when done
  DmaChnSetEvEnableFlags(DMA_CHANNEL1, DMA_EV_BLOCK_DONE);
  // set null as ending character (of a string)
  DmaChnSetMatchPattern(DMA_CHANNEL1, 0x00);
  
  
  //===================================================
  
#endif //#ifdef use_uart_serial
  
  // ===Set up timer1 ======================
  // timer 1: on,  interrupts, internal clock, 
  // set up to count millsec
  OpenTimer1(T1_ON  | T1_SOURCE_INT | T1_PS_1_1 , pb_clock/1000);
  // set up the timer interrupt with a priority of 2
  ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);
  mT1ClearIntFlag(); // and clear the interrupt flag
  // zero the system time tick
  time_tick_millsec = 0;

  //=== Set up VREF as a debugger output =======
  #ifdef use_vref_debug
  // set up the Vref pin and use as a DAC
  // enable module| eanble output | use low range output | use internal reference | desired step
  CVREFOpen( CVREF_ENABLE | CVREF_OUTPUT_ENABLE | CVREF_RANGE_LOW | CVREF_SOURCE_AVDD | CVREF_STEP_0 );
  // And read back setup from CVRCON for speed later
  // 0x8060 is enabled with output enabled, Vdd ref, and 0-0.6(Vdd) range
  CVRCON_setup = CVRCON; //CVRCON = 0x8060 from Tahmid http://tahmidmc.blogspot.com/

#endif //#ifdef use_vref_debug

}