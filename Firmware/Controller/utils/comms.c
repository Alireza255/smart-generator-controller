#include "comms.h"
#include "usbd_cdc_if.h"
#include <string.h>
#include "utils.h"
#include "crc.h"
#include "timing.h"
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
calibration_page cal_page = {
    .start_address = 0x08008000,
    .modified = 0};
runtime_data runtime_values;
osMutexId_t runtime_mutex;
osMutexId_t page_mutex;
usb_rx_packet_t rx_packet;
/**
 * See 'blockingFactor' in .ini
 */
static uint8_t tx_buffer[BLOCKING_FACTOR + 30];


/* Function delcearations  */
/**
 * @note size can be zero to transmit an empty packet
 */
void send_response(uint8_t *data, size_t size, comms_response_format_t mode);
void transmit_crc_packet(uint8_t flag, const uint8_t *buf, size_t size);
static void transmit_ok_response();
bool process_plain_command(uint8_t *cmd, uint16_t size);
void process_command(uint8_t *cmd, uint16_t size);



/* Function definations */

void send_response(uint8_t *data, size_t size, comms_response_format_t mode)
{

    if (mode == TS_CRC)
    {
        transmit_crc_packet(TS_RESPONSE_OK, data, size);
    }
    else
    {
        if (size > 0)
        {
            CDC_Transmit_FS(data, size);
        }
    }
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}

