/*
 * File:        main.c
 * Author:      Asena Ulug, Cynthia Zelga, Joceyln Gilbert
                Adapted from Sam Miller
 * Target PIC:  PIC32MX250F128B
 */
// Declare this file as main application for the pre-processor
#define MAIN_APPLICATION

#include <stdlib.h>
#include <math.h>
#include <xc.h>

#include "config_1_3_2.h"
#include "port_expander_brl4.h"
#include "rgb_matrix.h"
#include "matrix_gfx.h"
#include "blocking_delay.h"
#include "datetime.h"
#include "pt_cornell_1_3_2.h"
#include "ir_remote.h"
#include "serial_ext.h"
#include "clock_gfx.h"
#include "matrix_tests.h"
#include "dsplib_dsp.h"
#include "tft_master.h"
#include "tft_gfx.h"




ir_cmd_t ir_cmd; //used for IR remote for previous project
BOOL ir_cmd_new = FALSE;

struct pt pt_update_matrix, pt_serial, pt_ir, pt_animate, pt_print;

//// The original format BCD codified date/time and the decimal versions
rtccTime bcd_tm, dec_tm; //used for clock for previous project
rtccDate bcd_dt, dec_dt; //used for clock for previous project

#define DAC_config_chan_A 0b0011000000000000
// B-channel, 1x, active
#define DAC_config_chan_B 0b1011000000000000
// B-channel, 1x, active

volatile unsigned int DAC_data_A, DAC_data_B ;// output value
volatile SpiChannel spiChn = SPI_CHANNEL2 ;	// the SPI channel to use
volatile int spiClkDiv = 4 ; // 10 MHz max speed for port expander!!

int i = 0;
//int log2N = 6; // log2(64) = 6
////int N = 1 << log2N; // N = 2^6 = 64
//int N = 64;
//int16c din[64];
//int16c dout[64];
//int16c scratch[64];
//int16c twiddles[64];


// === the fixed point macros ========================================
typedef signed short fix14 ;
#define multfix14(a,b) ((fix14)((((long)(a))*((long)(b)))>>14)) //multiply two fixed 2.14
#define float2fix14(a) ((fix14)((a)*16384.0)) // 2^14
#define fix2float14(a) ((float)(a)/16384.0)
#define absfix14(a) abs(a) 
// === input arrays ==================================================
#define nSamp 64
#define nPixels 32
fix14 v_in[nSamp] ;

// === thread structures ============================================
// thread control structs
// note that UART input and output are threads
static struct pt pt_fft ;


//=== FFT ==============================================================
// FFT
#define N_WAVE          64    /* size of FFT 512 */
#define LOG2_N_WAVE     6     /* log2(N_WAVE) 0 */
#define begin {
#define end }

fix14 Sinewave[N_WAVE]; // a table of sines for the FFT
fix14 window[N_WAVE]; // a table of window values for the FFT
fix14 fr[N_WAVE], fi[N_WAVE]; //real and imaginary output of FFT
fix14 oldfr[N_WAVE] = {0}; // the previous value of fr
int pixels[nPixels] ; 

void FFTfix(fix14 fr[], fix14 fi[], int m)
//Adapted from code by:
//Tom Roberts 11/8/89 and Malcolm Slaney 12/15/94 malcolm@interval.com
//fr[n],fi[n] are real,imaginary arrays, INPUT AND RESULT.
//size of data = 2**m
// This routine does forward transform only
begin
    int mr,nn,i,j,L,k,istep, n;
    int qr,qi,tr,ti,wr,wi;

    mr = 0;
    n = 1<<m;   //number of points
    nn = n - 1;

    /* decimation in time - re-order data */
    for(m=1; m<=nn; ++m)
    begin
        L = n;
        do L >>= 1; while(mr+L > nn);
        mr = (mr & (L-1)) + L;
        if(mr <= m) continue;
        tr = fr[m];
        fr[m] = fr[mr];
        fr[mr] = tr;
        //ti = fi[m];   //for real inputs, don't need this
        //fi[m] = fi[mr];
        //fi[mr] = ti;
    end

    L = 1;
    k = LOG2_N_WAVE-1;
    while(L < n)
    begin
        istep = L << 1;
        for(m=0; m<L; ++m)
        begin
            j = m << k;
            wr =  Sinewave[j+N_WAVE/4];
            wi = -Sinewave[j];
            //wr >>= 1; do need if scale table
            //wi >>= 1;

            for(i=m; i<n; i+=istep)
            begin
                j = i + L;
                tr = multfix14(wr,fr[j]) - multfix14(wi,fi[j]);
                ti = multfix14(wr,fi[j]) + multfix14(wi,fr[j]);
                qr = fr[i] >> 1;
                qi = fi[i] >> 1;
                fr[j] = qr - tr;
                fi[j] = qi - ti;
                fr[i] = qr + tr;
                fi[i] = qi + ti;
            end
        end
        --k;
        L = istep;
    end
