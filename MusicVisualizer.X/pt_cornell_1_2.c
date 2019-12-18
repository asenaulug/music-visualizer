#include "pt_cornell_1_2.h"

#include "config.h"

//====================================================================
// build a string from the UART2 /////////////
//////////////////////////////////////////////
char PT_term_buffer[max_chars];
int num_char;
int PT_GetSerialBuffer(struct pt *pt)
{
    static char character;
    // mark the beginnning of the input thread
    PT_BEGIN(pt);

    num_char = 0;
    //memset(term_buffer, 0, max_chars);

    while(num_char < max_chars)
    {
        // get the character
        // yield until there is a valid character so that other
        // threads can execute
        PT_YIELD_UNTIL(pt, UARTReceivedDataIsAvailable(UART2) || U2STAbits.OERR);
        
        if (U2STAbits.OERR) {
            if (!(U2STA & _U2STA_URXDA_MASK)) {
                U2STACLR = _U2STA_OERR_MASK;
            }
        }
        
       // while(!UARTReceivedDataIsAvailable(UART2)){};
        character = UARTGetDataByte(UART2);
        //PT_YIELD_UNTIL(pt, UARTTransmitterIsReady(UART2));
        //UARTSendDataByte(UART2, character);

        // unomment to check backspace character!!!
        //printf("--%x--",character );

        // end line
        if(character == '\r'){
            PT_term_buffer[num_char] = 0; // zero terminate the string
            //crlf; // send a new line
            //PT_YIELD_UNTIL(pt, UARTTransmitterIsReady(UART2));
            //UARTSendDataByte(UART2, '\n');
            break;
        }
        else {
            PT_term_buffer[num_char++] = character;
        }

    } //end while(num_char < max_size)

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

    // kill this output thread, to allow spawning thread to execute
    PT_EXIT(pt);
    // and indicate the end of the thread
    PT_END(pt);
}
//#endif //#ifdef use_uart_serial

//======================================================================
// vref confing (if used)
int CVRCON_setup ;

// system time
volatile unsigned int time_tick_millsec ;

// Timer 5 interrupt handler ///////
// ipl2 means "interrupt priority level 2"
void __ISR(_TIMER_5_VECTOR, IPL2AUTO) Timer5Handler(void) //_TIMER_5_VECTOR
{
    // clear the interrupt flag
    mT5ClearIntFlag();
    //count milliseconds
    time_tick_millsec++ ;
}

void PT_setup (void)
{
  // Configure the device for maximum performance but do not change the PBDIV
    // Given the options, this function will change the flash wait states, RAM
    // wait state and enable prefetch cache but will not change the PBDIV.
    // The PBDIV value is already set via the pragma FPBDIV option above..
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

  ANSELA =0; //make sure analog is cleared
  ANSELB =0;
  
#ifdef use_uart_serial
  // === init the uart2 ===================
 PPSInput (2, U2RX, RPB11); //Assign U2RX to pin RPB11 -- Physical pin 22 on 28 PDIP
 PPSOutput(4, RPB10, U2TX); //Assign U2TX to pin RPB10 -- Physical pin 21 on 28 PDIP
  UARTConfigure(UART2, UART_ENABLE_PINS_TX_RX_ONLY);
  UARTSetLineControl(UART2, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
  UARTSetDataRate(UART2, pb_clock, BAUDRATE);
  UARTEnable(UART2, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));
  printf("\n\r..protothreads start..\n\r");
  // === set up DMA for UART output =========
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
#endif //#ifdef use_uart_serial
  
  // ===Set up timer5 ======================
  // timer 5: on,  interrupts, internal clock, 
  // set up to count millsec
  OpenTimer5(T5_ON  | T5_SOURCE_INT | T5_PS_1_1 , PBCLK/1000);
  // set up the timer interrupt with a priority of 2
  ConfigIntTimer5(T5_INT_ON | T5_INT_PRIOR_2);
  mT5ClearIntFlag(); // and clear the interrupt flag
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
