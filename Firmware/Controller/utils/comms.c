#include "comms.h"
#include "usbd_cdc_if.h"
#include <string.h>
#include "utils.h"
#include "crc.h"
#include "timing.h"
#include "controller.h"
/*******************************************************************************
 *                          TUNERSTUDIO PROTOCOL PACKET FORMAT
 * 
 * ┌──────────────────────────────────────────────────────────────────────────┐
 * │                            HOST → ECU REQUEST                            │
 * ├──────────┬────────────────────────────────────────────────┬──────────────┤
 * │   SIZE   │              PAYLOAD (CMD + DATA)              │    CRC32     │
 * └──────────┴────────────────────────────────────────────────┴──────────────┘
 * 
 *    2 bytes                      N bytes                         4 bytes
 * 
 * 
 * ┌───────────────────────────────────────────────────────────────────────────┐
 * │                            ECU → HOST RESPONSE                            │
 * ├──────────┬──────────┬──────────────────────────────────────┬──────────────┤
 * │   SIZE   │   FLAG   │           PAYLOAD (DATA)             │    CRC32     │
 * └──────────┴──────────┴──────────────────────────────────────┴──────────────┘
 * 
 *    2 bytes    1 byte                 N bytes                    4 bytes
 * 
 * NOTES:
 * - SIZE: Big-endian uint16 (payload length EXCLUDING CRC)
 * - FLAG: 0x00=OK, 0x7F=ERROR, 0x5A=DATA_PENDING
 * - CRC32: Calculated over Size+Payload (EXCLUDING the CRC itself)
 * - All multi-byte fields use BIG-ENDIAN byte order
 * - Minimum packet size: 7 bytes (2+1+0+4 for empty response)
 * For every packet sent to the Megasquirt, there will be a response packet, so the sender can validate correct
reception. The sender MUST act on the response code and MUST check the CRC32.
 ******************************************************************************/
// ==================== Global Variables ====================
osMessageQueueId_t usb_rx_queue;

usb_rx_packet_t rx_packet;
/**
 * See 'blockingFactor' in .ini
 */
uint8_t tx_buffer[TS_BLOCKING_FACTOR + 30];
uint8_t rx_buffer[TS_BLOCKING_FACTOR + 30];

/* Function delcearations  */
/**
 * @note size can be zero to transmit an empty packet
 */
void send_response(uint8_t flag, uint8_t *data, size_t size, comms_response_format_t mode);
void transmit_crc_packet(uint8_t flag, const uint8_t *buf, size_t size);
bool process_plain_command(uint8_t *request, uint16_t size);
void process_command(uint8_t *request, uint16_t size);
void handle_page_read_command(uint16_t page, uint16_t offset, uint16_t count);
void handle_page_write_command(uint16_t page, uint16_t offset, uint16_t count);

/* Function definations */

void send_response(uint8_t flag, uint8_t *data, size_t size, comms_response_format_t mode)
{

    if (mode == TS_CRC)
    {
        transmit_crc_packet(flag, data, size);
    }
    else
    {
        if (size > 0)
        {
			memcpy(tx_buffer, data, size);
            CDC_Transmit_FS(tx_buffer, size);
        }
    }
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}

void transmit_crc_packet(uint8_t flag, const uint8_t *buf, size_t size)
{

	/* We first calculate the prefix */
    uint16_t packet_size = 0;
	packet_size += 2; // prefix
	packet_size += 1; // flag
	packet_size += 4; // CRC
	packet_size += size; // payload

	/* Converte the size to big endian */
	uint16_t prefix = 0;
	prefix = swap_endian_uint16(sizeof(flag) + size);

	/* Now we calculate the CRC, over the flag and payload */
	uint32_t crc = 0;
	crc = crc32_inc(0, (void*)&flag, 1); // flag
	crc = crc32_inc(crc, buf, size); // payload

	/* Converte the crc to big endian */
	uint32_t suffix = 0;
	suffix = swap_endian_uint32(crc);

	/* Form the packet in the transmit buffer */
	size_t tx_buffer_index = 0;
	memcpy(tx_buffer, &prefix, sizeof(prefix)); // prefix to buffer
	tx_buffer_index += sizeof(prefix);
	memcpy(tx_buffer + tx_buffer_index, &flag, sizeof(flag)); // flag to buffer
	tx_buffer_index += sizeof(flag);
	memcpy(tx_buffer + tx_buffer_index, buf, size); // payload to buffer
	tx_buffer_index += size;
	memcpy(tx_buffer + tx_buffer_index, &suffix, sizeof(suffix)); // suffix to buffer
	tx_buffer_index += sizeof(suffix);

	/* Finally transmit over USB */

	CDC_Transmit_FS(tx_buffer, packet_size);
}



// ==================== Initialization ====================
void comms_init(void)
{
	usb_rx_queue = osMessageQueueNew(10, sizeof(usb_packet_ptr_t), NULL);

    const osThreadAttr_t comms_task_attrs = {
        .name = "comms_task",
        .stack_size = 1024 * 4,
        .priority = osPriorityNormal,
    };
    osThreadNew(comms_task, NULL, &comms_task_attrs);

}

// ==================== Communication Task ====================
void comms_task(void *argument)
{
    usb_packet_ptr_t packet;
    for (;;)
    {
        if (osMessageQueueGet(usb_rx_queue, &packet, NULL, osWaitForever) == osOK)
        {
            process_command(packet.data, packet.len);
        }
    }
}

