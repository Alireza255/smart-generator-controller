#ifndef COMMS_H
#define COMMS_H

#include "cmsis_os2.h"
#include <stdint.h>
#include <stdbool.h> // For bool

/// @defgroup group-serial-return-codes Serial return codes sent to TS
/// @{
static const uint8_t SERIAL_RC_OK         = 0x00U; //!< Success
static const uint8_t SERIAL_RC_REALTIME   = 0x01U; //!< Unused
static const uint8_t SERIAL_RC_PAGE       = 0x02U; //!< Unused
static const uint8_t SERIAL_RC_BURN_OK    = 0x04U; //!< EEPROM write succeeded
static const uint8_t SERIAL_RC_TIMEOUT    = 0x80U; //!< Timeout error
static const uint8_t SERIAL_RC_CRC_ERR    = 0x82U; //!< CRC mismatch
static const uint8_t SERIAL_RC_UKWN_ERR   = 0x83U; //!< Unknown command
static const uint8_t SERIAL_RC_RANGE_ERR  = 0x84U; //!< Incorrect range. TS will not retry command
static const uint8_t SERIAL_RC_BUSY_ERR   = 0x85U; //!< TS will wait and retry
///@}

// Define the signature and version strings that your controller will send
static const uint8_t TS_SIGNATURE[] = {SERIAL_RC_OK, 'S', 'G', 'C'};
static const uint8_t TS_FIRMWARE_VERSION[] = {SERIAL_RC_OK, 'S', 'G', 'C'};
static const uint8_t TS_SERIAL_VERSION[] = {SERIAL_RC_OK, '0', '0', '2'};

// Define the commands TunerStudio will send
#define TS_HELLO_COMMAND 'S'
#define TS_QUERY_COMMAND 'Q'
#define TS_COMMAND_F 'F'
#define TS_PACKET_HEADER_SIZE	3
#define TS_PACKET_TAIL_SIZE		4

#define TS_BLOCK_READ_TIMEOUT 3000
#define TS_BURN_COMMAND 'B'
#define TS_BURN_COMMAND_char B
#define TS_CHUNK_WRITE_COMMAND 'C'
#define TS_CHUNK_WRITE_COMMAND_char C
#define TS_COMMAND_F_char F
#define TS_COMPOSITE_DISABLE 2
#define TS_COMPOSITE_ENABLE 1
#define TS_COMPOSITE_READ 3
#define TS_CRC_CHECK_COMMAND 'k'
#define TS_CRC_CHECK_COMMAND_char k
#define TS_EXECUTE 'E'
#define TS_EXECUTE_char E
#define TS_FILE_VERSION 20250101
#define TS_FILE_VERSION_OFFSET 124
#define TS_GET_COMPOSITE_BUFFER_DONE_DIFFERENTLY '8'
#define TS_GET_COMPOSITE_BUFFER_DONE_DIFFERENTLY_char 8
#define TS_GET_CONFIG_ERROR 'e'
#define TS_GET_CONFIG_ERROR_char e
#define TS_GET_FIRMWARE_VERSION 'V'
#define TS_GET_FIRMWARE_VERSION_char V
#define TS_GET_OUTPUTS_SIZE '4'
#define TS_GET_OUTPUTS_SIZE_char 4
#define TS_GET_PROTOCOL_VERSION_COMMAND_F 'F'
#define TS_GET_PROTOCOL_VERSION_COMMAND_F_char F
#define TS_GET_SCATTERED_GET_COMMAND '9'
#define TS_GET_SCATTERED_GET_COMMAND_char 9
#define TS_GET_TEXT 'G'
#define TS_GET_TEXT_char G
#define TS_IO_TEST_COMMAND 'Z'
#define TS_IO_TEST_COMMAND_char Z
#define TS_ONLINE_PROTOCOL 'z'
#define TS_ONLINE_PROTOCOL_char z
#define TS_OUTPUT_ALL_COMMAND 'A'
#define TS_OUTPUT_ALL_COMMAND_char A
#define TS_OUTPUT_COMMAND 'O'
#define TS_OUTPUT_COMMAND_char O
#define TS_PERF_TRACE_BEGIN '_'
#define TS_PERF_TRACE_BEGIN_char _
#define TS_PERF_TRACE_GET_BUFFER 'b'
#define TS_PERF_TRACE_GET_BUFFER_char b
#define TS_PROTOCOL "002"
#define TS_QUERY_BOOTLOADER 'L'
#define TS_QUERY_BOOTLOADER_char L
#define TS_QUERY_BOOTLOADER_NONE 0
#define TS_QUERY_BOOTLOADER_OPENBLT 1
#define TS_QUERY_COMMAND_char Q
#define TS_READ_COMMAND 'R'
#define TS_READ_COMMAND_char R
#define TS_RESPONSE_BURN_OK 4
#define TS_RESPONSE_CRC_FAILURE 0x82
#define TS_RESPONSE_FRAMING_ERROR 0x8D
#define TS_RESPONSE_OK 0
#define TS_RESPONSE_OUT_OF_RANGE 0x84
#define TS_RESPONSE_OVERRUN 0x81
#define TS_RESPONSE_UNDERRUN 0x80
#define TS_RESPONSE_UNRECOGNIZED_COMMAND 0x83


// Define size for incoming USB CDC packets
#define USB_RX_PACKET_MAX_SIZE 64 // Max size for SGC_SIG or SGC_VER commands

// --- PROTOCOL CONSTANTS ---
#define PROTOCOL_LEN_FIELD_LEN      2   // 2 bytes for the payload length header
#define PROTOCOL_RETURN_CODE_LEN    1   // 1 byte for the success/error code (0x00 for OK)
#define PROTOCOL_CRC_LEN            4   // 4 bytes for the CRC32 checksum

// Standard Success Return Code
#define RET_OK                      0x00U


// Structure for a received USB packet to be put into the queue
typedef struct {
    uint8_t  data[USB_RX_PACKET_MAX_SIZE];
    uint32_t len;
} usb_rx_packet_t;

// Global handle for the message queue (used by CDC_Receive_FS and comm_task_entry)
extern osMessageQueueId_t usb_rx_queue_handle;

// Task function prototype
void comm_task_entry(void *argument);

void comms_init();

// --- CRC32 Function Prototype ---
uint32_t calculate_crc32(const uint8_t *data, uint16_t length);

#endif // COMMS_H