end
    
volatile int done = 0; //flag that indicates whether FFT input array is full
// set by the ISR, checked by the FFT thread

volatile int finished = 1; //flag that indicates whether FFT calculation is complete
//set by FFT thread, checked by ISR and animate thread

int index = 0; //index into the FFT input array
// used by ISR to put ADC value into correct index of array
void __ISR(_TIMER_5_VECTOR, ipl2) Timer5Handler(void)
{
    int junk;
    mT5ClearIntFlag();
    done = 0; //indicates FFT input array is not full yet
    if(finished) { //only store ADC value if FFT computation is complete
        int adc_9;
        adc_9 = ReadADC10(0);   // read the result of channel 9 conversion from the idle buffer
        AcquireADC10(); // not needed if ADC_AUTO_SAMPLING_ON below
        DAC_data_A = adc_9 << 2 ; // use this if you want to output data to DAC 

        v_in[index] = DAC_data_A; //store ADC data in FFT input array
        index++; //increment index
        if (index == nSamp) { //if input array full
            done = 1; //flag that indicates input array full
            index = 0; //reset index to 0
        }
    }

    // Use code below if you want to output ADC value to DAC
        // === Channel A =============
        // wait for possible port expander transactions to complete
//        while (TxBufFullSPI2());
//        // reset spi mode to avoid conflict with port expander
//    //    SPI_Mode16();
//        while (SPI2STATbits.SPIBUSY); // wait for end of transaction
//        // CS low to start transaction
//         mPORTBClearBits(BIT_4); // start transaction
//        // write to spi2 
//        WriteSPI2( DAC_config_chan_A | (DAC_data_A & 0xfff) );
//        while (SPI2STATbits.SPIBUSY); // wait for end of transaction
//         // CS high
//        mPORTBSetBits(BIT_4); // end transaction
//        // need to read SPI channel to avoid confusing port expander
//        junk = ReadSPI2(); 
    
}