// ==================== Command Processing ====================
bool process_plain_command(uint8_t *request, uint16_t size)
{
    uint8_t first_byte = request[0];
    switch (first_byte)
    {
    case TS_COMMAND_F:
    #ifndef TS_USE_OLD_PROTOCOL
        send_response(0, (uint8_t*)TS_PROTOCOL, sizeof(TS_PROTOCOL) - 1, TS_PLAIN);
        return true;
    #endif
        break;
    case TS_HELLO_COMMAND:
        send_response(0, (uint8_t*)TS_SIGNATURE, sizeof(TS_SIGNATURE) - 1, TS_PLAIN);
        return true;
        break;
    case TS_QUERY_COMMAND:
        send_response(0, (uint8_t*)TS_SIGNATURE, sizeof(TS_SIGNATURE) - 1, TS_PLAIN);
        return true;
        break;
    case TS_TEST_COMMS_COMMAND:
        send_response(0, (uint8_t *)0xFF, 1, TS_PLAIN);
        return true;
        break;
    case TS_CAN_ID_COMMAND:
        send_response(0, (uint8_t*)TS_CAN_ID, sizeof(TS_CAN_ID) - 1, TS_PLAIN);
        return true;
        break;

    default:
        break;
    }
        // This wasn't a valid command
        return false;
    
}

void process_command(uint8_t *request, uint16_t size)
{
    if (process_plain_command(request, size))
    {
        return;
    }
    uint16_t packet_size = 0;
    uint32_t packet_crc = 0;
    uint32_t calculated_packet_crc = 0;
    bool is_packet_valid = false;
    if (size > 5)
    {
        packet_size = swap_endian_uint16(*(uint16_t *)request);
        packet_crc = swap_endian_uint32(*(uint32_t *)request[size - 1 - TS_PACKET_CRC_SIZE]);
        calculated_packet_crc = crc32_inc(0, (void *)(request + TS_PACKET_PREFIX_SIZE), size - TS_PACKET_PREFIX_SIZE - TS_PACKET_CRC_SIZE);
    }
    if (calculated_packet_crc == packet_crc)
    {
        is_packet_valid = true;
    }

    uint8_t command = request[2];

    switch (command)
    {
    case TS_TEST_COMMS_COMMAND:
        send_response(TS_RESPONSE_OK, (uint8_t *)0xFF, 1, TS_CRC);
        return;
        break;

    case TS_COMMAND_F:
#ifndef TS_USE_OLD_PROTOCOL
        send_response(TS_RESPONSE_OK, (uint8_t *)TS_PROTOCOL, sizeof(TS_PROTOCOL) - 1, TS_CRC);
#endif
        return;
        break;

    case TS_CAN_ID_COMMAND:
        send_response(TS_RESPONSE_OK, (uint8_t *)TS_CAN_ID, sizeof(TS_CAN_ID), TS_CRC);
        return;
        break;

    case TS_QUERY_COMMAND:
        send_response(TS_RESPONSE_OK, (uint8_t *)TS_SIGNATURE, sizeof(TS_SIGNATURE) - 1, TS_CRC);
        return;
        break;

    case TS_HELLO_COMMAND:
        send_response(TS_RESPONSE_OK, (uint8_t *)TS_SIGNATURE, sizeof(TS_SIGNATURE), TS_CRC);
        return;
        break;
    case TS_SERIAL_INFO_COMMAND:
        uint8_t response[5];
        response[0] = 2; // serial version
        *(uint16_t *)&response[1] = swap_endian_uint16(TS_TABLE_BLOCKING_FACTOR);
        *(uint16_t *)&response[3] = swap_endian_uint16(TS_BLOCKING_FACTOR);
        send_response(TS_RESPONSE_OK, (uint8_t *)response, sizeof(response), TS_CRC);
        return;
        break;

    case TS_OUTPUT_COMMAND:
        send_response(TS_RESPONSE_OK, (uint8_t *)&runtime, sizeof(runtime), TS_CRC);
        return;
        break;
    case TS_READ_COMMAND:
        uint16_t page = 0;
        uint16_t offset = *(uint16_t *)&request[3];
        uint16_t size = *(uint16_t *)&request[5];
        handle_page_read_command(page, offset, size);
        return;
        break;
    case TS_CRC_CHECK_COMMAND:
        uint32_t page_crc = crc32_inc(0, (uint8_t *)&config, sizeof(config));
        send_response(TS_RESPONSE_OK, (uint8_t *)&page_crc, sizeof(page_crc), TS_CRC);
        return;
        break;

    case TS_CHUNK_WRITE_COMMAND:
    {
        uint16_t page = 0;
        uint16_t offset = *(uint16_t *)&request[3];
        uint16_t size = *(uint16_t *)&request[5];
        memcpy((uint8_t *)&config + offset, &request[7], size);
        send_response(TS_RESPONSE_OK, NULL, 0, TS_CRC);
        return;
        break;
    }
    case TS_SINGLE_WRITE_COMMAND:
    {
        uint16_t page = 0;
        uint16_t offset = *(uint16_t *)&request[3];
        uint16_t size = *(uint16_t *)&request[5];
        memcpy((uint8_t *)&config + offset, &request[7], size);
        send_response(TS_RESPONSE_OK, NULL, 0, TS_CRC);
        return;
        break;
    }

    case TS_BURN_COMMAND:
        // handle burn command and if it was ok, then send ok status
        send_response(TS_RESPONSE_BURN_OK, NULL, 0, TS_CRC);
        return;
        break;
    default:
        break;
    }
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}

void handle_page_read_command(uint16_t page, uint16_t offset, uint16_t count)
{
	if (page > 1)
	{
		return;
	}
	if ((offset + count) > TS_PAGE_SIZE)
	{
		return;
	}

	send_response(TS_RESPONSE_OK, (uint8_t*)&config + offset, count, TS_CRC);
}

void handle_page_write_command(uint16_t page, uint16_t offset, uint16_t count)
{
	if (page > 1)
	{
		return;
	}
	if ((offset + count) > TS_PAGE_SIZE)
	{
		return;
	}
	
}

