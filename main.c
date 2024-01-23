//iohsdoifaodihfaiosnvonaids
#include "driverlib.h"
#include <msp430i2021.h>
#include <stdio.h>
#include <msp430.h>

//I2C definitions
#define MSP430I2021_SLAVE_ADDRESS       0x48
#define NUM_OF_TX_BYTES     4
#define NUM_OF_RX_BYTES     4

static volatile uint8_t RXData[NUM_OF_RX_BYTES];
static volatile uint8_t RXDataIndex;

//TODO
//Put other remaining definitions here



//SD24 definitisons
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


//Functions declaraions
void init_sd24(void){
    //FOR SD24:
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
        return;
}


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

    //Initalize SD24's 2 analogue channels
    init_sd24();


    //Test code for I2C to follow:
    EUSCI_B_I2C_initSlaveParam i2cConfig = {
            MSP430I2021_SLAVE_ADDRESS,                              // Slave Address
            EUSCI_B_I2C_OWN_ADDRESS_OFFSET0,
            EUSCI_B_I2C_OWN_ADDRESS_ENABLE
        };

        WDT_hold(WDT_BASE);

        // Setting the DCO to use the internal resistor. DCO will be at 16.384MHz
        CS_setupDCO(CS_INTERNAL_RESISTOR);


        // Setting P1.6 and P1.7 as I2C pins
        GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
                                                   GPIO_PIN6 | GPIO_PIN7,
                                                   GPIO_PRIMARY_MODULE_FUNCTION);

        // Setting up I2C communication as slave
        EUSCI_B_I2C_initSlave(EUSCI_B0_BASE, &i2cConfig);

        // Set slave in receive mode
        EUSCI_B_I2C_setMode(EUSCI_B0_BASE, EUSCI_B_I2C_RECEIVE_MODE);

        // Enable the module for operation
        EUSCI_B_I2C_enable(EUSCI_B0_BASE);

        // Enable needed I2C interrupts
        EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE,
                                       EUSCI_B_I2C_RECEIVE_INTERRUPT0);
        EUSCI_B_I2C_enableInterrupt(EUSCI_B0_BASE,
                                    EUSCI_B_I2C_RECEIVE_INTERRUPT0);



    // Enter LPM0 w/ interrupts
    //__bis_SR_register(0b10000 | GIE); //LPM0 = 0x10 = 0b10000 (Bit mask to disable CPU)


    for (;;){

        // Reads data from ch0, before relays?
       Ch0results = SD24_getHighWordResults(SD24_BASE, SD24_CONVERTER_0);
       // Reads data from ch1, after relays?
       Ch1results = SD24_getHighWordResults(SD24_BASE, SD24_CONVERTER_1);

       //Make sure to test with like 500 v to test shutoff
       //VO_EN3 low, disables

       //en1, en2, for polarity switch, disable en3,
       //chech high voltage
       printf("%d, %d\n", Ch0results, Ch1results);

        P1OUT ^= (1 << 5); //flips the led on and off
        //P1OUT ^= 1 << 0; //FLips the relay
        for (i = 0; i < 40000; i++);
    }


}

//Interrupt vector for I2C communication
//NOTE:
    //I2C has higher priority interrupt compared to SD24
#pragma vector=USCI_B0_VECTOR
//Check example code for how to format intterupt
//No TX code at this moment
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
            RXData[RXDataIndex++] = EUSCI_B_I2C_slaveGetData(EUSCI_B0_BASE);

            // Reset index if at end of array
            //Writes over old data
            if(RXDataIndex == NUM_OF_RX_BYTES) {
                RXDataIndex = 0;
            }
            //can add printf statement here

            break;
        case USCI_I2C_UCTXIFG0: break;
        case USCI_I2C_UCBCNTIFG: break;
        case USCI_I2C_UCCLTOIFG: break;
        case USCI_I2C_UCBIT9IFG: break;
        default: break;
    }
}

