/*
 * FreeRTOS Kernel V10.2.1
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */
#include "stdio.h"
#include "string.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "Serial/serial.h"


//#include "driverlib.h"
#include "i2c.h"
#include "bme280.h"

/* Used for maintaining a 32-bit run time stats counter from a 16-bit timer. */
volatile uint32_t ulRunTimeCounterOverflows = 0;

/* The heap is allocated here so the "persistent" qualifier can be used.  This
requires configAPPLICATION_ALLOCATED_HEAP to be set to 1 in FreeRTOSConfig.h.
See http://www.freertos.org/a00111.html for more information. */

//#ifdef __ICC430__
//    __persistent                    /* IAR version. */
//#else
//    #pragma PERSISTENT( ucHeap )    /* CCS version. */
//#endif

uint8_t ucHeap[ configTOTAL_HEAP_SIZE ] = { 0 };

/*-----------------------------------------------------------*/

uint8_t rawData[8];
calibrPar bme280Params;

typedef struct weatherData
{
    int32_t  temp;
    uint32_t pres;
    uint32_t hum;
} weatherData;

#define QUEUE_LENGTH 5
#define QUEUE_ITEM_SIZE sizeof( weatherData )

static void prvSetupHardware( void )
{
    taskDISABLE_INTERRUPTS();

    WDTCTL = WDTPW + WDTHOLD;            // Stop watchdog timer

        //P1.0,LED1 for indicating sensors are gathering data
    P1DIR |= BIT0;
    P4DIR |= BIT7;//P4.7,LED2 for sending data to PC

    init_serial();
    initI2c(SLAVE_ADDRESS);
    configBme280();
    getBme280CalibrationParameters(SLAVE_ADDRESS, &bme280Params);
}

void vGetBme280Values(void * pvParameters)
{
    QueueHandle_t xQueue = ( QueueHandle_t ) pvParameters;
    weatherData data;

    int32_t temp;
    uint32_t press, hum;

    const TickType_t xDelay = pdMS_TO_TICKS(10000);
    int i;
    for(;;)
    {
        for(i=0; i<8;i++) rawData[i]=0xff;
        getI2cValues(SLAVE_ADDRESS, ADDR_DATA_Low, 8, rawData );

        press = (((uint32_t) rawData[0]) << 12) | (((uint32_t) rawData[1])<<4) | (((uint32_t) rawData[2])>>4);
        temp  = (((uint32_t) rawData[3]) << 12) | (((uint32_t) rawData[4])<<4) | (((uint32_t) rawData[5])>>4);
        hum   = (((uint32_t) rawData[6]) <<  8) | ( (uint32_t) rawData[7]);

        temp  = BME280_compensate_T_int32(temp, &bme280Params); // 0x00081b70
        hum   = BME280_compensate_H_int32 (hum, &bme280Params) >> 10;
        press = BME280_compensate_P_int32(press,&bme280Params);

        data.temp = temp;
        data.hum  = hum;
        data.pres = press;
        xQueueSend(xQueue, &data, pdMS_TO_TICKS(500));
        P4OUT ^= BIT7;

        vTaskDelay( xDelay );
    }

}

void vSendDataWithSerial(void * pvParameters)
{
    unsigned int i;

    unsigned int hPres;
    unsigned int deg;
    unsigned int cent;

    char s[60];

    int32_t tem2;
    uint32_t hum2, pres2;

    while(1)
    {
        QueueHandle_t xQueue = ( QueueHandle_t ) pvParameters;
        weatherData data;

        if(xQueueReceive(xQueue,&data,pdMS_TO_TICKS(10000))!= pdPASS)
        {
            /* Nothing was received from the queue – even after blocking to wait for data to arrive. */
            data.temp  = 1;
            data.hum  = 1;
            data.pres = 1;
        }
        P1OUT ^= BIT0;

        tem2  = data.temp;
        hum2  = data.hum;
        pres2 = data.pres;

        deg = tem2 / 100;
        cent = tem2 % 1000;
        hPres = pres2 / 100;

        sprintf(s, "START:%d.%d,%d,%dEND;",
                    deg,
                    cent,
                    (unsigned int) hum2,
                    hPres);

        for(i = 0; i < strlen(s); i++)
          bsend_ch(s[i]);
    }

}

int main( void )
{
     /* Perform any hardware setup necessary. */
     prvSetupHardware();

     QueueHandle_t xQueue = xQueueCreate( QUEUE_LENGTH, QUEUE_ITEM_SIZE );

     P1OUT &= ~BIT0; P4OUT &= ~BIT7;

     if( xQueue != NULL )
     {
         xTaskCreate( vGetBme280Values, "vGetBme280Values",
                          configMINIMAL_STACK_SIZE, (void *) xQueue, 1, NULL );
         xTaskCreate( vSendDataWithSerial , "SendsDataWithSerial",
                          configMINIMAL_STACK_SIZE, (void *) xQueue, 1, NULL);
     }

     vTaskStartScheduler();
     /* Execution will only reach here if there was
            insufficient heap to start the scheduler. */
     for( ;; );
     return 0;
}

/*-----------------------------------------------------------*/

/* The MSP430X port uses this callback function to configure its
 * tick interrupt.This allows the application to choose the tick
 * interrupt source. configTICK_VECTOR must also be set in
 * FreeRTOSConfig.h to the correct interrupt vector for the chosen
 * tick interrupt source.  This implementation of
 * vApplicationSetupTimerInterrupt() generates the tick from
 * timer A0, so in this case configTICK_VECTOR is set to
 * TIMER0_A0_VECTOR. */

void vApplicationSetupTimerInterrupt( void )
{
const unsigned short usACLK_Frequency_Hz = 32768;

    /* Ensure the timer is stopped. */
    TA0CTL = 0;


    /* Run the timer from the ACLK. */
    TA0CTL = TASSEL_1;

    /* Clear everything to start with. */
    TA0CTL |= TACLR;

    /* Set the compare match value according to
           the tick rate we want. */
    TA0CCR0 = usACLK_Frequency_Hz / configTICK_RATE_HZ;

    /* Enable the interrupts. */
    TA0CCTL0 = CCIE;

    /* Start up clean. */
    TA0CTL |= TACLR;

    /* Up mode. */
    TA0CTL |= MC_1;
}
/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/

void vConfigureTimerForRunTimeStats( void )
{
    /* Configure a timer that is used as the time base for run time stats.  See
    http://www.freertos.org/rtos-run-time-stats.html */

    /* Ensure the timer is stopped. */
    TA1CTL = 0;

    /* Start up clean. */
    TA1CTL |= TACLR;

    /* Run the timer from the ACLK/8, continuous mode, interrupt enable. */
    TA1CTL = TASSEL_1 | ID__8 | MC__CONTINUOUS | TAIE;
}
/*-----------------------------------------------------------*/

#pragma vector=TIMER1_A1_VECTOR
__interrupt void v4RunTimeStatsTimerOverflow( void )
{
    TA1CTL &= ~TAIFG;

    /* 16-bit overflow, so add 17th bit. */
    ulRunTimeCounterOverflows += 0x10000;
    __bic_SR_register_on_exit( SCG1 + SCG0 + OSCOFF + CPUOFF );
}

