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
#include <stdio.h>
#include <msp430.h>

#define Num_of_Results   8

//Using single values to store the voltage measurments
uint16_t Ch0results;
uint16_t Ch1results;
uint16_t i = 0;

//Switching direction
//1: Set direction, assume default if non present / no command(Do not turn on power supply or relays at this point)
    //Red is +, black is -
//2. Turn off 12v to relay MOSFET, never opens
//3. Relay, set to direction of current (forward / backward, from command)
//4. After done, set 12V


void main(void) {
    // Stop WDT
    WDT_hold(WDT_BASE);
    printf("Hello World!\n");

    //P1.0, P1.1 controls output relays
    P1DIR = (1 << 0) | (1 << 1);
    P1OUT = 0;

    //P1.5 controls an LED

    P1DIR |= (1 << 5);
    P1OUT = 1 << 5;
    //P1OUT = 0;

    //P1.0:

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
    SD24_startConverterConversion(SD24_BASE, SD24_CONVERTER_1);
    // Enter LPM0 w/ interrupts
    //__bis_SR_register(0b10000 | GIE); //LPM0 = 0x10 = 0b10000 (Bit mask to disable CPU)


    for (;;){

        // Save CH0 results (clears IFG)
       Ch0results = SD24_getHighWordResults(SD24_BASE, SD24_CONVERTER_0);
       // Save CH1 results (clears IFG)
       Ch1results = SD24_getHighWordResults(SD24_BASE, SD24_CONVERTER_1);

       printf("%d, %d\n", Ch0results, Ch1results);

        P1OUT ^= (1 << 5);
        P1OUT ^= 1 << 0;
        for (i = 0; i < 40000; i++);
    }


}

//Interrupt vector for I2C communication
//NOTE:
    //I2C has higher priority interrupt compared to SD24
#pragma vector=USCI_B0_VECTOR
//Check example code for how to format intterupt
__interrupt void USCIB0_ISR(void) {
    switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG)) {
        case USCI_NONE: break;
        case USCI_I2C_UCALIFG: break;
        case USCI_I2C_UCNACKIFG: break;
        case USCI_I2C_UCSTTIFG: break;
        case USCI_I2C_UCSTPIFG: break;
        case USCI_I2C_UCRXIFG3: break;
        case USCI_I2C_UCTXIFG3: break;
        case USCI_I2C_UCRXIFG2: break;
        case USCI_I2C_UCTXIFG2: break;
        case USCI_I2C_UCRXIFG1: break;
        case USCI_I2C_UCTXIFG1: break;
        case USCI_I2C_UCRXIFG0:
            //RXData[RXDataIndex++] = EUSCI_B_I2C_slaveGetData(EUSCI_B0_BASE);

            // Reset index if at end of array
            //Writes over old data
            //if(RXDataIndex == NUM_OF_RX_BYTES) {
            //    RXDataIndex = 0;
            //}

            break;
        case USCI_I2C_UCTXIFG0: break;
        case USCI_I2C_UCBCNTIFG: break;
        case USCI_I2C_UCCLTOIFG: break;
        case USCI_I2C_UCBIT9IFG: break;
        default: break;
    }
}

