// includes / Header files
#include "driverlib.h"
#include <msp430i2021.h>
#include <stdio.h>
#include <msp430.h>

// I2C definitions
#define MSP430I2021_SLAVE_ADDRESS 0x48
#define NUM_OF_TX_BYTES 50
#define NUM_OF_RX_BYTES 50

//Flash definition
#define FLOC 0x1060
//Location in flash

// 8ms Period Calculation:
// SMCLK = DCO = 16.384 MHz
// TACLK = SMCLK / 40 = 0.4096 MHz
// 5ms / (1 / 0.4096MHz) = 2048
#define TIMER_PERIOD        205

// 75% Duty Cycle Calculation:
// TIMER_PERIOD * 0.75 = 24576
#define TIMER_DUTY_CYCLE    1024

//1536

static volatile uint8_t RXData[NUM_OF_RX_BYTES];
static volatile uint8_t RXDataIndex;
static volatile uint8_t RXInProgress; // flag to indicate RX command receiving in progress.

// master to send to MSP430
// byte1- Polarity 00, normal (default) 01, reversed
// byte2- Current limit high byte
// byte3- Current limit low byte  in uA (0-2000)
// byte4- 0-1000V voltage offset high byte
// byte5- 0-1000V voltage offset low byte
// byte6- 1000V-2000V voltage offset high byte
// byte7- 1000V-2000V voltage offset low byte
// byte8- 2000V-3000V voltage offset high byte
// byte9- 2000V=3000V voltage offset low byte
// byte10-26 RFID

static volatile uint8_t TXData[NUM_OF_TX_BYTES] = {}; // Test data
static volatile uint8_t TXDataIndex;

// slave to send out data in following format:
// byte0: voltage high byte
// byte1: voltage low byte
// byte2: current high byte
// byte3: current low byte
// byte 4-20: RFID send 0 if there is no valid RFID programmed.

uint8_t CMD1Data[NUM_OF_RX_BYTES]; // CMD buffer. once received RX data copied to CMDbuffer, RXData will be cleared and wait for next I2C.
uint8_t CMD1Length;                // total length of received command. command length can be 1 to 26.
uint8_t CMD2Data[NUM_OF_RX_BYTES]; // CMD buffer. once received RX data copied to CMDbuffer, RXData will be cleared and wait for next I2C.
uint8_t CMD2Length;                // total length of received command. command length can be 1 to 26.

uint8_t *CMDDataPtr;
uint8_t CMDLength;
uint8_t CMDIndex; // CMD buffer index

uint16_t i = 0; // Index for I2C Interrupts

// SD24 definitions
#define Num_of_Results 8

// SD24 full scale range is +-928mV. single ended is 0-928mv. however, the FSR(7FFFFF)= 1.158V. To convert to mv. result / 7FFFFF * 1158mv
// current in V: hv_v X0.301/1000.301=  V ( measured in mv) so hv_v = result /7FFFFF *1158 (mv)/0.301X1000.301 = (result X1158 /7FFFFF)*3.33259
#define HV_V_RATIO 3.323259

// ADC number / ratio = current in 0.1uA. Calculated. 3.16K
// current in uA: C(ua) *3.16K = V ( measured in mv) so C = V (mv) /3.16(kR) = result /7FFFFF *1158 (mv)/3.16 = result X1158 /(7FFFFX3.16)
// example, read value 18632F =1598255
#define HV_C_RATIO 0.316

// Using single values to store the voltage measurements
static volatile uint32_t Ch0results;
static volatile uint32_t Ch1results;

static volatile float hv_v; // in Voltage
static volatile float hv_c; // in 0.1uA

static volatile uint8_t hv_update; // HV ADC value updated

// Flash Values
uint8_t polarity = 0;             // 0 = Default direction, 1 = Reversed direction
uint16_t currentLimit = 1000; // value in .01ua
uint16_t vOffset1 = 0;
uint16_t vOffset2 = 0;
uint16_t vOffset3 = 0;

// 16 bytes of RFID data
uint8_t rfidData[16];

