/*! @file radio_config.h
 * @brief This file contains the automatically generated
 * configurations.
 *
 * @n WDS GUI Version: 3.2.3.0
 * @n Device: Si4455 Rev.: B1                                 
 *
 * @b COPYRIGHT
 * @n Silicon Laboratories Confidential
 * @n Copyright 2013 Silicon Laboratories, Inc.
 * @n http://www.silabs.com
 */

#ifndef RADIO_CONFIG_H_
#define RADIO_CONFIG_H_

// USER DEFINED PARAMETERS
// Define your own parameters here

// INPUT DATA
/*
// %%	Crys_freq(Hz)	Crys_tol(ppm)	IF_mode	High_perf_Ch_Fil	OSRtune	Ch_Fil_Bw_AFC	ANT_DIV	PM_pattern
// 	30000000	30	2	1	0	0	0	0
//%%	MOD_type	Rsymb(sps)	Fdev(Hz)	RXBW(Hz)	Mancheste	AFC_en	Rsymb_error	Chip-Version
// 	2	10000	30000	200000	0	1	0.0	2
//%%	RF Freq.(MHz)	API_TC	fhst	inputBW	BERT	RAW_dout	D_source	Hi_pfm_div
// 	915	28	250000	0	0	0	0	0
//
// # WB filter 3 (BW = 185.22 kHz);  NB-filter 3 (BW = 185.22 kHz) 
// 
// Modulation index: 6
*/


// CONFIGURATION PARAMETERS
#define RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ                     {30000000L}
#define RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER                    {0x00}
#define RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH               {0x4B}
#define RADIO_CONFIGURATION_DATA_RADIO_STATE_AFTER_POWER_UP        {0x03}
#define RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET       {0xF000}
#define RADIO_CONFIGURATION_DATA_CUSTOM_LONG_PAYLOAD					   {0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, \
0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, \
0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, \
0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5, 0xC5}


// CONFIGURATION COMMANDS

/*
// Command:                  RF_POWER_UP
// Description:              Power-up the device with the specified function. Power-up is complete when the CTS bit is set. This command may take longer to set the CTS bit than other commands.
*/
#define RF_POWER_UP 0x02, 0x01, 0x00, 0x01, 0xC9, 0xC3, 0x80

/*
// Set properties:           RF_INT_CTL_ENABLE_4
// Number of properties:     4
// Group ID:                 0x01
// Start ID:                 0x00
// Default values:           0x04, 0x00, 0x00, 0x04, 
// Descriptions:
//   INT_CTL_ENABLE - Enables top-level interrupt sources to generate HW interrupts at the NIRQ pin. The three interrupt groups are Chip, Modem and Packet Handler. Each of them contains multiple possible interrupt sources that must be individually enabled via the INT_CTL_PH_ENABLE, INT_CTL_MODEM_ENABLE, INT_CTL_CHIP_ENABLE properties. Note that this property only provides for global enabling/disabling of the HW interrupt indication on the NIRQ output pin. An internal interrupt event may still be generated even if the indication on the NIRQ output pin is disabled. The interrupt event may always be monitored by polling a GPIO pin, or via GET_INT_STATUS, GET_CHIP_STATUS, GET_PH_STATUS, or GET_MODEM_STATUS commands.
//   INT_CTL_PH_ENABLE - Enable individual interrupt sources within the Packet Handler Interrupt Group in order to generate a HW interrupt at the NIRQ output pin. In order to fully enable a HW interrupt, it is necessary to enable both the individual interrupt source (within this property) as well as the corresponding interrupt group (e.g., set INT_CTL_ENABLE:PH_INT_STATUS_EN). Note that even if an interrupt source is not enabled to generate a HW NIRQ interrupt, the given interrupt event still may occur inside the chip and may be monitored by polling a GPIO pin, or via the GET_INT_STATUS or GET_PH_STATUS commands.
//   INT_CTL_MODEM_ENABLE - Enable individual interrupt sources within the Modem Interrupt Group in order to generate a HW interrupt at the NIRQ output pin. In order to fully enable a HW interrupt, it is necessary to enable both the individual interrupt source (within this property) as well as the corresponding interrupt group (e.g., set INT_CTL_ENABLE:MODEM_INT_STATUS_EN ). Note that even if an interrupt source is not enabled to generate a HW NIRQ interrupt, the given interrupt event still may occur inside the chip and may be monitored by polling a GPIO pin, or via the GET_INT_STATUS or GET_MODEM_STATUS commands.
//   INT_CTL_CHIP_ENABLE - Enable individual interrupt sources within the Chip Interrupt Group in order to generate a HW interrupt at the NIRQ output pin. In order to fully enable a HW interrupt, it is necessary to enable both the individual interrupt source (within this property) as well as the corresponding interrupt group (e.g., set INT_CTL_ENABLE:CHIP_INT_STATUS_EN). Note that even if an interrupt source is not enabled to generate a HW NIRQ interrupt, the given interrupt event still may occur inside the chip and may be monitored by polling a GPIO pin, or via the GET_INT_STATUS or GET_CHIP_STATUS commands.
*/
#define RF_INT_CTL_ENABLE_4 0x11, 0x01, 0x04, 0x00, 0x07, 0x19, 0x01, 0x08