// === Animate Thread =================================================
// map FFT output to the LED matrix
static PT_THREAD (protothread_animate(struct pt *pt))
{
    PT_BEGIN(pt);
    //  // set up LED to blink
    //  mPORTASetBits(BIT_0 );	//Clear bits to ensure light is off.
    //  mPORTASetPinsDigitalOut(BIT_0 );    //Set port as output


    // array of all the colors used in the display
    // in rainbow order 
    UINT16 colors[16] = {COLOR565_RED, COLOR565_ORANGE, COLOR565_YELLOW, COLOR565_LIGHT_GREEN, COLOR565_GREEN, 
                        COLOR565_ICE, COLOR565_CYAN, COLOR565_NICE_BLUE, COLOR565_BLUE, 
                        COLOR565_PURPLE,  COLOR565_LIPSTICK, COLOR565_MAGENTA,
                            COLOR565_PINK, COLOR565_PERIWINKLE,  COLOR565_WHITE, COLOR565_WHITE};
      while(1) {
          
        PT_WAIT_UNTIL(pt, finished==1); //wait until FFT computation is complete
        matrix_fillScreen(COLOR565_BLACK); //clear screen
        

        
        // TODO: 
        // get rid of first two bins, always have DC offset in those bins
        // change FFT to be 68 samples, nPixels to 34
        
        // store old fr in an array
        // if new value of fr[i]> old fr[i], draw new value
        // if new value of fr[i]< old fr[i], draw old value * 0.95
        // low passes signal, makes it look better
        
        // change bins and height to make as cute as possible
       
        fr[1] = 0.9*fr[2]; //change DC bin to be related to bin 3
        fr[0] = 0.9*fr[1]; //change DC bin to be related to bin 3
        
        int q; //loop variable
        for(q=0;q<nPixels;q++) { //from 0 to 32 bins

            if (fr[q] < oldfr[q]) {
                // if new value of fr is less than previous value
                // then slowly decay the amplitude
                fr[q] = multfix14(float2fix14(0.97),oldfr[q]);
            }
            UINT16 c; //color
            int h; //height
            // map the amplitude to a color and height
            if(0<fr[q] && fr[q]<80){ c = colors[0]; h =0; } 
            else if(fr[q]<83)      {c = colors[1]; h = 1;} 
            else if(fr[q]<86)      {c = colors[2]; h = 2;}
            else if(fr[q]<89)      {c = colors[3]; h = 3;}
            else if(fr[q]<92)      {c = colors[4]; h = 4;}
            else if(fr[q]<95)      {c = colors[5]; h = 5;}
            else if(fr[q]<98)      {c = colors[6]; h = 6;}
            else if(fr[q]<101)      {c = colors[7]; h = 7;}
            else if(fr[q]<104)      {c = colors[8]; h = 8;}
            else if(fr[q]<107)      {c = colors[9]; h = 9;}
            else if(fr[q]<110)      {c = colors[10]; h = 10;}
            else if(fr[q]<113)      {c = colors[11]; h = 11;} 
            else if(fr[q]<116)      {c = colors[12]; h = 12;} 
            else if(fr[q]<119)      {c = colors[13];h = 13;} 
            else if(fr[q]<122)      {c = colors[14];h = 14;} 
            else                    {c = colors[15]; h = 15;}                   

            matrix_drawLine(31-q,0,31-q,h,c); //draw the column
            oldfr[q] = fr[q]; //set oldfr to previous value of fr 
        }
        PT_YIELD_TIME_msec(30); 
        //this yield is necessary to allow the FFT thread to run
        
         //NEVER exit while
      } // END WHILE(1)
  PT_END(pt);
} // timer thread


//GET RID OF DMA STUFF!!!

// We have 32 bins, for each bin delta_f = 125 Hz
// Each bin corresponds to a column on the LED matrix display
// === FFT Thread =============================================
    
// DMA channel busy flag
#define CHN_BUSY 0x80
#define log_min 0x10   
static PT_THREAD (protothread_fft(struct pt *pt))
{
    PT_BEGIN(pt);
    static int sample_number ;
    static fix14 zero_point_4 = float2fix14(0.4) ;
    // approx log calc ;
    static int sx, y, ly, temp ;
    
    while(1) {
       
        // enable ADC DMA channel and get
        // 512 samples from ADC
//        DmaChnEnable(0);
        // yield until DMA done: while((DCH0CON & Chn_busy) ){};
        
        //Put in a yield until v_in[] is filled. Will use a variable "done" to signal done
        PT_WAIT_UNTIL(pt, done == 1);
        finished = 0;
        // 
        // profile fft time
        WriteTimer4(0);
        
        // compute and display fft
        // load input array
        for (sample_number=0; sample_number<nSamp-1; sample_number++){
            // window the input and perhaps scale it
            fr[sample_number] = multfix14(v_in[sample_number], window[sample_number]); 
            fi[sample_number] = 0 ;
        }
        
        // do FFT
        FFTfix(fr, fi, LOG2_N_WAVE);
        // get magnitude and log
        // The magnitude of the FFT is approximated as: 
        //   |amplitude|=max(|Re|,|Im|)+0.4*min(|Re|,|Im|). 
        // This approximation is accurate within about 4% rms.
        // https://en.wikipedia.org/wiki/Alpha_max_plus_beta_min_algorithm
        for (sample_number = 0; sample_number < nPixels; sample_number++) {  
            // get the approx magnitude
            fr[sample_number] = abs(fr[sample_number]); //>>9
            fi[sample_number] = abs(fi[sample_number]);
            // reuse fr to hold magnitude
            fr[sample_number] = max(fr[sample_number], fi[sample_number]) + 
                    multfix14(min(fr[sample_number], fi[sample_number]), zero_point_4); 
            
            // reuse fr to hold log magnitude
            // shifting finds most significant bit
            // then make approxlog  = ly + (fr-y)./(y) + 0.043;
            // BUT for an 8-bit approx (4 bit ly and 4-bit fraction)
            // ly 1<=ly<=14
            // omit the 0.043 because it is too small for 4-bit fraction
            
            sx = fr[sample_number];
            y=1; ly=0;
            while(sx>1) {
               y=y*2; ly=ly+1; sx=sx>>1;
            }
            // shift ly into upper 4-bits as integer part of log
            // take bits below y and shift into lower 4-bits
            fr[sample_number] = ((ly)<<4) + ((fr[sample_number]-y)>>(ly-4) ) ;
            // bound the noise at low amp
            if(fr[sample_number]<log_min) fr[sample_number] = log_min;
        }
        
        // timer 4 set up with prescalar=8, 
        // hence mult reading by 8 to get machine cycles
//        sprintf(buffer, "FFT cycles %d", (ReadTimer4())*8);
//        printLine2(0, buffer, ILI9340_WHITE, ILI9340_BLACK);
        
        // Display on TFT
        // erase, then draw
//        for (sample_number = 0; sample_number < nPixels; sample_number++) {
////            tft_drawPixel(sample_number, pixels[sample_number], ILI9340_BLACK);
//            pixels[sample_number] = -fr[sample_number] + 200 ;
////            tft_drawPixel(sample_number, pixels[sample_number], ILI9340_WHITE);
//            // reuse fr to hold magnitude 
//
//          
//        }    
        // PRINT OUT TO PUTTY
        finished = 1;
//        for (sample_number = 0; sample_number < nPixels; sample_number++) {
//            
//            //print out values at one x
//            cursor_pos(2+sample_number,1);
//            //pixels[sample_number] = -fr[sample_number] + 200;
//            sprintf(PT_send_buffer,"sn=%d value=%d\n", sample_number, fr[sample_number]);
//            PT_SPAWN(pt, &pt_DMA_output, PT_DMA_PutSerialBuffer(&pt_DMA_output) );
//            //no zeroes
//        }
        
//        
//        int j;
//        for (j=0; j<20; j++){
//            cursor_pos(2+j,1);
//            sprintf(PT_send_buffer,"j=%d pixels[j]=%d\n", j,  pixels[j]);
//            PT_SPAWN(pt, &pt_DMA_output, PT_DMA_PutSerialBuffer(&pt_DMA_output) );
//        }
        // NEVER exit while
      } // END WHILE(1)
  PT_END(pt);
} // FFT thread