// Values for Protection
bool ProtectionFlag = false;

// Functions declarations
void init_sd24(void)
{
    // FOR SD24:
    //  Internal ref
    SD24_init(SD24_BASE, SD24_REF_INTERNAL);

    // Group with Channel 0
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

    // Group with Channel 1
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
    // TODO: Implement interrupts for SD24, instead of polling
    SD24_enableInterrupt(SD24_BASE, SD24_CONVERTER_0, SD24_CONVERTER_INTERRUPT);
    SD24_enableInterrupt(SD24_BASE, SD24_CONVERTER_1, SD24_CONVERTER_INTERRUPT);
    // Delay ~200us for 1.2V ref to settle
    __delay_cycles(3200);

    // Start conversion
    SD24_startConverterConversion(SD24_BASE, SD24_CONVERTER_0);
    SD24_startConverterConversion(SD24_BASE, SD24_CONVERTER_1);
    return;
}

void init_gpio(void)
{
    // P1.0 VO_EN1 output, default low
    // P1.1 VO_EN2 output, default low
    // P1.4 VO_EN3 output, default low


    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);


    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN4);

}

void init_i2c(void)
{

    EUSCI_B_I2C_initSlaveParam i2cConfig = {
        MSP430I2021_SLAVE_ADDRESS, // Slave Address
        EUSCI_B_I2C_OWN_ADDRESS_OFFSET0,
        EUSCI_B_I2C_OWN_ADDRESS_ENABLE};

    // WDT_hold(WDT_BASE);

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
                               EUSCI_B_I2C_RECEIVE_INTERRUPT0 | EUSCI_B_I2C_TRANSMIT_INTERRUPT0 |
                                   EUSCI_B_I2C_STOP_INTERRUPT);

    EUSCI_B_I2C_enableInterrupt(EUSCI_B0_BASE,
                                EUSCI_B_I2C_RECEIVE_INTERRUPT0 | EUSCI_B_I2C_TRANSMIT_INTERRUPT0 |
                                    EUSCI_B_I2C_STOP_INTERRUPT);

    // Stay active w/ interrupts
    __bis_SR_register(GIE); //
}

void init_flash(void)
{
    FlashCtl_setupClock(390095, 16384000, FLASHCTL_MCLK);

    polarity = *(uint8_t *)(FLOC + 16);
    currentLimit = *(uint8_t *)(FLOC + 17) | *(uint8_t *)(FLOC + 18) << 8;
    vOffset1 = *(uint8_t *)(FLOC + 19) | *(uint8_t *)(FLOC + 20) << 8;
    vOffset2 = *(uint8_t *)(FLOC + 21) | *(uint8_t *)(FLOC + 22) << 8;
    vOffset3 = *(uint8_t *)(FLOC + 23) | *(uint8_t *)(FLOC + 24) << 8;

    // First start / values contain garbage
    if (polarity != 0 || polarity != 1) {
        polarity = 0;
        currentLimit = 0;
        vOffset1 = 0;
        vOffset2 = 0;
        vOffset3 = 0;
    }

}

// erase segment might erase the flash! (512 bytes)
void write_rfid_flash(uint8_t* DataArr, uint8_t * FlashL )
{
    // Update / merge with write_flash, or else eraseSegment 
    // might delete all stored flash data
    FlashCtl_unlockInfo();
    //Any chance this might cause least significant bit to flip over? (Wrong order)
    if (FlashCtl_performEraseCheck (FlashL,16)==STATUS_FAIL){
         FlashCtl_eraseSegment (FlashL);
    }

    FlashCtl_write8(DataArr, FlashL, 16);


    FlashCtl_lockInfo();

}