/*
// Set properties:           RF_FRR_CTL_A_MODE_4
// Number of properties:     4
// Group ID:                 0x02
// Start ID:                 0x00
// Default values:           0x01, 0x02, 0x09, 0x00, 
// Descriptions:
//   FRR_CTL_A_MODE - Set the data that is present in fast response register A.
//   FRR_CTL_B_MODE - Set the data that is present in fast response register B.
//   FRR_CTL_C_MODE - Set the data that is present in fast response register C.
//   FRR_CTL_D_MODE - Set the data that is present in fast response register D.
*/
#define RF_FRR_CTL_A_MODE_4 0x11, 0x02, 0x04, 0x00, 0x01, 0x02, 0x09, 0x00

/*
// Set properties:           RF_EZCONFIG_XO_TUNE_1
// Number of properties:     1
// Group ID:                 0x24
// Start ID:                 0x03
// Default values:           0x40, 
// Descriptions:
//   EZCONFIG_XO_TUNE - Crystal oscillator frequency tuning value. 0x00 is maximum frequency value and 0x7F is lowest frequency value. Each LSB code corresponds to a 70 fF capacitance change. The total adjustment range assuming a 30 MHz XTAL is +/-100ppm.
*/
#define RF_EZCONFIG_XO_TUNE_1 0x11, 0x24, 0x01, 0x03, 0x52

/*
// Command:                  RF_WRITE_TX_FIFO
// Description:              This command does not cause CTS to go low, and can be sent and the reply read while CTS is low. This command has no response to be read. If you write more data than the TX FIFO can hold it will trigger a FIFO Overflow interrupt.
*/
#define RF_WRITE_TX_FIFO 0x66, 0xC6, 0x90, 0xEA, 0x8C, 0xDB, 0xCE, 0x78, 0xB7, 0x56, 0x48, 0x6B, 0xD2, 0x46, 0xC1, 0xC0, 0xA8, 0x8E, 0xD1, 0xE3, \
0x30, 0x0D, 0x52, 0xA7, 0x52, 0xE2, 0x33, 0x43, 0x89, 0x42, 0x9F, 0xE2, 0x27, 0x62, 0x93, 0xD8, 0x10, 0x18, 0x18, 0x42, \
0xB0, 0xD5, 0xB7, 0x6D, 0xB6, 0xEE, 0x33, 0x34, 0xB5, 0xC1, 0x3A, 0xBC, 0x8E, 0xC3, 0x15, 0xFE, 0x5B, 0x37, 0x3C, 0x9D, \
0x84, 0xA8, 0xAC, 0x24, 0x38, 0x74, 0x7B, 0xD6, 0xBD, 0x7E, 0x85, 0x18, 0xE7, 0xA2, 0xB9, 0x70, 0x95, 0x58, 0x77, 0xC7, \
0x56, 0x67, 0xAB, 0x80, 0x77, 0x80, 0x3C, 0x25, 0xF6, 0x1E, 0x34, 0x1D, 0x7B, 0x3E, 0xF9, 0x3D, 0x20, 0xCA, 0x52, 0x04, \
0xE2, 0xCE, 0xE5, 0xBD, 0x1D, 0x18, 0xF1

/*
// Command:                  RF_NOP
// Description:              Can be used to ensure communication with the device.
*/
#define RF_NOP 0x00

