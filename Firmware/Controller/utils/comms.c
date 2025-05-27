#include "comms.h"
#include "usbd_cdc_if.h" // For CDC_Transmit_FS
#include "controller.h" // For signature and version strings
#include <string.h> // For strlen(), memcmp()

// Global handle for the message queue (defined here as it's created in main)
osMessageQueueId_t usb_rx_queue_handle;

osThreadId_t comm_task_handle;



typedef enum {
	TS_PLAIN = 0,
	TS_CRC = 1
} ts_response_format_e;

void comms_send_response(ts_response_format_e mode, const uint8_t *buffer, size_t size);
void comms_write_crc_packet(uint8_t response_code, const uint8_t *buffer, size_t size);
void serial_write(const uint8_t* buffer, size_t size);

void comms_init()
{
    // usb_rx_queue_handle is declared extern in comms.h
    usb_rx_queue_handle = osMessageQueueNew(
      8, // Queue size: Can hold 8 usb_rx_packet_t messages
      sizeof(usb_rx_packet_t),
      NULL // Default attributes
    );
    // Define and create the communication task
    const osThreadAttr_t comm_task_attributes = {
    .name = "CommTask", // Name for debugging
    .stack_size = 512 * 4, // Stack size in bytes (adjust as needed, 512 words typical)
    .priority = (osPriority_t) osPriorityNormal, // Task priority
    };
    comm_task_handle = osThreadNew(comm_task_entry, NULL, &comm_task_attributes);
}

// --- Task Entry Function ---
void comm_task_entry(void *argument) {
    for (;;) {
        usb_rx_packet_t received_packet;
        osStatus_t status = osMessageQueueGet(usb_rx_queue_handle, &received_packet, NULL, osWaitForever); // 1-second timeout
        if (status != osOK) { continue; }
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        volatile uint8_t command;
        command = received_packet.data[0];
        if (command == 0)
        {
            continue;// ignore null commands
        }
        else if (command == TS_QUERY_COMMAND || command == TS_HELLO_COMMAND)
        {
            
            CDC_Transmit_FS(TS_SIGNATURE, sizeof(TS_SIGNATURE));
        }
        else if (command == TS_COMMAND_F)
        {
            CDC_Transmit_FS(TS_PROTOCOL, sizeof(TS_PROTOCOL));
        }
        
    }
    
}

void comms_send_response(ts_response_format_e mode, const uint8_t *buffer, size_t size)
{
    if (size == 0)
        return;
    if (mode == TS_CRC)
    {
        // writeCrcPacket(TS_RESPONSE_OK, buffer, size);
    }
    else
    {
        serial_write(buffer, size);
    }
}

void serial_write(const uint8_t* buffer, size_t size)
{
	
    if (size == 0)
    {
        return;
    }
    
}