void write_flash(){

    uint8_t * FlashL = FLOC + 16;

    FlashCtl_unlockInfo();
    //Any chance this might cause least significant bit to flip over? (Wrong order)
    if (FlashCtl_performEraseCheck (FlashL, 9)==STATUS_FAIL){
         FlashCtl_eraseSegment (FlashL);
    }


    FlashCtl_write8(&polarity, FlashL, 1);
    FlashCtl_write8((currentLimit | 0b11111111), FlashL + 1, 1);
    FlashCtl_write8(currentLimit | (0b11111111 << 8) >> 8, FlashL + 2, 1);

    FlashCtl_write8(vOffset1 | 0b11111111, FlashL + 3, 1);
    FlashCtl_write8(vOffset1 | (0b11111111 << 8) >> 8, FlashL + 4, 1);

    FlashCtl_write8(vOffset2 | 0b11111111, FlashL + 5, 1);
    FlashCtl_write8(vOffset2 | (0b11111111 << 8) >> 8, FlashL + 6, 1);

    FlashCtl_write8(vOffset3 | 0b11111111, FlashL + 7, 1);
    FlashCtl_write8(vOffset3 | (0b11111111 << 8) >> 8, FlashL + 8, 1);

    FlashCtl_lockInfo();

}

// Main Function
void main(void)
{

    // Temp values for buffering ADC Voltage and Current
    uint32_t temp_v;
    uint32_t temp_c;
    uint8_t data[16];

    int i;

    // Stop WDT
    WDT_hold(WDT_BASE);

    // Call Init Functions
    // Init Flash
     for (i=0;i<16;i++) {
         rfidData[i]=30+i;
     }

    init_flash();

    write_rfid_flash(rfidData, (uint8_t *) (FLOC+32) );

     for (i=0;i<16;i++) {
           data[i]=  * (uint8_t *) (FLOC+32+i);
       }



    // Read flash, to global variables
    write_flash();
    int b[30] = {};

    for ( i = 0; i < 30; i++){
        b[i] = *(uint8_t *) (FLOC + 16 + i);

    }

    // Init GPIO
    init_gpio();

    // Init I2C
    init_i2c();

    // Initalize SD24's 2 analogue channels
    init_sd24();

    Timer_A_outputPWMParam outputPWMConfig = {
            TIMER_A_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
            TIMER_A_CLOCKSOURCE_DIVIDER_8,      // SMCLK/64 (0x20) = 4.096 MHz
            10240,                       // 5ms
            TIMER_A_CAPTURECOMPARE_REGISTER_1,  // Output on TA0.1
            TIMER_A_OUTPUTMODE_RESET_SET,       // Generate PWM
            5120                 // 75% Duty Cycle
     };


    CS_setupDCO(CS_INTERNAL_RESISTOR);
    CS_initClockSignal(CS_SMCLK, CS_CLOCK_DIVIDER_16);

    // Setting up P1.5 as TA0.1 for PWM signal generation
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P1,
                                                GPIO_PIN5,
                                                GPIO_SECONDARY_MODULE_FUNCTION);

    // Configure the timer to use ACLK and interrupt on overflow
    Timer_A_outputPWM(TIMER_A0_BASE, &outputPWMConfig);

    // Define / Set GPIO Relays to polarity stored in flash
    // Merge with init_gpio function later
    if (polarity)
    {
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN1);
    }
    else
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);
    }

    // P1.4: 12V to enable relay switches
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);

    __bis_SR_register(GIE); //

    // Infinite loop
    while (true)
    {

        // If there is a new value from the ADC ready
        if (hv_update)
        {

            // copy to local
            // calculate voltage and current

            hv_v = (unsigned long long) Ch0results * 1158 / 0x7FFFFF;
            hv_v= hv_v * (float)HV_V_RATIO;

            hv_c = (unsigned long long) Ch1results * 1158 / 0x7FFFFF / (float)HV_C_RATIO;

            temp_v = (int)hv_v;
            temp_c = (int)hv_c;
            hv_update = 0; // Clear flag

            // Place into I2C buffer array, to TX to master
            //  Need to add a flag on when the new ADC value is available.
            if ((temp_v < 5000) && (temp_c < 3000))
            { // valid voltage reading
                __disable_interrupt(); // disable all interrupts --> GIE = 0 (LOW)
                TXData[1] = temp_v;
                TXData[0] = temp_v >> 8;
                TXData[3] = temp_c;
                TXData[2] = temp_c >> 8;
                __enable_interrupt(); // enable all interrupts --> GIE = 1 (HIGH)

                if  (outputPWMConfig.timerPeriod!=  TIMER_PERIOD * (31-(temp_v/100))) {
                        outputPWMConfig.timerPeriod = TIMER_PERIOD * (31-(temp_v/100));
                        outputPWMConfig.dutyCycle =  outputPWMConfig.timerPeriod *0.25;
                        Timer_A_outputPWM(TIMER_A0_BASE, &outputPWMConfig);
                }
            }



            // Protection code
            // Current over current limit, cut 12V to relays
            if (temp_c > currentLimit * 2)
            {
                GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);
                ProtectionFlag = true;
            }

            // Voltage is below 10V, and the system is in a protection state, can
            // Return to normal by enabling 12V
            //  exit protection mode when voltage drop to close to 0V, that means HV has been turned off.
            if (temp_v < 10 && ProtectionFlag == true)
            {

                GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);
            }
        }

        // Check to see if there is any valid command. please note, current implementation does not support repeat start on host write of command.
        if ((CMD1Length > 0) && (CMD1Length <= 26))
        {
            CMDDataPtr = CMD1Data;
            CMDLength = CMD1Length;
            CMDIndex = 1; // indicate CMD in buffer1
        }
        else if ((CMD2Length > 0) && (CMD2Length <= 26))
        {
            CMDDataPtr = CMD2Data;
            CMDLength = CMD2Length;
            CMDIndex = 2; // indicate CMD in buffer2
        }

        // Polarity
        if (CMDLength >= 1)
        {

            // Check if polarity has changed from value in flash:
            // If so, change local value and flash, and:
            // Switching direction
            // 1: Set direction, assume default if non present / no command (Do not turn on power supply or relays at this point)
            // Red is +, black is -
            // 2. Turn off 12v to relay MOSFET, never opens
            // 3. Relay, set to direction of current (forward / backward, from command)
            // 4. After done, set 12V

            // Check if polarity changed from what is stored locally / before TX
            if (polarity != CMDDataPtr[0])
            {
                // Switch values to new TX value
                polarity = CMDDataPtr[0];
                write_flash();
                // flash_polarity = RXData[0]; //Put it in flash (Function, instead of a value)
                if (polarity)
                {
                    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN1);
                }
                else
                {
                    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);
                    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
                }
            }
        }

        if (CMDLength >= 3)
        {

            // Current Limit
            // First, append the high byte to the rear of the low byte (hint: use bitwise operators)
            // Ex:   RXData[1] = 10010101, High byte
            //       RXData[2] = 00101000, Low byte
            //           Both of these bytes in the array represent one half of a 16 bit integer
            //       So, the High byte needs to be shifted over 8 bits
            //       And the low byte is added in through the OR bitwise operator
            //   Visualized:
            //       1. Shift High Byte in 16 bit integer:       xxxxxxxx10010101 << 8
            //                                                 = 1001010100000000
            //       2. Use Bitwise to add Low Byte:             1001010100000000 | 00101000
            //                                                 = 1001010100101000

            currentLimit = (CMDDataPtr[1] << 8) | CMDDataPtr[2];
                write_flash();

            // if(currentLimit != flash_currentLimit)
            // flash_currentLimit = currentLimit;
            // No other adjustment needed; polling will access the currentLimit variable
            // and it will be auto updated.

            // Voltage Offset
            // Adjust the voltage offset for different points / regions, as the voltage increases
            // Still High / Low byte, so same process as above
            // Values are used automatically in calculations, so no need to change anything
            // other than flash
        }
        if (CMDLength >= 1)
        {

            vOffset1 = (RXData[3] << 8) | RXData[4];
                write_flash();

        }
        if (CMDLength >= 7)
        {

            vOffset2 = (RXData[5] << 8) | RXData[6];
                write_flash();

        }
        if (CMDLength >= 1)
        {

            vOffset3 = (RXData[7] << 8) | RXData[8];
                write_flash();

        }

        if (CMDLength == 26)
        {
            // Check each voltage offset independently to make sure it changed
            // before writing to flash
            // if (shit)
            //...
            //...

            // byte10-26 RFID
            // TO BE IMPLEMENTED
        }

        if (CMDLength > 0)
        {
            if (CMDIndex == 1)
                CMD1Length = 0;
            else if (CMDIndex == 2)
                CMD2Length = 0;
            CMDLength = 0;
        }

        // Flash LED code
        if (ProtectionFlag);

    }
}

