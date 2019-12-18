#include "blocking_delay.h"

#include "config.h"

#define dTime_ms PBCLK/2000
#define dTime_us PBCLK/2000000

void delay_ms(unsigned long i){
    /* Create a software delay about i ms long
     * Parameters:
     *      i:  equal to number of milliseconds for delay
     * Returns: Nothing
     * Note: Uses Core Timer. Core Timer is cleared at the initialiazion of
     *      this function. So, applications sensitive to the Core Timer are going
     *      to be affected
     */
    unsigned int j;
    j = dTime_ms * i;
    WriteCoreTimer(0);
    while (ReadCoreTimer() < j);
}

void delay_us(unsigned long i){
    /* Create a software delay about i us long
     * Parameters:
     *      i:  equal to number of microseconds for delay
     * Returns: Nothing
     * Note: Uses Core Timer. Core Timer is cleared at the initialization of
     *      this function. So, applications sensitive to the Core Timer are going
     *      to be affected
     */
    unsigned int j;
    j = dTime_us * i;
    WriteCoreTimer(0);
    while (ReadCoreTimer() < j);
}
