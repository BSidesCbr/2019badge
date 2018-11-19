/*! @file main.c
 * @brief The main.c file of the Fixed packet length Custom RX demo
 * for Si4455 devices.
 *
 * Contains the initialization of the MCU & the radio.
 * @n The main loop controls the program flow & the radio.
 *
 * @b CREATED
 * @n Silicon Laboratories Ltd.
 *
 * @b COPYRIGHT
 * @n Silicon Laboratories Confidential
 * @n Copyright 2012 Silicon Laboratories, Inc.
 * @n http://www.silabs.com
 *
 */

#include "..\bsp.h"


/*------------------------------------------------------------------------*/
/*                          Global variables                              */
/*------------------------------------------------------------------------*/
SEGMENT_VARIABLE(bMain_IT_Status, U8, SEG_XDATA);
SEGMENT_VARIABLE(lPer_MsCnt, U16, SEG_DATA);

/*------------------------------------------------------------------------*/
/*                              Defines                                   */
/*------------------------------------------------------------------------*/
#define PACKET_SEND_INTERVAL 2000u

/*------------------------------------------------------------------------*/
/*                             Enumeration                                */
/*------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*/
/*                             Typedefs                                   */
/*------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*/
/*                          Local variables                               */
/*------------------------------------------------------------------------*/
SEGMENT_VARIABLE(rollingCounter, U16, SEG_XDATA);
/*------------------------------------------------------------------------*/
/*                      Local function prototypes                         */
/*------------------------------------------------------------------------*/
void vPlf_McuInit        (void);
void vInitializeHW       (void);
void DemoApp_Pollhandler (void);

// Compare the desired custom payload with the incoming payload if variable field is not used
BIT  gSampleCode_StringCompare(U8* pbiPacketContent, U8* pbiString, U8 lenght);

// Waiting for a short period of time
void vSample_Code_Wait(U16 wWaitTime);

// Send "ACK" message  
void vSampleCode_SendAcknowledge(void);

// Send the custom message
BIT  gSampleCode_SendVariablePacket(void);

/*------------------------------------------------------------------------*/
/*                          Function implementations                      */
/*------------------------------------------------------------------------*/

/** \fn void main(void)
 * \brief The main function of the demo.
 *
 * \todo Create description
 */
void main(void)
{
  // Initialize the Hardware and Radio
  vInitializeHW();

#ifdef SILABS_LCD_DOG_GLCD
  /* Initialize graphic LCD */
  vLcd_InitLcd();

  /* Set logo location to center */
  bLcd_LcdSetPictureCursor(bLcd_Line1_c, 35u);

  /* Draw SiLabs logo */
  vLcd_LcdDrawPicture(silabs66x30);
#endif
  // Start RX with Packet handler settings
  vRadio_StartRX(pRadioConfiguration->Radio_ChannelNumber,0u);

  while (TRUE)
  {
    // The following Handlers requires care on invoking time interval
    if (wIsr_Timer2Tick)
    {
      if (lPer_MsCnt < 0xFFFF)
      {
        lPer_MsCnt++;
      }

      vHmi_LedHandler();
      vHmi_BuzzHandler();
      vHmi_PbHandler();

      wIsr_Timer2Tick = 0;
    }

    // Demo Application Poll-Handler function
    DemoApp_Pollhandler();
  }
}

/**
 *  Demo Application Poll-Handler
 *
 *  @note This function must be called periodically.
 *
 */