// === Main  ===================================================================
void main(void) {
    // Configure the device for maximum performance but do not change the PBDIV
	// Given the options, this function will change the flash wait states, RAM
	// wait state and enable prefetch cache but will not change the PBDIV.
	// The PBDIV value is already set via the pragma FPBDIV option above..
//	SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
   
    
    ANSELA = 0; ANSELB = 0; 
//    INTDisableInterrupts();
    
 //   rtcc_init();
    matrix_init(FALSE);
//    ir_init();
    
    INTEnableSystemMultiVectoredInt();
    
//    // timer 3 setup for adc trigger  ==============================================
//    // Set up timer3 on,  no interrupts, internal clock, prescalar 1, compare-value
//    // This timer generates the time base for each ADC sample. 
//    // works at ??? Hz
//    #define sample_rate 500000
//    // 40 MHz PB clock rate
//    #define timer_match 40000000/sample_rate
//    OpenTimer3(T3_ON | T3_SOURCE_INT | T3_PS_1_1, timer_match); 
//    
//    //=== DMA Channel 0 transfer ADC data to array v_in ================================
//    // Open DMA Chan1 for later use sending video to TV
//    #define DMAchan0 0
//	DmaChnOpen(DMAchan0, 0, DMA_OPEN_DEFAULT);
//    DmaChnSetTxfer(DMAchan0, (void*)&ADC1BUF0, (void*)v_in, 2, nSamp*2, 2); //512 16-bit integers
//    DmaChnSetEventControl(DMAchan0, DMA_EV_START_IRQ(28)); // 28 is ADC done
//    // ==============================================================================
    
    
    
    
    
    
  //  matrix_fillScreen(COLOR565_BLUE);
    
//    matrix_drawPixel(2, 5, COLOR565_BLUE);
//    matrix_drawLine(0, 0, 0, 10,COLOR565_BLUE );
//    matrix_drawLine(15, 0, 15, 10,COLOR565_MAGENTA );
//    matrix_drawLine(10, 0, 10, 10,COLOR565_YELLOW );
//    matrix_drawLine(20, 0, 20, 10,COLOR565_GREEN );
//    matrix_drawLine(25, 0, 25, 14,COLOR565_WHITE );
//    matrix_drawLine(31, 0, 31, 10,COLOR565_CYAN );
//    matrix_drawLine(27, 0, 27, 15,COLOR565_WHITE ); 
//    matrix_drawLine(7, 0, 7, 11,COLOR565_RED );
//    PT_INIT(&pt_update_matrix);
//    PT_INIT(&pt_serial);

    
    // -------------------------ADC----------------
  // the ADC ///////////////////////////////////////
        // configure and enable the ADC
	CloseADC10();	// ensure the ADC is off before setting the configuration

	// define setup parameters for OpenADC10
	// Turn module on | ouput in integer | trigger mode auto | enable autosample
        // ADC_CLK_AUTO -- Internal counter ends sampling and starts conversion (Auto convert)
        // ADC_AUTO_SAMPLING_ON -- Sampling begins immediately after last conversion completes; SAMP bit is automatically set
        // ADC_AUTO_SAMPLING_OFF -- Sampling begins with AcquireADC10();
        #define PARAM1  ADC_FORMAT_INTG16 | ADC_CLK_AUTO | ADC_AUTO_SAMPLING_OFF //

	// define setup parameters for OpenADC10
	// ADC ref external  | disable offset test | disable scan mode | do 1 sample | use single buf | alternate mode off
	#define PARAM2  ADC_VREF_AVDD_AVSS | ADC_OFFSET_CAL_DISABLE | ADC_SCAN_OFF | ADC_SAMPLES_PER_INT_1 | ADC_ALT_BUF_OFF | ADC_ALT_INPUT_OFF
        //
	// Define setup parameters for OpenADC10
        // use peripherial bus clock | set sample time | set ADC clock divider
        // ADC_CONV_CLK_Tcy2 means divide CLK_PB by 2 (max speed)
        // ADC_SAMPLE_TIME_5 seems to work with a source resistance < 1kohm
      #define PARAM3 ADC_CONV_CLK_PB | ADC_SAMPLE_TIME_5 | ADC_CONV_CLK_Tcy2 //ADC_SAMPLE_TIME_15| ADC_CONV_CLK_Tcy2

	// define setup parameters for OpenADC10
	// set AN11 and  as analog inputs
	#define PARAM4	ENABLE_AN11_ANA // mapped to RB13

	// define setup parameters for OpenADC10
	// do not assign channels to scan
	#define PARAM5	SKIP_SCAN_ALL

	// use ground as neg ref for A | use AN11 for input A     
	// configure to sample AN11
	SetChanADC10( ADC_CH0_NEG_SAMPLEA_NVREF | ADC_CH0_POS_SAMPLEA_AN11 ); // configure to sample AN1 
	OpenADC10( PARAM1, PARAM2, PARAM3, PARAM4, PARAM5 ); // configure ADC using the parameters defined above

	EnableADC10(); // Enable the ADC
    //for adc,using pin 24 = RB13 = AN11
    
    //TODO: 
    
  ///////////////////////////////////////////////////////
    
//    TIMER used for ISR # 44.1 kHz (audio sampling)/5000 cycles = 8 kHz
    OpenTimer5(T5_ON | T5_SOURCE_INT | T5_PS_1_1, 5000);

    // set up the timer interrupt with a priority of 2
    ConfigIntTimer5(T5_INT_ON | T5_INT_PRIOR_2);
    mT5ClearIntFlag(); // and clear the interrupt flag
    
    
  //-------------------ADC------------------------
    
        // SCK2 is pin 26 
    // SDO2 (MOSI) is in PPS output group 2, could be connected to RB5 which is pin 14
//    PPSOutput(2, RPB5, SDO2);
  // set up DAC on big board
  // timer interrupt //////////////////////////
    // Set up timer2 on,  interrupts, internal clock, prescalar 1, toggle rate
    // at 30 MHz PB clock 60 counts is two microsec
    // 400 is 100 ksamples/sec
    // 2000 is 20 ksamp/sec
    // set up the timer interrupt with a priority of 2
    // control CS for DAC
    mPORTBSetPinsDigitalOut(BIT_4);
    mPORTBSetBits(BIT_4);
    // SCK2 is pin 26 
    // SDO2 (MOSI) is in PPS output group 2, could be connected to RB5 which is pin 14
//    PPSInput(2, U2RX, RPA1);
//    PPSOutput(4, RPB10, U2TX);

    // 16 bit transfer CKP=1 CKE=1
    // possiblCHANNEL2, SPI_OPEN_ON | SPI_OPEN_MODE16 | SPI_OPEN_MSTEN | SPI_OPEN_CKE_REV , 4);
    
    
//    PT_INIT(&pt_animate);es SPI_OPEN_CKP_HIGH;   SPI_OPEN_SMP_END;  SPI_OPEN_CKE_REV
    // For any given peripherial, you will need to match these
    // NOTE!! IF you are using the port expander THEN
    // -- clk divider must be set to 4 for 10 MHz
   // SpiChnOpen(SPI_CHANNEL2, SPI_OPEN_ON | SPI_OPEN_MODE16 | SPI_OPEN_MSTEN | SPI_OPEN_CKE_REV , 4);
    
    
    PT_setup();
    
    
//    
//    
//    // timer 4 setup for profiling code ===========================================
//    // Set up timer2 on,  interrupts, internal clock, prescalar 1, compare-value
//    // This timer generates the time base for each horizontal video line
    OpenTimer4(T4_ON | T4_SOURCE_INT | T4_PS_1_8, 0xffff);
        
    // === init FFT data =====================================
    // one cycle sine table
    //  required for FFT
    int ii;
    for (ii = 0; ii < N_WAVE; ii++) {
        Sinewave[ii] = float2fix14(sin(6.283 * ((float) ii) / N_WAVE)*0.5);
        window[ii] = float2fix14(1.0 * (1.0 - cos(6.283 * ((float) ii) / (N_WAVE - 1))));
        //window[ii] = float2fix(1.0) ;
    }
    // ========================================================
    // init the threads
    PT_INIT(&pt_fft);
    PT_INIT(&pt_animate);
    //PT_INIT(&pt_print);
   // PT_INIT(&pt_animate);
//    #define COLOR565_BLACK   0x0000
//#define	COLOR565_BLUE    0x001F
//#define	COLOR565_RED     0xF800
//#define COLOR565_ORANGE  0xFB40
//#define	COLOR565_GREEN   0x0CE0//0x07E0 
//#define COLOR565_GREEN2  0xB6F1
//#define COLOR565_CYAN    0x07FF
//#define COLOR565_MAGENTA 0xF81F
//#define COLOR565_LIPSTICK 0xE1CF
//#define COLOR565_PURPLE  0x9957
//#define COLOR565_YELLOW  0xFFE0
//#define COLOR565_YELLOW2 0xFFC8
//#define COLOR565_WHITE   0xFFFF
    
//    UINT16 colors[15] = {COLOR565_RED, COLOR565_ORANGE, COLOR565_YELLOW,COLOR565_GREEN, COLOR565_LIGHT_GREEN,
//                            COLOR565_CYAN, COLOR565_BLUE, COLOR565_NICE_BLUE, COLOR565_PURPLE, COLOR565_PINK, COLOR565_MAGENTA,
//                            COLOR565_LIPSTICK, COLOR565_PERIWINKLE, COLOR565_WHITE, COLOR565_ICE };
//    int red = 15;
//    int blue = 3;
//    int green = 0;
//    
//    
//    
//    //blue+(green<<5)+(red<<11))
//    
//    int p;
//    for (p =0; p < 15; p++){
//        matrix_fillScreen(colors[p]);
//        delay_ms(1000);
//        //blue++;
////        matrix_fillScreen(COLOR565_PURPLE);
////        delay_ms(500);
////        matrix_fillScreen(COLOR565_MAGENTA);
////        delay_ms(500);
////        matrix_fillScreen(COLOR565_PERIWINKLE);
////        delay_ms(500);
////        matrix_fillScreen(COLOR565_LIPSTICK);
////        delay_ms(500);
//    }
//            
        
        
        
        
    while (TRUE) {
////        PT_SCHEDULE(protothread_update_matrix(&pt_update_matrix));
//        PT_SCHEDULE(protothread_serial(&pt_serial));
       PT_SCHEDULE(protothread_animate(&pt_animate));
//          PT_SCHEDULE(protothread_print(&pt_print));
        PT_SCHEDULE(protothread_fft(&pt_fft));
    }
}
// === end  ======================================================