// Interrupt vector for I2C communication
// NOTE:
// I2C has higher priority interrupt compared to SD24
#pragma vector = USCI_B0_VECTOR
__interrupt void USCIB0_ISR(void)
{
    switch (__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
    {
    case USCI_NONE:
        break;
    case USCI_I2C_UCALIFG:
        break;
    case USCI_I2C_UCNACKIFG:
        break;
    case USCI_I2C_UCSTTIFG:
        break;
    case USCI_I2C_UCSTPIFG: // when master terminated the read or write with a Nack followed by a STOP condition.
                            // Check RXInProgress bit for Host read or Host write.
        if (RXInProgress)
        { // host write

            int i;
            if (CMD1Length == 0)
            {
                for (i = 0; i < RXDataIndex; i++)
                {
                    CMD1Data[i] = RXData[i];
                }
                CMD1Length = RXDataIndex;
            }
            else
            { // assuming CMD2 buffer is empty
                for (i = 0; i < RXDataIndex; i++)
                {
                    CMD2Data[i] = RXData[i];
                }
                CMD2Length = RXDataIndex;
            }
            RXDataIndex = 0;
        }
        else
        { // host read
            TXDataIndex = 0;
            // EUSCI_B_I2C_disableInterrupt(EUSCI_B0_BASE,EUSCI_B_I2C_TRANSMIT_INTERRUPT0);
        }
        break;
    case USCI_I2C_UCRXIFG3:
        break;
    case USCI_I2C_UCTXIFG3:
        break;
    case USCI_I2C_UCRXIFG2:
        break;
    case USCI_I2C_UCTXIFG2:
        break;
    case USCI_I2C_UCRXIFG1:
        break;
    case USCI_I2C_UCTXIFG1:
        break;
    case USCI_I2C_UCRXIFG0:

        // Puts all 26 received bits from master, and puts it into RXData, which increments it each time the data is full
        RXData[RXDataIndex++] = EUSCI_B_I2C_slaveGetData(EUSCI_B0_BASE);
        RXInProgress = 1;
        break;

    case USCI_I2C_UCTXIFG0: // for Master read, UCTR(TR mode, based on R/W# and UCTXIFGO will be set. after transmit the data, UXTXIFGO will set again if master read more data

        EUSCI_B_I2C_slavePutData(EUSCI_B0_BASE, TXData[TXDataIndex++]);
        RXInProgress = 0;

        break;

    case USCI_I2C_UCBCNTIFG:
        break;
    case USCI_I2C_UCCLTOIFG:
        break;
    case USCI_I2C_UCBIT9IFG:
        break;
    default:
        break;
    }
}

#pragma vector = SD24_VECTOR
__interrupt void SD24_ISR(void)
{
    switch (__even_in_range(SD24IV, SD24IV_SD24MEM3))
    {
    case SD24IV_NONE:
        break;
    case SD24IV_SD24OVIFG:
        break;
    case SD24IV_SD24MEM0:

        // Save CH0 results (clears IFG)
        Ch0results = SD24_getResults(SD24_BASE, SD24_CONVERTER_0);

        break;
    case SD24IV_SD24MEM1:

        // Save CH1 results (clears IFG)
        Ch1results = SD24_getResults(SD24_BASE, SD24_CONVERTER_1);

        hv_update = 1; // a flag to indicate new ADC value is available

        break;
    case SD24IV_SD24MEM2:
        break;
    case SD24IV_SD24MEM3:
        break;
    default:
        break;
    }
}
