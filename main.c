//  MSP430i20xx SD24 Example 01 - SD24, Continuous Conversion on a Group of 3 Channels
//
//  Description: This program uses the SD24 module to perform continuous
//  conversions on a group of channels (0, 1 and 2). A SD24 interrupt occurs
//  whenever the conversions have completed.
//
//  Test by applying voltages to the 3 input channels and setting a breakpoint
//  at the indicated line. Run program until it reaches the breakpoint, then use
//  the debugger's watch window to view the conversion results.
//
//  Results (upper 16 bits only) are stored in three arrays, one for each channel.
//
//  ACLK = 32kHz, MCLK = SMCLK = Calibrated DCO = 16.384MHz, SD_CLK = 1.024MHz
//  * Ensure low_level_init.c is included when building/running this example *
//
//  Notes: For minimum Vcc required for SD24 module - see datasheet
//         100nF cap btw Vref and AVss is recommended when using 1.2V ref
//******************************************************************************
#include "driverlib.h"
#include <msp430i2021.h>

#define Num_of_Results   8

//Using single values to store the voltage measurments
uint16_t Ch0results;
uint16_t Ch1results;
uint16_t i = 0;

void main(void) {
    // Stop WDT
    WDT_hold(WDT_BASE);
    P1OUT = 0;
    P1DIR = 0;

    // Internal ref
    SD24_init(SD24_BASE, SD24_REF_INTERNAL);

    //Group with Channel 0
    SD24_initConverterAdvancedParam param = {0};
    param.converter = SD24_CONVERTER_0;
    param.conversionMode = SD24_CONTINUOUS_MODE;
    param.groupEnable = SD24_NOT_GROUPED;
    param.inputChannel = SD24_INPUT_CH_ANALOG;
    param.dataFormat = SD24_DATA_FORMAT_2COMPLEMENT;
    param.interruptDelay = SD24_FOURTH_SAMPLE_INTERRUPT;
    param.oversampleRatio = SD24_OVERSAMPLE_256;
    param.gain = SD24_GAIN_1;
    SD24_initConverterAdvanced(SD24_BASE, &param);

    //Group with Channel 1
    param.converter = SD24_CONVERTER_1;
    param.conversionMode = SD24_CONTINUOUS_MODE;
    param.groupEnable = SD24_NOT_GROUPED;
    param.inputChannel = SD24_INPUT_CH_ANALOG;
    param.dataFormat = SD24_DATA_FORMAT_2COMPLEMENT;
    param.interruptDelay = SD24_FOURTH_SAMPLE_INTERRUPT;
    param.oversampleRatio = SD24_OVERSAMPLE_256;
    param.gain = SD24_GAIN_1;
    SD24_initConverterAdvanced(SD24_BASE, &param);

    // Enable interrupt
    SD24_enableInterrupt(SD24_BASE, SD24_CONVERTER_1, SD24_CONVERTER_INTERRUPT);

    // Delay ~200us for 1.2V ref to settle
    __delay_cycles(3200);

    // Start conversion
    SD24_startConverterConversion(SD24_BASE, SD24_CONVERTER_2);
    // Enter LPM0 w/ interrupts
    __bis_SR_register(GIE); //removed LMP0 |, to test for active running

    int b;
    while (1){
        b++;
    }


}


#pragma vector=SD24_VECTOR

//Function defined, with no return value and no parameters
__interrupt void SD24_ISR(void) {

    switch (__even_in_range(SD24IV,SD24IV_SD24MEM1)) {
        case SD24IV_NONE: break;
        case SD24IV_SD24OVIFG: break;
        case SD24IV_SD24MEM0: break;
        case SD24IV_SD24MEM1:
                   // Save CH0 results (clears IFG)
                   Ch0results = SD24_getHighWordResults(SD24_BASE, SD24_CONVERTER_0);
                   // Save CH1 results (clears IFG)
                   Ch1results = SD24_getHighWordResults(SD24_BASE, SD24_CONVERTER_1);



                       __no_operation();           // SET BREAKPOINT HERE

                   break;
        default: break;
    }
}
