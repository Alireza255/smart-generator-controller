#ifndef COMMS_H
#define COMMS_H
#define COMMS_ENABLED true

#if COMMS_ENABLED == true
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include <stdint.h>
#include "usbd_cdc_if.h"
#include <stdbool.h>
#include "config_and_runtime.h"
/**
 * CRC packets are implemend like this:
 * size in the beginning of a packet and a crc32 at the end.
 * |size or status?||data||crc32|
 */

// ==================== Configuration ====================
#define TS_PAGE_SIZE sizeof(config_t)

#define TS_BLOCKING_FACTOR 1024 // same blocking factor defined in the .ini file
#define TS_TABLE_BLOCKING_FACTOR 1024 
//#define TS_USE_OLD_PROTOCOL
// ==================== Status Codes ====================
#define TS_PACKET_PREFIX_SIZE   2
#define TS_PACKET_FLAG_SIZE     1
#define TS_PACKET_CRC_SIZE      4

#define TS_PROTOCOL "002"
#define TS_CAN_ID (uint8_t)0
#define TS_CAN_ID_COMMAND 'I'
#define TS_SIGNATURE "JimStim dev.2025.07"
#define TS_SERIAL_INFO_COMMAND 'f'
#define TS_SIMULATE_CAN '>'
#define TS_SIMULATE_CAN_char >
#define TS_SINGLE_WRITE_COMMAND 'w'
#define TS_CHUNK_WRITE_COMMAND 'W'
#define TS_TEST_COMMAND 't'
#define TS_BURN_COMMAND 'B'
#define TS_TEST_COMMAND_char t
#define TS_TOTAL_OUTPUT_SIZE 1800
#define TS_TRIGGER_SCOPE_CHANNEL_1_NAME "Channel 1"
#define TS_TRIGGER_SCOPE_CHANNEL_2_NAME "Channel 2"
#define TS_TRIGGER_SCOPE_DISABLE 5
#define TS_TRIGGER_SCOPE_ENABLE 4
#define TS_TRIGGER_SCOPE_READ 6
#define TS_BLOCK_READ_TIMEOUT 3000
#define TS_RESPONSE_BURN_OK 0x04

#define ts_cic_idle false
#define ts_command_e_TS_BENCH_CATEGORY 22
#define ts_command_e_TS_BOARD_ACTION 29
#define ts_command_e_TS_CLEAR_WARNINGS 17
#define ts_command_e_TS_COMMAND_1 1
#define ts_command_e_TS_COMMAND_10 10
#define ts_command_e_TS_COMMAND_11 11
#define ts_command_e_TS_COMMAND_12 12
#define ts_command_e_TS_COMMAND_13 13
#define ts_command_e_TS_COMMAND_14 14
#define ts_command_e_TS_COMMAND_15 15
#define ts_command_e_TS_COMMAND_16 16
#define ts_command_e_TS_COMMAND_2 2
#define ts_command_e_TS_COMMAND_3 3
#define ts_command_e_TS_COMMAND_4 4
#define ts_command_e_TS_COMMAND_5 5
#define ts_command_e_TS_COMMAND_6 6
#define ts_command_e_TS_COMMAND_7 7
#define ts_command_e_TS_COMMAND_8 8
#define ts_command_e_TS_COMMAND_9 9
#define ts_command_e_TS_DEBUG_MODE 0
#define ts_command_e_TS_IGNITION_CATEGORY 18
#define ts_command_e_TS_INJECTOR_CATEGORY 19
#define ts_command_e_TS_LUA_OUTPUT_CATEGORY 32
#define ts_command_e_TS_SET_DEFAULT_ENGINE 31
#define ts_command_e_TS_SET_ENGINE_TYPE 30
#define ts_command_e_TS_SOLENOID_CATEGORY 25
#define ts_command_e_TS_UNUSED_23 23
#define ts_command_e_TS_UNUSED_24 24
#define ts_command_e_TS_UNUSED_26 26
#define ts_command_e_TS_UNUSED_27 27
#define ts_command_e_TS_UNUSED_28 28
#define ts_command_e_TS_WIDEBAND 21
#define ts_command_e_TS_X14 20
#define TS_COMMAND_F 'F'
#define TS_COMMAND_F_char F
#define TS_COMPOSITE_DISABLE 2
#define TS_COMPOSITE_ENABLE 1
#define TS_COMPOSITE_READ 3
#define TS_CRC_CHECK_COMMAND 'k'
#define TS_CRC_CHECK_COMMAND_char k
#define TS_EXECUTE 'E'
#define TS_TEST_COMMS_COMMAND 'C'
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
#define TS_HELLO_COMMAND 'S'
#define TS_HELLO_COMMAND_char S
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
#define TS_QUERY_BOOTLOADER 'L'
#define TS_QUERY_BOOTLOADER_char L
#define TS_QUERY_BOOTLOADER_NONE 0
#define TS_QUERY_BOOTLOADER_OPENBLT 1
#define TS_QUERY_COMMAND 'Q'
#define TS_QUERY_COMMAND_char Q
#define TS_READ_COMMAND 'r'
#define TS_READ_COMMAND_char r
#define TS_RESPONSE_CRC_FAILURE 0x82
#define TS_RESPONSE_FRAMING_ERROR 0x8D
#define TS_RESPONSE_OK 0
#define TS_RESPONSE_OUT_OF_RANGE 0x84
#define TS_RESPONSE_OVERRUN 0x81
#define TS_RESPONSE_UNDERRUN 0x80
#define TS_RESPONSE_UNRECOGNIZED_COMMAND 0x83
#define TS_SET_LOGGER_SWITCH 'l'
#define TS_SET_LOGGER_SWITCH_char l

// ==================== Type Definitions ====================
typedef enum
{
    TS_PLAIN = 0,
    TS_CRC = 1
} comms_response_format_t;

// ==================== USB CDC Configuration ====================
#define USB_RX_QUEUE_SIZE    10  // Number of packets to buffer
#define USB_MAX_PACKET_SIZE  64  // Max CDC packet size (64 bytes for full-speed USB)

typedef struct {
    uint8_t data[USB_MAX_PACKET_SIZE];
    uint16_t len;
} usb_rx_packet_t;

typedef struct {
    uint8_t *data;
    uint32_t len;
} usb_packet_ptr_t;

extern osMessageQueueId_t usb_rx_queue;

// ==================== Function Prototypes ====================
void comms_init();
void comms_task(void *argument);
void process_command(uint8_t *cmd, uint16_t len);

#endif
#endif // COMMS_H