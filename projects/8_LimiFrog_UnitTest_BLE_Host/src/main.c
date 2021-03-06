/*******************************************************************************
 * main.c
 * 
 * (c)2015 LimiFrog / CYMEYA
 * This program is licensed under the terms of the MIT License.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 * Please refer to the License File LICENSE.txt located at the root of this
 * project for full licensing conditions, 
 * or visit https://opensource.org/licenses/MIT.
 ******************************************************************************/


///  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!     
//   THIS IS A QUICK IMPLEMENTATION     
//   THAT WOULD NEED TO BE REWORKED.
//   But at least it shows BLE operation in
//   host mode.
///  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!     



#include "LBF_Global.h"


volatile bool Rx_uart3_done  ;
  // global variable shared with Interrupt Service Routine in stm32_it.c


/*******************************************************************************
* Function Name  : main.
* Description    : Main routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
int main(void)
{

bool  Success = TRUE;

/* ================================================================ */
/* Board Initializations and Configurations except OLED             */
/* (clocks, I/Os, on-chip peripherals, on-board ICs)                */
/* ================================================================ */

   LBF_Board_Fixed_Inits();
   LBF_Board_Selective_Inits(); 
             // actions driven by User_Configuration.h

   LBF_Led_ON();

/* ================================================================ */
/* Optional initialization of Middleware libraries :                */
/* USBD drivers, FatFS File System, STemWin GUI                     */
/* ================================================================ */

    // UNCOMMENT AS NEEDED:
    // (refer to INFO.txt for details on actions performed)

    /* ... To use La BlueFrog as USB Mass Storage (Full Speed)      */
    //Delay_ms(1000);
    Success &= LBF_LaunchUSB_MassStorage();
 
    /* ... To initialize FatFS                                      */
    /*     and mount the Data Flash as FAT File System              */
    // Success &= LBF_FatFS_Init();

    /* ... To initialize the STemWin Graphical Library              */ 
    /*     Caution: reserves some RAM - keep some for stack/heap    */
    Success &= LBF_emWin_Init();

    // ERROR HANDLER
    /* Replace by your own as wished */
    LBF_Led_StopNBlinkOnFalse (Success);
    	// stops here if one of above inits has failed

    LBF_Led_OFF();



/* ===================================================== */
/* Application Code Below */
/* ===================================================== */

// --------------------------------------------------------
// BTLE must be enabled by #define in User_Configuration.h
// Name of the file containing code to download by STM32 
// into BTLE module also defined there -- download occurs
// during LBF_Board_Selective_Inits() phase.
// This executable must be present on FAT File System on Flash 
// (typically stored there from PC through USB).
// In this demo, code is supposed to be Serial Port Service
// snippet provided by Dialog -- this configures the BTLE
// to behave as UART cable replacement. Data sent to
// BTLE module over UART1 at 115200 baud is transitted over BTLE
// This data can be received by DSPS application running in iOS
// or Android, provided by Dialog, available on Appple/Android
// app stores
// --------------------------------------------------------


/* ==  User Declarations =============================== */

char RxString[256];
uint8_t RxByte ;

uint32_t i;




/* ==  Body     ======================================== */


    // Initialize IT from UART3 
    HAL_NVIC_SetPriority(USART3_IRQn, __USART3_IRQn_PRIO, 0 );
    HAL_NVIC_EnableIRQ(USART3_IRQn);



    LBF_OLED_Switch_ON();
    // Provide power (13V) to OLED panel, enable display

    LBF_OLED_PrintString("\n Ready\n");


    while(1) 
    {

        // Clear UART OverRun Error Flag in case stuff (such as XON/XOFF)
        // has arrived in UART Rx buffer while busy displaying, 
	// causing OVR (overrun) error - i.e.previous Rx data overwritten
        // (See also Caution note at the end)  
        // Else UART Rx is automatically disabled when OverRun Error Flag is set !

        __HAL_UART_CLEAR_FLAG( &huart3, UART_CLEAR_OREF );  
	    // UART3 is UART used by BLE
	    // huart3 global variable


        // Receive valid byte from BTLE module through UART3
        i = 0;
        while (i<255)  // 255 char max in string to receive
        {
            LBF_Led_ON();

            Rx_uart3_done = false;    
            HAL_UART_Receive_IT(&huart3, &RxByte, 0x1 );

            while( !Rx_uart3_done ) ; 
	      // wait until new byte received (flag set in ISR)

            RxString[i] = RxByte;

            // Current BLE Host firmware regularly sends XON/XOFF
            // ...ignore these bytes
            if ( (RxString[i] != XON) && (RxString[i]!=XOFF) )
            {
                if (RxString[i++] == '\0')  
                   break;  
            }

        };


       // Display received message
       LBF_OLED_PrintString(RxString);

       // CAUTION:
       // Current BLE Host firmware regularly sends XON/XOFF 
       // over UART when no (payload) data received from BLE radio
       // As display takes time there is a risk to have
       // overrun on UART RX 



    }  // end of while(1)

}


/***************************************************************END OF FILE****/// 
