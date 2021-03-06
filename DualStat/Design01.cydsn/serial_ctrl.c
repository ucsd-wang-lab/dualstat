/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "main.h"
#include "serial_ctrl.h"

/* Functions */
uint8_t isCharReady(void);
uint8_t getRxStr(void);
void processCommandMsg(void);
CY_ISR_PROTO(MfgRxInt);          // process Rx interrupt
char HexToAscii(uint8_t value, uint8_t nibble);

/* Global variables */
uint8_t	RxBuffer[RxBufferSize];     // Rx circular buffer to hold all incoming command
uint8_t  *RxReadIndex	 = RxBuffer;    // pointer to position in RxBuffer to write incoming Rx bytes
uint8_t  *RxWriteIndex = RxBuffer;    // pointer to position in RxBuffer to read and process bytes

char   *RxStrIndex = RB.RxStr;      // pointer to command string buffer (processed messages)
                                    // each Rx command consists of: <byte command><string value><CR>

//buffer to hold application settings
typedef struct TParamBuffer{
    uint8 R;
} ParamBuffer; //settings

volatile ParamBuffer PB;  

void startSerialCtrlInt(void)
{
    Rx_Int_StartEx(MfgRxInt);//start Rx interrupt  
}

void serialCtrl_ProcessEvents(void)
{
    if(isCharReady()) //Rx buffer not empty
    {
        if (getRxStr())   //extract first command message (non-blocking)
        {
            processCommandMsg(); 
        }
    }   
}

CY_ISR(MfgRxInt) //interrupt on Rx byte received
{   
    DBG_UART_ClearRxInterruptSource(DBG_UART_INTR_RX_NOT_EMPTY ); //clear interrupt
    
    //move all available characters from Rx queue to RxBuffer
    char byte;
    while((byte = DBG_UART_UartGetChar()) !=0 )
    {
        *RxWriteIndex++ = byte; 
		if (RxWriteIndex >= RxBuffer + RxBufferSize) RxWriteIndex = RxBuffer;      
	}   
}

//===========================================================================
uint8_t isCharReady(void) 
{
	return !(RxWriteIndex == RxReadIndex);
}

//===========================================================================

uint8_t getRxStr(void)
{
    uint8_t RxStr_flag = 0;
    static uint8_t Ch;//static?
   
	Ch = *RxReadIndex++;       //read next char in buffer
    if (RxReadIndex >= RxBuffer + RxBufferSize) RxReadIndex = RxBuffer;
            
    //if (Ch == EOM_char)
    if ( (Ch == EOM_CR) || (Ch == EOM_LF) ) //any standard terminator
    {
        *RxStrIndex = 0;        //terminate string excluding EOM_char
        RxStrIndex = RB.RxStr;  //reset pointer
        if (strlen(RB.RxStr) > 0)//non-empty message received
        {
            RxStr_flag  = 1;    //set flag to process message
        }   
    }
    else                        //string body char received
    {
        *RxStrIndex++ = Ch;     //build command message   
        //todo: problem if first char is empty space
    }   

    return RxStr_flag;        
}

//===========================================================================
//===========================================================================
// Process UART Receive Buffer content: RB.RxStr = RB.cms + RB.valstr
// 
//===========================================================================

void processCommandMsg(void)
{    
    // check received message for any valid command and execute it if necessary or report old value
    // if command not recognized, then report error (!)
    if  (RB.cmd == cmd_V)//command 'V' received..
    {
        DBG_PRINTF("V\n\r");//echo command  
        DBG_PRINTF("TEST> %s: FW Version %d.%d.%d Built: %s, %s\n\r", PROJECT_NAME, FV_MAJOR, FV_MINOR, FV_POINT, COMPILE_TIME, FIRMWARE_DATE);
    }
//    else if (RB.cmd == cmd_M)//command 'M' received..set MAC address
//    {
//        //check mac address length
//        uint8 macLength = sizeof(RB.valstr)/sizeof(char);
//        DBG_PRINTF("M");
//        for(uint8 i=0; i < macLength; i++)
//        {
//            if (RB.valstr[i] == 0)
//            {
//                macLength--;
//                //RB.valstr[i] = '*';
//            }
//            DBG_PRINTF("%c", RB.valstr[i]);
//        }
//        DBG_PRINTF("\r\n");
//        if (macLength == 12)
//        {
//            if (ParseMAC(RB.valstr))
//            {
//                SaveMAC();
//                DBG_PRINTF("TEST>  Saved MAC Address: ");
//                DisplayMAC();   //read MAC from SFlash and display
//                DBG_PRINTF("\r\n");
//                UpdateBLE_Advert();   //create UUID from MAC
//            }
//            else
//            {
//                DBG_PRINTF("TEST>  Invalid MAC characters.\r\n");
//            }
//        }
//        else 
//        {
//            DBG_PRINTF("TEST>  Invalid MAC address length.\r\n");
//        }
//    }
    else if (RB.cmd == cmd_R)//command 'R' received..
    {
        PB.R = RB.valstr[0];
        if (PB.R == ASCII_0)
        {
            DBG_PRINTF("R0\n\rTEST>  Red LED OFF\r\n");//echo command   
            LED_2_Write(LED_OFF);
        }
        else if (PB.R == ASCII_1)
        {
            DBG_PRINTF("R1\n\rTEST>  Red LED ON\r\n");//echo command
            LED_2_Write(LED_ON);
        }
        else 
        {
            DBG_PRINTF("TEST> Red LED command value '%c' not recognized. Valid values are 0 and 1.\r\n", RB.valstr[0]);//echo command and value       
        }        
    }    
    else //command unrecognized - echo unrecognized command
    {
        DBG_PRINTF("%c\r\n", RB.cmd);
        DBG_PRINTF("TEST>  Command %c is not recognized.\r\n", RB.cmd);//echo command and value  
    }
}

/*******************************************************************************
* Function Name: HexToAscii
********************************************************************************
* Summary:
*        Converts either the higher or lower nibble of a hex byte to its
* corresponding ASCII.
*
* Parameters:
*  value - hex value to be converted to ASCII
*  nibble - 0 = lower nibble, 1 = higher nibble
*
* Return:
*  char - hex value for the value/nibble specified in the parameters
*
*******************************************************************************/
char HexToAscii(uint8_t value, uint8_t nibble)
{
    if(nibble == 1)
    {
        /* bit-shift the result to the right by four bits */
        value = value & 0xF0;
        value = value >> 4;
        
        if (value >9)
        {
            value = value - 10 + 'A'; /* convert to ASCII character */
        }
        else
        {
            value = value + '0'; /* convert to ASCII number */
        }
    }
    else if (nibble == 0)
    {
        /* extract the lower nibble */
        value = value & 0x0F;
        
        if (value >9)
        {
            value = value - 10 + 'A'; /* convert to ASCII character */
        }
        else
        {
            value = value + '0'; /* convert to ASCII number */
        }
    }
    else
    {
        value = ' ';  /* return space for invalid inputs */
    }
    return value;
}

unsigned char atoh (unsigned char data)
{ 
    if (data > '9') 
    { 
        data += 9;
    }
    return (data &= 0x0F);
}
/* [] END OF FILE */
