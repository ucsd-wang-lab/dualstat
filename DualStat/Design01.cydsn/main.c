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
#include <project.h>
#include "main.h"
#include "serial_ctrl.h"

/*Function declarations */
void StackEventHandler( uint32 eventCode, void *eventParam );
uint32_t getTick(void);
void resetTick(void);
void tickStart(void);
void tickStop(void);
/* Global variables*/
volatile uint32_t tickTimer = 0;

/*******************************************************************************
* Function Name: Tick_Interrupt
********************************************************************************
*
* Summary:
*  Handles the Interrupt Service Routine for the WDT tick mechanism.
*
*******************************************************************************/
CY_ISR(Tick_Interrupt) // MAIN_TICK_FREQ
{
    /* Indicate that timer is raised to the main loop */
    tickTimer++;
}

uint32_t getTick(void)
{
//    #if (DEBUG_UART_ENABLED == ENABLED)
//    if(tickTimer > 1)
//    {
//        //DBG_PRINTF("**********\tMissed tick! Tick Val: %d\t**********\r\n", tickTimer);
//    }
//    #endif
    return tickTimer;
}

void resetTick(void)
{
    tickTimer = 0;
    return;
}

void tickStart(void)
{
    /* Unlock the WDT registers for modification */
    CySysWdtUnlock(); 
    // Register Tick_Interrupt() by the WDT COUNTER0 to generate an interrupt every tick
    CySysWdtSetInterruptCallback(CY_SYS_WDT_COUNTER0, Tick_Interrupt);
    // Enable the Counter0 ISR Handler
    CySysWdtEnableCounterIsr(CY_SYS_WDT_COUNTER0);
    /* Lock out configuration changes to the Watchdog timer registers */
    CySysWdtLock();   
}

void tickStop(void)
{
    /* Unlock the WDT registers for modification */
    CySysWdtUnlock(); 
    /* Disable the specified WDT counter */
    CySysWdtDisable(CY_SYS_WDT_COUNTER0);
    /* Locks out configuration changes to the Watchdog timer registers */
    CySysWdtLock();    
}

#define TEMP_TOGGLE_CNT     4096
uint16_t temp_cnt = 0;

int main (void)
{
    CyGlobalIntEnable;   /* Enable global interrupts */
    DBG_UART_Start();
    DBG_PRINTF("\r\n\r\nTEST> %s: FW Version %d.%d.%d Built: %s, %s\n\r", PROJECT_NAME, FV_MAJOR, FV_MINOR, FV_POINT, COMPILE_TIME, FIRMWARE_DATE);
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    tickStart();
    
    #if (SERIAL_CTRL == DISABLED)
        CyBle_Start( StackEventHandler );
    #else
        startSerialCtrlInt();
    #endif
    
    for(;;)
    {
        #if (SERIAL_CTRL == ENABLED)
            serialCtrl_ProcessEvents(); //handle serial events
        #else
            CyBle_ProcessEvents();  //handle BLE events
        #endif
        
        if(getTick())//MAIN_TICK_FREQ
        { 
            if (++temp_cnt == TEMP_TOGGLE_CNT)
            {
                temp_cnt = 0;
                (LED_1_Read() == GPIO_HIGH) ? LED_1_Write(GPIO_LOW) : LED_1_Write(GPIO_HIGH);
            }
            resetTick();
        }
    }
}

