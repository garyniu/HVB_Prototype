//includes / Header files
#include "driverlib.h"
#include <msp430i2021.h>
#include <stdio.h>
#include <msp430.h>

//I2C definitions
#define MSP430I2021_SLAVE_ADDRESS       0x48
#define NUM_OF_TX_BYTES     21
#define NUM_OF_RX_BYTES     26

static volatile uint8_t RXData[NUM_OF_RX_BYTES];
static volatile uint8_t RXDataIndex;

static volatile uint8_t TXData[NUM_OF_TX_BYTES] = {5, 5, 3, 4};    //Test data
static volatile uint8_t TXDataIndex;

//TODO
//Put other remaining I2C definitions here



//SD24 definitisons
#define Num_of_Results   8

//Using single values to store the voltage measurments
uint16_t Ch0results;
uint16_t Ch1results;
uint16_t i = 0;

//Switching direction
//1: Set direction, assume default if non present / no command (Do not turn on power supply or relays at this point)
    //Red is +, black is -
//2. Turn off 12v to relay MOSFET, never opens
//3. Relay, set to direction of current (forward / backward, from command)
//4. After done, set 12V


//Regular Definitions
bool InterruptWOccured = false;

//Values to be stored in flash
int polarity = 0; // 0 = Default direction, 1 = Reversed direction
uint16_t currentLimit = 0;
uint16_t vOffset1 = 0;
uint16_t vOffset2 = 0;
uint16_t vOffset3 = 0;

//16 byts of RFID data
uint8_t rfidData[16];

int temp=0;
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

void init_i2c(void){
    
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
}

void Relay_Polarity(int polarity){
    if (!polarity){
        //Default direction
            //Make sure to disable 1.4 first before 
            //changing direction of the relays
    } else {
        //Inverted direction
    }
}

void main(void) {
    // Stop WDT
    WDT_hold(WDT_BASE);
    printf("Hello World!\n");


    //Init ports
    P1OUT = 0;
    //P1.0, P1.1: Relays
    P1DIR = (1 << 0) | (1 << 1);

    //P1.4: 12V to enable relay switches
    P1DIR |= (1 << 4);
    P1OUT |= (1 << 4); //Enable relays

    //P1.5: LED
    P1DIR |= (1 << 5);
    


    //Other inits
    //Initalize SD24's 2 analogue channels
    init_sd24();

    //Load values from flash
    //flash_read();

    //Init I2C
    init_i2c();


    for (;;){

        //Check if interrupt occured, operations and flash if needed
        if (InterruptWOccured){
            //Clear it
            InterruptWOccured = false;

            //Polarity
                //Check if polarity has changed from value in flash:
                //If so, change local value and flash, and

            //if (flash_polarity != polarity){
                polarity = RXData[0];
                //flash_polarity = RXData[0]; //Put it in flash (Function, instead of a value)
                Relay_Polarity(polarity); //Switch direction of relays

            //}
            

            //Current Limit
                //First, append the high byte to the rear of the low byte (hint: use bitwise operators)
                //Ex:   RXData[1] = 10010101, High byte
                //      RXData[2] = 00101000, Low byte
                //          Both of these bytes in the array represent one half of a 16 bit integer
                //      So, the High byte needs to be shifted over 8 bits
                //      And the low byte is added in through the OR bitwise operator
                //  Visualized:
                //      1. Shift High Byte in 16 bit integer:       xxxxxxxx10010101 << 8
                //                                                = 1001010100000000
                //      2. Use Bitwise to add Low Byte:             1001010100000000 | 00101000
                //                                                = 1001010100101000

            currentLimit = (RXData[1] << 8) | RXData[2];
            //if(currentLimit != flash_currentLimit)
            //flash_currentLimit = currentLimit;
                //No other adjustment needed; polling will access the currentLimit variable
                //and it will be auto updated.

            //Voltage Offset
                //Adjust the voltage offset for different points / regions, as the voltage increases
                //Still High / Low byte, so same process as above
                    //Values are used automatically in calculations, so no need to change anything
                    //other than flash

            vOffset1 = (RXData[3] << 8) | RXData[4];
            vOffset2 = (RXData[5] << 8) | RXData[6];
            vOffset3 = (RXData[7] << 8) | RXData[8];

                //Check each voltage offset independently to make sure it changed
                //before writing to flash
            //if (shit)
            //...
            //...


            //byte10-26 RFID
            //TO BE IMPLEMENTED

        }

        // Reads data from ch0, before relays?
       Ch0results = SD24_getHighWordResults(SD24_BASE, SD24_CONVERTER_0);
       // Reads data from ch1, after relays?
       Ch1results = SD24_getHighWordResults(SD24_BASE, SD24_CONVERTER_1);

       //Make sure to test with like 500 v to test shutoff
       //VO_EN3 low, disables

       //en1, en2, for polarity switch, disable en3,
       //chech high voltage
       //printf("%d, %d\n", Ch0results, Ch1results);

       //printf("%d, %d, %d\n", RXData[2], RXData[3], RXData[4]);

        temp+=1;
        //printf("%d\n", temp);

        if (temp == 20000){
            temp = 0;
            P1OUT ^= (1 << 5);
            P1OUT ^= 1 ;
        }
    
        
    }

}

//Interrupt vector for I2C communication
//NOTE:
    //I2C has higher priority interrupt compared to SD24
#pragma vector=USCI_B0_VECTOR
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

            //Puts all 26 received bits from master, and puts it into RXData, which increments it each time the data is full
            RXData[RXDataIndex++] = EUSCI_B_I2C_slaveGetData(EUSCI_B0_BASE);

            // Reset index if at end of array
            if(RXDataIndex == NUM_OF_RX_BYTES) {
                RXDataIndex = 0;
                //Indicates that there has been a write, and that settings need to change
                InterruptWOccured = true;
            }


            break;
        case USCI_I2C_UCTXIFG0:

            //Test, flashes the led with each byte of data
            EUSCI_B_I2C_slavePutData(EUSCI_B0_BASE, TXData[TXDataIndex++]);

            // Disable TX if all data is sent
            if(TXDataIndex == NUM_OF_TX_BYTES) {
                EUSCI_B_I2C_disableInterrupt(EUSCI_B0_BASE,
                                             EUSCI_B_I2C_TRANSMIT_INTERRUPT0);
            }
            break;

        case USCI_I2C_UCBCNTIFG: break;
        case USCI_I2C_UCCLTOIFG: break;
        case USCI_I2C_UCBIT9IFG: break;
        default: break;
    }
}