void DemoApp_Pollhandler()
{
  static SEGMENT_VARIABLE(lPktSending, U8, SEG_XDATA) = 0u;

  bMain_IT_Status = bRadio_Check_Tx_RX();

  switch (bMain_IT_Status)
  {
    case SI4455_CMD_GET_INT_STATUS_REP_PACKET_SENT_PEND_BIT:

      if(gSampleCode_StringCompare(&customRadioPacket[1],"ACK",3) == TRUE)
	  {
        // Message "ACK" sent successfully
        vHmi_ChangeLedState(eHmi_Led4_c, eHmi_LedBlinkOnce_c);

        // Start RX with radio packet length
		vRadio_StartRX(pRadioConfiguration->Radio_ChannelNumber , 0u);
      }
      else
      {
        // Custom message sent successfully
        vHmi_ChangeLedState(eHmi_Led1_c, eHmi_LedBlinkOnce_c);

        // Start RX waiting for "ACK" message
		vRadio_StartRX(pRadioConfiguration->Radio_ChannelNumber, 0u);
      }
		
	  /* Clear Packet Sending flag */
      lPktSending = 0u;

      break;

    case SI4455_CMD_GET_INT_STATUS_REP_PACKET_RX_PEND_BIT:

	  if(gSampleCode_StringCompare(&customRadioPacket[1],"ACK",3) == TRUE)
	  {
        // Start RX with radio packet length
		vRadio_StartRX(pRadioConfiguration->Radio_ChannelNumber, 0u);

        // Buzz once 
        vHmi_ChangeBuzzState(eHmi_BuzzOnce_c);

        // "ACK" arrived successfully
        vHmi_ChangeLedState(eHmi_Led1_c, eHmi_LedBlinkOnce_c);
        vHmi_ChangeLedState(eHmi_Led2_c, eHmi_LedBlinkOnce_c);
        vHmi_ChangeLedState(eHmi_Led3_c, eHmi_LedBlinkOnce_c);
        vHmi_ChangeLedState(eHmi_Led4_c, eHmi_LedBlinkOnce_c);

        break;
      }
      else
      {
        if (gSampleCode_StringCompare((U8 *) &customRadioPacket[0u], pRadioConfiguration->Radio_CustomPayload,
            pRadioConfiguration->Radio_PacketLength) == TRUE )
        {
          // Blink once LED1 as payload match
          vHmi_ChangeLedState(eHmi_Led1_c, eHmi_LedBlinkOnce_c);

          // Send ACK back
          vSampleCode_SendAcknowledge();
         }
		 else
		 {
		 	// Discard the packet and turn on the receiver.
	        vRadio_StartRX(pRadioConfiguration->Radio_ChannelNumber, 0u);
		 }

      }
      break;

    default:
      break;
  } /* switch */

  if ((lPer_MsCnt >= PACKET_SEND_INTERVAL) && (0u == lPktSending))
  {
    if (TRUE == gSampleCode_SendVariablePacket())
    {
      /* Reset packet send counter */
      lPer_MsCnt = 0u;

	  /* Set Packet Sending flag */
      lPktSending = 1u;
    }
  }
}

/*!
 * This function is used to compare the content of the received packet to a string.
 *
 * @return  None.
 */
BIT gSampleCode_StringCompare(U8* pbiPacketContent, U8* pbiString, U8 lenght)
{
  while ((*pbiPacketContent++ == *pbiString++) && (lenght > 0u))
  {
    if( (--lenght) == 0u )
    {
      return TRUE;
    }
  }

  return FALSE;
}

/*!
 * This function is used to wait for a little time.
 *
 * @return  None.
 */
void vSample_Code_Wait(U16 wWaitTime)
{
  SEGMENT_VARIABLE(wDelay, U16 , SEG_DATA) = wWaitTime;

  for (; wDelay--; )
  {
    NOP();
  }
}

/*!
 * This function is used to send "ACK"back to the sender.
 *
 * @return  None.
 */
void vSampleCode_SendAcknowledge(void)
{

  // Payload	
  customRadioPacket[0u] = 0x03;
  customRadioPacket[1u] = 'A';
  customRadioPacket[2u] = 'C';
  customRadioPacket[3u] = 'K';
	
  vRadio_StartTx_Variable_Packet(pRadioConfiguration->Radio_ChannelNumber, &customRadioPacket[0], 4u);
}


/*!
 * This function is used to send the custom packet.
 *
 * @return  None.
 */
BIT gSampleCode_SendVariablePacket(void)
{
  SEGMENT_VARIABLE(boPbPushTrack,  U8, SEG_DATA);
  SEGMENT_VARIABLE(lTemp,         U16, SEG_DATA);
  SEGMENT_VARIABLE(pos,            U8, SEG_DATA);

  gHmi_PbIsPushed(&boPbPushTrack, &lTemp);
  
  if( boPbPushTrack & eHmi_Pb1_c)
  { 
  	for(pos = 0u; pos < pRadioConfiguration->Radio_PacketLength; pos++)
  	{
    	customRadioPacket[pos] = pRadioConfiguration->Radio_CustomPayload[pos];
  	}

   	vRadio_StartTx_Variable_Packet(pRadioConfiguration->Radio_ChannelNumber, &customRadioPacket[0], pRadioConfiguration->Radio_PacketLength);
   
   	/* Packet sending initialized */
   	return TRUE;
   }

   return FALSE;
}

/**
 *  Calls the init functions for the used peripherals/modules
 *
 *  @note Should be called at the beginning of the main().
 *
 */
