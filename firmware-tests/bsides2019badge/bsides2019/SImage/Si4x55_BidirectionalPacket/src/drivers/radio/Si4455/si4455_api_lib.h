/*!
 * File:
 *  si4455_api_lib.h
 *
 * Description:
 *  This file contains the Si4455 API library.
 *
 * Silicon Laboratories Confidential
 * Copyright 2011 Silicon Laboratories, Inc.
 */

#ifndef _SI4455_API_LIB_H_
#define _SI4455_API_LIB_H_

extern SEGMENT_VARIABLE( Si4455Cmd, union si4455_cmd_reply_union, SEG_XDATA );
extern SEGMENT_VARIABLE( radioCmd[16], U8, SEG_XDATA );


#define SI4455_FIFO_SIZE 64

enum
{
    SI4455_SUCCESS,
    SI4455_NO_PATCH,
    SI4455_CTS_TIMEOUT,
    SI4455_PATCH_FAIL,
    SI4455_COMMAND_ERROR
};

/* Minimal driver support functions */
void si4455_reset(void);
void si4455_power_up(U8 BOOT_OPTIONS, U8 XTAL_OPTIONS, U32 XO_FREQ);

U8 si4455_configuration_init(const U8* pSetPropCmd);

void si4455_write_ezconfig_array(U8 numBytes, U8* pEzConfigArray);
void si4455_ezconfig_check(U16 CHECKSUM);

void si4455_start_tx(U8 CHANNEL, U8 CONDITION, U16 TX_LEN);
void si4455_start_rx(U8 CHANNEL, U8 CONDITION, U16 RX_LEN, U8 NEXT_STATE1, U8 NEXT_STATE2, U8 NEXT_STATE3);

void si4455_get_int_status(U8 PH_CLR_PEND, U8 MODEM_CLR_PEND, U8 CHIP_CLR_PEND);

void si4455_gpio_pin_cfg(U8 GPIO0, U8 GPIO1, U8 GPIO2, U8 GPIO3, U8 NIRQ, U8 SDO, U8 GEN_CONFIG);

void si4455_set_property( U8 GROUP, U8 NUM_PROPS, U8 START_PROP, ... );

void si4455_change_state(U8 NEXT_STATE1);

#ifdef RADIO_DRIVER_EXTENDED_SUPPORT
  /* Extended driver support functions */
  void si4455_nop(void);

  void si4455_fifo_info(U8 FIFO);
  void si4455_part_info(void);

  void si4455_write_tx_fifo( U8 numBytes, U8* pData );
  void si4455_read_rx_fifo( U8 numBytes, U8* pRxData );

  void si4455_get_property(U8 GROUP, U8 NUM_PROPS, U8 START_PROP);

  #ifdef RADIO_DRIVER_FULL_SUPPORT
    /* Full driver support functions */
    void si4455_func_info(void);

    void si4455_frr_a_read(U8 respByteCount);
    void si4455_frr_b_read(U8 respByteCount);
    void si4455_frr_c_read(U8 respByteCount);
    void si4455_frr_d_read(U8 respByteCount);

    void si4455_read_cmd_buff(void);
    void si4455_request_device_state(void);

    void si4455_get_adc_reading( U8 ADC_EN, U8 ADC_CFG );

    void si4455_get_ph_status(U8 PH_CLR_PEND);
    void si4455_get_modem_status( U8 MODEM_CLR_PEND );
    void si4455_get_chip_status( U8 CHIP_CLR_PEND );

  #endif
#endif

#endif //_SI4455_API_LIB_H_
