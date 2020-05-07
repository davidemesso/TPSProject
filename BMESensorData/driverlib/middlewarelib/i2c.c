//
// 02/11/2019, V 1.1
// Paolo Santinelli
//
#include "driverlib.h"
#include "usci_b_i2c.h"
#include "i2c.h"
volatile uint8_t *receiveBuffPointer;
volatile uint16_t receiveCnt;
volatile uint8_t *transmitBuffPointer;
volatile uint16_t transmitCnt = 0;
volatile uint8_t i2cError;
volatile uint8_t dummy;

//******************************************************************************
//
// Init I2C routine.
//
//******************************************************************************
//------------------------------------------------------------------------------
void initI2c(uint8_t slaveAddress)
{
    //Assign I2C pins to USCI_B0


    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN0 + GPIO_PIN1);

    //Initialize Master
    USCI_B_I2C_initMasterParam param = {0};
    param.selectClockSource = USCI_B_I2C_CLOCKSOURCE_SMCLK;
    param.i2cClk = UCS_getSMCLK();
    param.dataRate = USCI_B_I2C_SET_DATA_RATE_400KBPS;
    USCI_B_I2C_initMaster(USCI_B0_BASE, &param);

    //Specify slave address
    USCI_B_I2C_setSlaveAddress(USCI_B0_BASE, slaveAddress);

    //Set in transmit mode
    USCI_B_I2C_setMode(USCI_B0_BASE, USCI_B_I2C_TRANSMIT_MODE);

    //Enable I2C Module to start operations
    USCI_B_I2C_enable(USCI_B0_BASE);

    __enable_interrupt();
}
//------------------------------------------------------------------------------
int i2c_transfer(const i2c_device *dev, i2c_transaction *data)
{
    i2cError = 0;

    /* Set the slave device address */
    USCI_B_I2C_setSlaveAddress(dev->baseAddress, dev->slaveAddress);

    /* Transmit data is there is any */
    if (data->tx_len > 0) {
        //Initialize transmitter data pointer and counter
        transmitBuffPointer = (uint8_t*)data->tx_buf;
        transmitCnt = data->tx_len;

        //Enable NAK interrupt
        USCI_B_I2C_enableInterrupt(dev->baseAddress, USCI_B_I2C_NAK_INTERRUPT);
        //Set transmit mode
        USCI_B_I2C_setMode(dev->baseAddress, USCI_B_I2C_TRANSMIT_MODE);

        //Clear transmit interrupt flag UCTXIFG
        USCI_B_I2C_clearInterrupt(dev->baseAddress,USCI_B_I2C_TRANSMIT_INTERRUPT);
        //Enable transmit interrupt
        USCI_B_I2C_enableInterrupt(dev->baseAddress, USCI_B_I2C_TRANSMIT_INTERRUPT);

        /* Send the start condition to start data transmission */
        USCI_B_I2C_masterSendStart (dev->baseAddress);
        //Wait until transmission completes or aborts because of NACK
        while (USCI_B_I2C_isBusBusy(dev->baseAddress));
        if (i2cError) return 1; //Master transmit NACK error
    }

    /* Receive data is there is any */
    if (data->rx_len > 0) {
        //Initialize receiver data pointer and counter
        receiveBuffPointer = data->rx_buf;
        receiveCnt = data->rx_len;

        // Enable receive interrupt
        USCI_B_I2C_enableInterrupt(dev->baseAddress, USCI_B_I2C_RECEIVE_INTERRUPT);

        // Start to receive data from slave device - Send start -
        USCI_B_I2C_masterReceiveMultiByteStart(dev->baseAddress);
        //Wait until reception is completed or aborted because of NACK
        while (USCI_B_I2C_isBusBusy(dev->baseAddress));
        if (i2cError) return 2; //Master receive NACK error
    }

    return 0; // Transaction success
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_B0_VECTOR)))
#endif
void USCI_B0_ISR (void)
{
    switch (__even_in_range(UCB0IV,12)){
          case  0: break;                           // Vector  0: No interrupts
          case  2: break;                           // Vector  2: ALIFG
          case  USCI_I2C_UCNACKIFG:                 // Vector  4: NACKIFG
          {
              //Send STOP
              HWREG8(USCI_B0_BASE + OFS_UCBxCTL1) |= UCTXSTP;
              // Disable transmit interrupt
              HWREG8(USCI_B0_BASE + OFS_UCBxIE) &= ~ UCTXIE;
              // Set i2c communication error flag
              i2cError = 1;
              break;
          }

          case  6: break;                           // Vector  6: STTIFG
          case  8: break;                           // Vector  8: STPIFG

          case USCI_I2C_UCRXIFG:                    //Vector 10: Receive buffer full - RXIF
          {
              if (receiveCnt){
                  *receiveBuffPointer++ = HWREG8(USCI_B0_BASE + OFS_UCBxRXBUF);
                  if (receiveCnt == 1) {
                      //Send STOP
                      HWREG8(USCI_B0_BASE + OFS_UCBxCTL1) |= UCTXSTP;
                  }
                  //Decrement RX byte counter
                  receiveCnt--;
              }
              else {
                  //Dummy read
                  dummy = HWREG8(USCI_B0_BASE + OFS_UCBxRXBUF);
              }
              break;
          }

          //Vector 12: Transmit buffer empty - TXIF
          case USCI_I2C_UCTXIFG:
              {

                  if(transmitCnt)
                      HWREG8(USCI_B0_BASE + OFS_UCBxTXBUF) = *transmitBuffPointer++;

                  else{
                      // Send STOP
                      HWREG8(USCI_B0_BASE + OFS_UCBxCTL1) |= UCTXSTP;
                      // Clear USCI_B0 TX interrupt flag
                      //HWREG8(USCI_B0_BASE + OFS_UCBxIFG) &= ~UCTXIFG;    //Disable the interrupt masked bit
                      HWREG8(USCI_B0_BASE + OFS_UCBxIE) &= ~UCTXIE;

                  }
                  transmitCnt--;
                  break;
              }
          default:  break;
      }
}
//------------------------------------------------------------------------------