void vInitializeHW()
{
  // Initialize the MCU peripherals
  vPlf_McuInit();

  // Initialize IO ports
  vCio_InitIO();

  // Start Timer2 peripheral with overflow interrupt
  vTmr_StartTmr2(eTmr_SysClkDiv12_c, wwTmr_Tmr2Periode.U16, TRUE, bTmr_TxXCLK_00_c);

  // Start the push button handler
  vHmi_InitPbHandler();

  // Start the Led handler
  vHmi_InitLedHandler();

  // Start the Buzzer Handler
  vHmi_InitBuzzer();

  // Initialize the Radio
  vRadio_Init();

  // Enable configured interrupts
  mIsr_EnableAllIt();
}

/** \fn void MCU_Init(void)
 *  \brief Initializes ports of the MCU.
 *
 *  \return None
 *
 *  \note It has to be called from the Initialization section.
 *  \todo Create description
 */
void vPlf_McuInit(void)
{
  U16 wDelay = 0xFFFF;

  /* disable watchdog */
  PCA0MD &= (~M_WDTE);

  /* Init Internal Precision Oscillator (24.5MHz) */
  SFRPAGE = LEGACY_PAGE;
  FLSCL   = M_BYPASS;

  OSCICN |= M_IOSCEN; // p7: Internal Prec. Osc. enabled
  CLKSEL  = 0x00;     // Int. Prec. Osc. selected (24.5MHz)

#if ((defined SILABS_PLATFORM_RFSTICK) || (defined SILABS_PLATFORM_LCDBB))
  P0MDOUT = M_P0_UART_TX; //PBs: P0.0-P0.3 (same as RF_GPIO0-3) used as input
  P1MDOUT = M_P1_SPI1_SCK | M_P1_SPI1_MOSI | M_P1_RF_NSEL | M_P1_RF_PWRDN;
  P2MDOUT = M_P2_LED1 | M_P2_LED2 | M_P2_LED3 | M_P2_LED4 | M_P2_BZ1;
#if (defined SILABS_PLATFORM_LCDBB)
  P1MDOUT |= M_P1_LCD_NSEL | M_P1_LCD_A0;
#endif

  P0SKIP  = (~M_P0_UART_TX) & (~M_P0_UART_RX) & (~M_P0_I2C_SCL) & (~M_P0_I2C_SDA); //skip all on port, but UART & SMBus
  P1SKIP  = (~M_P1_SPI1_SCK) & (~M_P1_SPI1_MISO) & (~M_P1_SPI1_MOSI); //skip all on port, but SPI1
  P2SKIP  = (~M_P2_BZ1); //skip all on port, but buzzer with PCA CEX0
#elif ((defined SILABS_MCU_DC_EMIF_F930) || (SILABS_MCU_DC_EMIF_F930_STANDALONE))

#if (defined SILABS_MCU_DC_EMIF_F930)
  // Init master hw SPI interface (SCK clock: 2.45MHz)
  // Init SPI0 (LCD)
  SPI0CFG = M_MSTEN0; //p6: SPI0 enable master mode
  SPI0CN  = M_SPI0EN; //p1: SPI0 enable
  SPI0CKR = 0x04;     //fSCK = SYSCLK / 10
#endif

  P0MDOUT = M_P0_UART_TX | M_P0_LED1 | M_P0_LED2 | M_P0_LED3 | M_P0_LED4;
  P1MDOUT = M_P1_SPI1_SCK | M_P1_SPI1_MOSI | M_P1_RF_NSEL;
  P2MDOUT = M_P2_RF_PWRDN;
#if (defined SILABS_MCU_DC_EMIF_F930)
  P1MDOUT |= M_P1_SPI0_SCK | M_P1_SPI0_MOSI;
  P2MDOUT |=  M_P2_LCD_NSEL | M_P2_LCD_A0;
#endif

  P0SKIP  = (~M_P0_UART_TX) & (~M_P0_UART_RX) ; //skip all on port, but UART
  P1SKIP  = (~M_P1_SPI1_SCK) & (~M_P1_SPI1_MISO) & (~M_P1_SPI1_MOSI); //skip all on port, but SPI1
  P2SKIP  = (~M_P2_I2C_SCL) & (~M_P2_I2C_SDA); //skip all on port, but SMBus
#if (defined SILABS_MCU_DC_EMIF_F930)
  P1SKIP  &= (~M_P1_SPI0_SCK) & (~M_P1_SPI0_MISO) & (~M_P1_SPI0_MOSI); //do not skip SPI0 for LCD
#endif

#elif (defined SILABS_PLATFORM_WMB930)
  /* Port IN/OUT init */
  P0MDOUT = 0x80;
  P1MDOUT = 0xF5;
  P2MDOUT = 0x49;

  P0SKIP  = 0xCF;
  P1SKIP  = 0x18;
  P2SKIP  = 0xB9;
#elif (defined SILABS_PLATFORM_WMB912)
  /* Port IN/OUT init */
  /* P0: 2,3,4,6,7 push-pull */
  /* P1: 0,2,3,6 push-pull */
  /* P2: no push-pull */
  P0MDOUT   = 0xDC;
  P1MDOUT   = 0x4D;

  /* P0: 0,1,2,3,6,7 skipped */
  /* P1: 3,6 skipped */
  /* P2: 7 skipped */
  P0SKIP    = 0xCF;
  P1SKIP    = 0x48;

  /* Set SMBUS clock speed */
  Set115200bps_24MHZ5;
  /* Start Timer1 */
  TR1 = 1;
  /* Initialize SMBus */
  vSmbus_InitSMBusInterface();
#else
#error TO BE WRITTEN FOR OTHER PLARFORMS!
#endif

  P0MDIN  = 0xFF; // All pin configured as digital port
  P1MDIN  = 0xFF; // All pin configured as digital port
#if !(defined SILABS_PLATFORM_WMB912)
  P2MDIN  = 0xFF; // All pin configured as digital port
#endif

  /* Set Drive Strenght */
  SFRPAGE = CONFIG_PAGE;
  P0DRV   = 0x00;
  P1DRV   = 0x00;
#if !(defined SILABS_PLATFORM_WMB912)
  P2DRV   = 0x00;
#endif

  SFRPAGE = LEGACY_PAGE;

  /* Crossbar configuration */
  XBR0    = M_URT0E | M_SMB0E; //p0: UART enabled on XBAR
  XBR1    = M_SPI1E ; //p6: SPI1 enabled on XBAR
#if ((defined SILABS_PLATFORM_RFSTICK) || (defined SILABS_PLATFORM_LCDBB))
  XBR1    |= (1 << BF_PCA0ME_0); //p0: PCA CEX0 enabled on XBAR
#elif(defined SILABS_MCU_DC_EMIF_F930)
  XBR0    |= M_SPI0E ; //p6: SPI1 enabled on XBAR
#elif (defined SILABS_MCU_DC_EMIF_F930_STANDALONE)

#elif (defined SILABS_PLATFORM_WMB930)
  XBR1    |= (1 << BF_PCA0ME_0); //p0: PCA CEX0 enabled on XBAR
  XBR0    |= M_SPI0E;
#elif (defined SILABS_PLATFORM_WMB912)

#else
#error TO BE WRITTEN FOR OTHER PLARFORMS!
#endif
  XBR2    = M_XBARE; //p6: XBAR enable

  /* latch all inputs to '1' */
  P0      = ~P0MDOUT;
  P1      = ~P1MDOUT;
#if !(defined SILABS_PLATFORM_WMB912)
  P2      = ~P2MDOUT;
#endif

  /* set all output to its default state */
  LED1      = EXTINGUISH;
#if !(defined SILABS_PLATFORM_WMB912)
  LED2      = EXTINGUISH;
  LED3      = EXTINGUISH;
  LED4      = EXTINGUISH;
#endif
  RF_NSEL   = TRUE;
  RF_PWRDN  = FALSE;

  /* SPI1 & SPI0 Config & Enable */
  SPI0CFG   = 0x40;
  SPI1CFG   = 0x40;
#if !(defined SILABS_PLATFORM_WMB912)
  SPI0CN    = 0x01;
#else
  SPI0CN    = 0x00;
#endif
  SPI1CN    = 0x01;
  SPI0CKR   = 0x0B;
  SPI1CKR   = 0x0B;

  /* De-select radio SPI */
  vSpi_SetNsel(eSpi_Nsel_RF);

#if ((defined SILABS_LCD_DOG_GLCD) || (defined SILABS_MCU_DC_EMIF_F930) || (defined SILABS_PLATFORM_WMB))
  /* De-select LCD SPI */
  vSpi_SetNsel(eSpi_Nsel_LCD);

  LCD_A0    = FALSE;
#endif

  /* Startup delay */
  for (; wDelay; wDelay--)  ;

}

#ifdef SDCC
/**
 * \brief External startup function of SDCC.
 *
 * It performs operations
 * prior static and global variable initialization.
 * Watchdog timer should be disabled this way, otherwise it
 * can expire before variable initialization is carried out,
 * and may prevent program execution jumping into main().
 *
 * \param None
 * \return None
 */
void _sdcc_external_startup(void)
{
  PCA0MD &= ~0x40;      // Disable Watchdog timer
}
#endif