/*
// Command:                  RF_WRITE_TX_FIFO_1
// Description:              This command does not cause CTS to go low, and can be sent and the reply read while CTS is low. This command has no response to be read. If you write more data than the TX FIFO can hold it will trigger a FIFO Overflow interrupt.
*/
#define RF_WRITE_TX_FIFO_1 0x66, 0x23, 0x4E, 0x56, 0xFF, 0x4F, 0x6F, 0xB6, 0xC6, 0x71, 0x3D, 0x2C, 0xD1, 0x4E, 0xE2, 0x7A, 0x3A, 0x81, 0xD7, 0x7E, \
0xF9, 0x3D, 0x48, 0xF5, 0x94, 0x61, 0xFF, 0xBF, 0x64, 0xFA, 0x07, 0xD9, 0xF6, 0xAD, 0x5D, 0xF4, 0xE3, 0x41, 0x2D, 0xBE, \
0x7F, 0xBF, 0x66, 0xBE, 0x90, 0xC3, 0x5A, 0x5E, 0xD5, 0x13, 0x3F, 0x1F, 0xC1, 0xEC, 0x44, 0x3E, 0x2E, 0xE7, 0xF1, 0x8F, \
0xB6, 0xBD, 0x95, 0x04, 0x64, 0x6B, 0x8B, 0xB7, 0x85, 0x87, 0xA9, 0x10, 0x94, 0x33, 0x77, 0x66, 0x62, 0x3A, 0x7C, 0x6F, \
0x64, 0x12, 0x98, 0xC7, 0xC0, 0x3F, 0xAE, 0x9D, 0xA8, 0x95, 0x21, 0x4C, 0x66, 0xDB, 0x22, 0xEB, 0xB1, 0xFB, 0x04, 0xD1, \
0xA9, 0xFB, 0x4D, 0x25, 0x74, 0x8B

/*
// Command:                  RF_EZCONFIG_CHECK
// Description:              
*/
#define RF_EZCONFIG_CHECK 0x19, 0x16, 0x2C

/*
// Command:                  RF_GPIO_PIN_CFG
// Description:              Configures the gpio pins.
*/
#define RF_GPIO_PIN_CFG 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00


// AUTOMATICALLY GENERATED CODE! 
// DO NOT EDIT/MODIFY BELOW THIS LINE!
// --------------------------------------------

#ifndef FIRMWARE_LOAD_COMPILE
#define RADIO_CONFIGURATION_DATA_ARRAY { \
        0x07, RF_POWER_UP, \
        0x08, RF_INT_CTL_ENABLE_4, \
        0x08, RF_FRR_CTL_A_MODE_4, \
        0x05, RF_EZCONFIG_XO_TUNE_1, \
        0x6B, RF_WRITE_TX_FIFO, \
        0x01, RF_NOP, \
        0x6A, RF_WRITE_TX_FIFO_1, \
        0x03, RF_EZCONFIG_CHECK, \
        0x08, RF_GPIO_PIN_CFG, \
        0x00 \
 }
#else
#define RADIO_CONFIGURATION_DATA_ARRAY { 0 }
#endif

// DEFAULT VALUES FOR CONFIGURATION PARAMETERS
#define RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ_DEFAULT                     30000000L
#define RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER_DEFAULT                    0x00
#define RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH_DEFAULT               0x10
#define RADIO_CONFIGURATION_DATA_RADIO_STATE_AFTER_POWER_UP_DEFAULT        0x01
#define RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET_DEFAULT       0x1000
#define RADIO_CONFIGURATION_DATA_CUSTOM_PAYLOAD_DEFAULT  		   0x42, 0x55, 0x54, 0x54, 0x4F, 0x4E, 0x31 // BUTTON1 

#define RADIO_CONFIGURATION_DATA_RADIO_PATCH_INCLUDED                      0x00
#define RADIO_CONFIGURATION_DATA_RADIO_PATCH_SIZE                          0x00
#define RADIO_CONFIGURATION_DATA_RADIO_PATCH                               {  }

#ifndef RADIO_CONFIGURATION_DATA_ARRAY
#error "This property must be defined!"
#endif

#ifndef RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ
#define RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ         { RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ_DEFAULT }
#endif

#ifndef RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER
#define RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER        { RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER_DEFAULT }
#endif

#ifndef RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH
#define RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH   { RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH_DEFAULT }
#endif

#ifndef RADIO_CONFIGURATION_DATA_RADIO_STATE_AFTER_POWER_UP
#define RADIO_CONFIGURATION_DATA_RADIO_STATE_AFTER_POWER_UP  { RADIO_CONFIGURATION_DATA_RADIO_STATE_AFTER_POWER_UP_DEFAULT }
#endif

#ifndef RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET
#define RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET { RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET_DEFAULT }
#endif

#ifndef RADIO_CONFIGURATION_DATA_CUSTOM_LONG_PAYLOAD
#define RADIO_CONFIGURATION_DATA_CUSTOM_LONG_PAYLOAD        { RADIO_CONFIGURATION_DATA_CUSTOM_PAYLOAD_DEFAULT }
#endif

#define RADIO_CONFIGURATION_DATA { \
                            Radio_Configuration_Data_Array,                            \
                            RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER,                   \
                            RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH,              \
                            RADIO_CONFIGURATION_DATA_RADIO_STATE_AFTER_POWER_UP,       \
                            RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET,      \
                            Radio_Configuration_Data_Custom_Long_Payload_Array         \
                            }

#endif /* RADIO_CONFIG_H_ */
