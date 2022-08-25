#include <msp430.h> 
#include <stdio.h>  // sprintf


#define VCC 3.326
#define VSS 0
#define RXD BIT1
#define TXD BIT2


char num[5];  // string to store stringified adc output
double voltage;


void send_char(const char c);
void send_str(char* c);


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	// set clock for 1MHz
	DCOCTL = 0;
	BCSCTL1 = CALBC1_1MHZ;  // use calibration data
	DCOCTL = CALDCO_1MHZ;   // use calibration data

	// configure UART for 9600 Baud Rate
	UCA0CTL1 = UCSWRST;     // put into reset state
	// configure pins to use for UART
	P1DIR = TXD;           // set as output pin
	P1SEL |= RXD | TXD;   // set pins for uart mode
	P1SEL2 |= RXD | TXD;  // set pins for uart mode
	UCA0CTL1 |= UCSSEL_2;   // use SMCLK as source - 1MHz
	UCA0BR0 = 104;          // UCBRx = 104
	UCA0BR1 = 0;            // nothing about this in user guide
	UCA0MCTL = UCBRS_1;     // UCBRSx = 1 ; UCBRFx = 0
	UCA0CTL1 &= ~UCSWRST;   // take out of reset state

	// configure ADC
	ADC10CTL0 |= ADC10ON | ADC10IE;     // turn on ADC10 and enable interrupts
	ADC10CTL1 |= ADC10DIV_7 | INCH_3;   // divide clock by 8 and select pin 1.3 as input
	ADC10AE0 = BIT3;                    // enable pin 1.3 as input
	ADC10CTL0 |= ENC | ADC10SC;         // start conversion

	__enable_interrupt();

	while(1) {
	    LPM4;
	}
}

void send_char(const char c) {
    while (!(IFG2 & UCA0TXIFG));  // wait until the transmit buffer is ready
    UCA0TXBUF = c;                // put char into the transmit buffer
}

void send_str(char* c) {
    while (*c != 0) {
        send_char(*c);
        ++c;            // next character
    }
}

#pragma vector = ADC10_VECTOR
__interrupt void ADC10_interrupt(void) {
    send_str(" Received: ");
    voltage = ADC10MEM * (VCC - VSS) / 1023 + VSS;
    sprintf(num, "%f", voltage);    // get string version of adc output
    send_str(num);
    ADC10CTL0 |= ENC | ADC10SC;      // start conversion
}