void transmit_crc_packet(uint8_t flag, const uint8_t *buf, size_t size)
{
    /* Determine the size first */
    if (!buf || buf == NULL)
    {
        size = 0;
    }
	/* We first calculate the prefix */
    uint16_t packet_size = 0;
	packet_size += 2; // prefix
	packet_size += 1; // flag
	packet_size += 4; // CRC
	packet_size += size; // payload

	/* Converte the size to big endian */
	uint16_t prefix = 0;
	prefix = swap_endian_uint16(packet_size);

	/* Now we calculate the CRC, over the flag and payload */
	uint32_t crc = 0;
	crc += crc32_inc(0, (void*)flag, 1); // flag
	crc += crc32_inc(crc, buf, size); // payload

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


static void transmit_ok_response()
{
    send_response(NULL, 0, TS_CRC);
}

// ==================== Initialization ====================
void comms_init(void)
{
    usb_rx_queue = osMessageQueueNew(16, sizeof(usb_rx_packet_t), NULL);

    const osMutexAttr_t mutex_attrs = {
        .name = "comms_mutex",
        .attr_bits = osMutexRecursive | osMutexPrioInherit,
    };
    runtime_mutex = osMutexNew(&mutex_attrs);
    page_mutex = osMutexNew(&mutex_attrs);

    if (osMutexAcquire(page_mutex, osWaitForever) == osOK)
    {
        memcpy(cal_page.data, (void *)cal_page.start_address, PAGE_SIZE);
        osMutexRelease(page_mutex);
    }

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
    
    for (;;)
    {
        if (osMessageQueueGet(usb_rx_queue, &rx_packet, NULL, osWaitForever) == osOK)
        {
            process_command(rx_packet.data, rx_packet.len);
        }
    }
}

// ==================== Command Processing ====================
bool process_plain_command(uint8_t *cmd, uint16_t size)
{
    uint8_t first_byte = cmd[0];
    // Bail fast if guaranteed not to be a plain command
	switch (first_byte)
	{
	case TS_COMMAND_F:
		send_response((uint8_t*)TS_PROTOCOL, sizeof(TS_PROTOCOL) - 1, TS_PLAIN);
		return true;
		break;
	case TS_HELLO_COMMAND:
		send_response((uint8_t*)TS_SIGNATURE, sizeof(TS_SIGNATURE) - 1, TS_PLAIN);
		return true;
		break;
	case TS_QUERY_COMMAND:
		send_response((uint8_t*)TS_SIGNATURE, sizeof(TS_SIGNATURE) - 1, TS_PLAIN);
		return true;
		break;
	case TS_TEST_COMMS_COMMAND:
		uint16_t seconds_running = (uint16_t)(get_time_ms() / 1000U);
		send_response((uint8_t*)&seconds_running, sizeof(seconds_running), TS_PLAIN);
		return true;
		break;
	case TS_CAN_ID_COMMAND:
		uint8_t id = TS_CAN_ID;
		send_response((uint8_t*)&id, sizeof(id), TS_PLAIN);
		return true;
		break;
	
	default:
		break;
	}
		// This wasn't a valid command
		return false;
	
}

void process_command(uint8_t *cmd, uint16_t size)
{
    if (process_plain_command(cmd, size))
    {
        return;
    }
	uint8_t command = cmd[2];

	switch (command)
	{
	case TS_TEST_COMMS_COMMAND:
		uint16_t seconds_running = (uint16_t)(get_time_ms() / 1000U);
		send_response((uint8_t*)&seconds_running, sizeof(seconds_running), TS_CRC);
		break;

	case TS_COMMAND_F:
		send_response((uint8_t *)TS_PROTOCOL, sizeof(TS_PROTOCOL) - 1, TS_CRC);
		break;

	case TS_CAN_ID_COMMAND:
		uint8_t id = TS_CAN_ID;
		send_response((uint8_t *)&id, sizeof(id), TS_CRC);
		break;
	case TS_QUERY_COMMAND:
		send_response((uint8_t *)TS_SIGNATURE, sizeof(TS_SIGNATURE) - 1, TS_CRC);

		break;
	case TS_HELLO_COMMAND:
		send_response((uint8_t *)TS_SIGNATURE, sizeof(TS_SIGNATURE) - 1, TS_CRC);

		break;

	default:
		break;
	}
	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
#if FALSE
 	uint8_t secondByte;
	/* second byte should be received within minimal delay */
	received = tsChannel->readTimeout(&secondByte, 1, TS_COMMUNICATION_TIMEOUT_SHORT);
	if (received != 1) {
		tunerStudioError(tsChannel, "TS: ERROR: no second byte");
		tsChannel->in_sync = false;
		return -1;
	}

	uint16_t incomingPacketSize = firstByte << 8 | secondByte;
	size_t expectedSize = incomingPacketSize + TS_PACKET_TAIL_SIZE;

	if ((incomingPacketSize == 0) || (expectedSize > sizeof(tsChannel->scratchBuffer))) {
		if (tsChannel->in_sync) {
			efiPrintf("process_ts: channel=%s invalid size: %d", tsChannel->name, incomingPacketSize);
			tunerStudioError(tsChannel, "process_ts: ERROR: packet size");
			/* send error only if previously we were in sync */
			sendErrorCode(tsChannel, TS_RESPONSE_OUT_OF_RANGE, "invalid size");
		}
		tsChannel->in_sync = false;
		return -1;
	}

	char command;
	if (tsChannel->in_sync) {
		/* we are in sync state, packet size should be correct so lets receive full packet and then check if command is supported
		 * otherwise (if abort reception in middle of packet) it will break synchronization and cause error on next packet */
		received = tsChannel->readTimeout((uint8_t*)(tsChannel->scratchBuffer), expectedSize, TS_COMMUNICATION_TIMEOUT);
		command = tsChannel->scratchBuffer[0];

		if (received != expectedSize) {
			/* print and send error as we were in sync */
			efiPrintf("Got only %d bytes while expecting %d for command 0x%02x", received,
					expectedSize, command);
			tunerStudioError(tsChannel, "ERROR: not enough bytes in stream");
			// MS serial protocol spec: There was a timeout before all data was received. (25ms per character.)
			sendErrorCode(tsChannel, TS_RESPONSE_UNDERRUN, "underrun");
			tsChannel->in_sync = false;
			return -1;
		}

		if (!isKnownCommand(command)) {
			/* print and send error as we were in sync */
			efiPrintf("unexpected command %x", command);
			sendErrorCode(tsChannel, TS_RESPONSE_UNRECOGNIZED_COMMAND, "unknown");
			tsChannel->in_sync = false;
			return -1;
		}
	} else {
		/* receive only command byte to check if it is supported */
		received = tsChannel->readTimeout((uint8_t*)(tsChannel->scratchBuffer), 1, TS_COMMUNICATION_TIMEOUT_SHORT);
		command = tsChannel->scratchBuffer[0];

		if (!isKnownCommand(command)) {
			/* do not report any error as we are not in sync */
			return -1;
		}

		received = tsChannel->readTimeout((uint8_t*)(tsChannel->scratchBuffer) + 1, expectedSize - 1, TS_COMMUNICATION_TIMEOUT);
		if (received != expectedSize - 1) {
			/* do not report any error as we are not in sync */
			return -1;
		}
	}

#if EFI_SIMULATOR
	logMsg("command %c\r\n", command);
#endif

	uint32_t expectedCrc = *(uint32_t*) (tsChannel->scratchBuffer + incomingPacketSize);

	expectedCrc = SWAP_UINT32(expectedCrc);

	uint32_t actualCrc = crc32(tsChannel->scratchBuffer, incomingPacketSize);
	if (actualCrc != expectedCrc) {
		/* send error only if previously we were in sync */
		if (tsChannel->in_sync) {
			efiPrintf("TunerStudio: command %c actual CRC %x/expected %x", tsChannel->scratchBuffer[0],
					(unsigned int)actualCrc, (unsigned int)expectedCrc);
			tunerStudioError(tsChannel, "ERROR: CRC issue");
			sendErrorCode(tsChannel, TS_RESPONSE_CRC_FAILURE, "crc_issue");
			tsChannel->in_sync = false;
		}
		return -1;
	}

	/* we were able to receive known command with correct crc and size! */
	tsChannel->in_sync = true;

	int success = tsInstance.handleCrcCommand(tsChannel, tsChannel->scratchBuffer, incomingPacketSize);

	if (!success) {
		efiPrintf("got unexpected TunerStudio command %x:%c", command, command);
		return -1;
	}

	return 0;
#endif
}

// ==================== Runtime Update Task ====================
void runtime_update_task(void *argument)
{
    for (;;)
    {
        if (osMutexAcquire(runtime_mutex, osWaitForever) == osOK)
        {
            // Replace with actual sensor readings
            runtime_values.rpm = 1;
            runtime_values.map = 2;
            runtime_values.tps = 3;
            runtime_values.lambda = 4;
            runtime_values.advance = 5;
            runtime_values.dwell = 6;
            runtime_values.vbatt = 7;
            runtime_values.clt = 8;
            osMutexRelease(runtime_mutex);
        }
        osDelay(20);
    }
}