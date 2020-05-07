#include "driverlib.h"
#include "serial.h"
#include "circular_buffer.h"

CircularBuffer rx, tx;

void init_serial(void)
{
  int buffers_dim = 16;
  P4SEL 	|= BIT4+BIT5;                   // P4.4,5 = USCI_A1 TXD/RXD
  UCA1CTL1	|= UCSWRST;                     // **Put state machine in reset**
  UCA1CTL1	|= UCSSEL_2;                    // SMCLK
  UCA1BR0 	= 9;                            // 1MHz 115200 (see User's Guide)
  UCA1BR1 	= 0;                            // 1MHz 115200
  UCA1MCTL 	|= UCBRS_1 + UCBRF_0;           // Modulation UCBRSx=1, UCBRFx=0
  UCA1CTL1 	&= ~UCSWRST;                    // **Initialize USCI state machine**
  UCA1IE 	|= UCRXIE;                      // Enable USCI_A0 RX interrupt
  cbInit(&rx, buffers_dim);
  cbInit(&tx, buffers_dim);

}
unsigned char tx_char(unsigned char ch)
{
  if(UCA1IFG&UCTXIFG){
    UCA1TXBUF = ch;
    return 1;
  }
  else return 0;
}

unsigned char rx_char(unsigned char *ch)
{
  if(UCA1IFG&UCRXIFG){
    *ch = UCA1RXBUF;
    return 1;
  }
  else return 0;
}

unsigned char breceive_ch(unsigned char * ch)
{
  return !cbRead(&rx, ch);
}

int bsend_ch(unsigned char ch)
{
  int ret;
  UCA1IE &=~ UCTXIE;
  ret = !cbWrite(&tx, ch);
  UCA1IE |= UCTXIE;
  return ret;
}

#pragma vector = USCI_A1_VECTOR
__interrupt void usci_a1_irq (void)
{
  unsigned char ch;
  
  if(UCA1IFG&UCRXIFG){	                        // Data Recieved
    	cbWrite(&rx, UCA1RXBUF);
    }

  if((UCA1IFG&UCTXIFG) && (UCA1IE&UCTXIE) ){	// Transmitter buffer empty
    	cbRead(&tx, &ch);
    	if(cbIsEmpty(&tx)) UCA1IE &=~ UCTXIE;
    	  UCA1TXBUF = ch;
    }
}