void StackEventHandler( uint32 eventCode, void *eventParam )
{
    switch( eventCode )
    {
        /* Generic events */

        case CYBLE_EVT_HOST_INVALID:
        break;

        case CYBLE_EVT_STACK_ON:
            /* CyBle_GappStartAdvertisement( CYBLE_ADVERTISING_FAST ); */
        break;

        case CYBLE_EVT_TIMEOUT:
        break;

        case CYBLE_EVT_HARDWARE_ERROR:
        break;

        case CYBLE_EVT_HCI_STATUS:
        break;

        case CYBLE_EVT_STACK_BUSY_STATUS:
        break;

        case CYBLE_EVT_PENDING_FLASH_WRITE:
        break;


        /* GAP events */

        case CYBLE_EVT_GAP_AUTH_REQ:
        break;

        case CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
        break;

        case CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
        break;

        case CYBLE_EVT_GAP_AUTH_COMPLETE:
        break;

        case CYBLE_EVT_GAP_AUTH_FAILED:
        break;

        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
        break;

        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            /* CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST); */
        break;

        case CYBLE_EVT_GAP_ENCRYPT_CHANGE:
        break;

        case CYBLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
        break;

        case CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
        break;


        /* GAP Peripheral events */

        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
        break;


        /* GAP Central events */

        case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
        break;

        case CYBLE_EVT_GAPC_SCAN_START_STOP:
        break;


        /* GATT events */

        case CYBLE_EVT_GATT_CONNECT_IND:
        break;

        case CYBLE_EVT_GATT_DISCONNECT_IND:
        break;


        /* GATT Client events (CYBLE_EVENT_T) */

        case CYBLE_EVT_GATTC_ERROR_RSP:
        break;

        case CYBLE_EVT_GATTC_XCHNG_MTU_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_BY_GROUP_TYPE_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_BY_TYPE_RSP:
        break;

        case CYBLE_EVT_GATTC_FIND_INFO_RSP:
        break;

        case CYBLE_EVT_GATTC_FIND_BY_TYPE_VALUE_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_BLOB_RSP:
        break;

        case CYBLE_EVT_GATTC_READ_MULTI_RSP:
        break;

        case CYBLE_EVT_GATTC_WRITE_RSP:
        break;

        case CYBLE_EVT_GATTC_EXEC_WRITE_RSP:
        break;

        case CYBLE_EVT_GATTC_HANDLE_VALUE_NTF:
        break;

        case CYBLE_EVT_GATTC_HANDLE_VALUE_IND:
        break;


        /* GATT Client events (CYBLE_EVT_T) */

        case CYBLE_EVT_GATTC_INDICATION:
        break;

        case CYBLE_EVT_GATTC_SRVC_DISCOVERY_FAILED:
        break;

        case CYBLE_EVT_GATTC_INCL_DISCOVERY_FAILED:
        break;

        case CYBLE_EVT_GATTC_CHAR_DISCOVERY_FAILED:
        break;

        case CYBLE_EVT_GATTC_DESCR_DISCOVERY_FAILED:
        break;

        case CYBLE_EVT_GATTC_SRVC_DUPLICATION:
        break;

        case CYBLE_EVT_GATTC_CHAR_DUPLICATION:
        break;

        case CYBLE_EVT_GATTC_DESCR_DUPLICATION:
        break;

        case CYBLE_EVT_GATTC_SRVC_DISCOVERY_COMPLETE:
        break;

        case CYBLE_EVT_GATTC_INCL_DISCOVERY_COMPLETE:
        break;

        case CYBLE_EVT_GATTC_CHAR_DISCOVERY_COMPLETE:
        break;

        case CYBLE_EVT_GATTC_DISCOVERY_COMPLETE:
        break;


        /* GATT Server events (CYBLE_EVENT_T) */

        case CYBLE_EVT_GATTS_XCNHG_MTU_REQ:
        break;

        case CYBLE_EVT_GATTS_WRITE_REQ:
        break;

        case CYBLE_EVT_GATTS_WRITE_CMD_REQ:
        break;

        case CYBLE_EVT_GATTS_PREP_WRITE_REQ:
        break;

        case CYBLE_EVT_GATTS_EXEC_WRITE_REQ:
        break;

        case CYBLE_EVT_GATTS_HANDLE_VALUE_CNF:
        break;

        case CYBLE_EVT_GATTS_DATA_SIGNED_CMD_REQ:
        break;


        /* GATT Server events (CYBLE_EVT_T) */

        case CYBLE_EVT_GATTS_INDICATION_ENABLED:
        break;

        case CYBLE_EVT_GATTS_INDICATION_DISABLED:
        break;


        /* L2CAP events */

        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ:
        break;

        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:
        break;

        case CYBLE_EVT_L2CAP_COMMAND_REJ:
        break;

        case CYBLE_EVT_L2CAP_CBFC_CONN_IND:
        break;

        case CYBLE_EVT_L2CAP_CBFC_CONN_CNF:
        break;

        case CYBLE_EVT_L2CAP_CBFC_DISCONN_IND:
        break;

        case CYBLE_EVT_L2CAP_CBFC_DISCONN_CNF:
        break;

        case CYBLE_EVT_L2CAP_CBFC_DATA_READ:
        break;

        case CYBLE_EVT_L2CAP_CBFC_RX_CREDIT_IND:
        break;

        case CYBLE_EVT_L2CAP_CBFC_TX_CREDIT_IND:
        break;

        case CYBLE_EVT_L2CAP_CBFC_DATA_WRITE_IND:
        break;


        /* default catch-all case */

        default:
        break;
    }
}
