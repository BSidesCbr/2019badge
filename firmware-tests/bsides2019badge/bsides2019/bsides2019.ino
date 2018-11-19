//#include <U8g2lib.h>

#include <SPI.h>
#include <Wire.h>
#include <ZetaRF.h>
#include <si4455_defs.h>

#define ZETARF_PACKET_LENGTH 8

// Uses normal SPI pins, adding CE (cs), DC (rw) and Reset
// In the prototype this was missing the OSC and VOUT hookup, fixed in v2 prototype
//U8G2_PCD8544_84X48_1_4W_HW_SPI u8g2(U8G2_R0, 15, 17, 8);

// Func | Si4455 Pin | Arduino | AVR Physical
//  CS  |    nSEL    |   D9    |   13 / PB1 
//  IRQ |    nIRQ    | A1/D15  |   24 / ADC1    * pin changed from v1 prototype (was originally 14/PB2/D10)
// GPIO |    SDN     | A0/D14  |   23 / ADC0    * this was missing in v1 prototype, fixed in v2 prototype

//ZetaRF Zeta(9, 14, 15);  // Pins: SPI CS, Shutdown, IRQ // new
ZetaRF Zeta(9, 14, 10);  // Pins: SPI CS, Shutdown, IRQ // old

bool transmitting = false;

void setup() {

  // Set up serial
  Serial.begin(115200);
  Serial.println(F("Badge setup started"));

  // Set up SPI
  SPI.begin();

  // Toggle reset on display
  Serial.println(F("Start display"));
  digitalWrite(8, LOW);
  digitalWrite(8, HIGH);
  // Set up and start display
  //u8g2.begin();
  //u8g2.setContrast(135);
  
  Serial.println(F("Start RF"));
  
  Zeta.begin(1, ZETARF_PACKET_LENGTH);
  
  Serial.println(F("RF information:"));
  
  const Si4455_PartInfo &pi = Zeta.readPartInfo();

  // on the rev1 board, the supplied part was Si4455-B1A
  Serial.println(F("----------"));
  Serial.print(F("Chip rev: ")); Serial.println(pi.CHIPREV, HEX);
  Serial.print(F("Part    : ")); Serial.println((uint32_t)(pi.PART.U16>>8) | (pi.PART.U16<<8), HEX);
  Serial.print(F("PBuild  : ")); Serial.println(pi.PBUILD, HEX);
  Serial.print(F("ID      : ")); Serial.println((uint32_t)(pi.ID.U16>>8) | (pi.ID.U16<<8), HEX);
  Serial.print(F("Customer: ")); Serial.println(pi.CUSTOMER, HEX);
  Serial.print(F("Rom ID  : ")); Serial.println(pi.ROMID, HEX);
  Serial.print(F("Bond    : ")); Serial.println(pi.BOND, HEX);
  Serial.print(F("\n"));

  const Si4455_FuncInfo &fi = Zeta.readFuncInfo();
  Serial.print(F("Rev Ext   : ")); Serial.println(fi.REVEXT, HEX);
  Serial.print(F("Rev Branch: ")); Serial.println(fi.REVBRANCH, HEX);
  Serial.print(F("Rev Int   : ")); Serial.println(fi.REVINT, HEX);
  Serial.print(F("Patch     : ")); Serial.println((uint32_t)(fi.PATCH.U16>>8) | (fi.PATCH.U16<<8), HEX);
  Serial.print(F("Func      : ")); Serial.println(fi.FUNC, HEX);
  Serial.print(F("SVN Flags : ")); Serial.println(fi.SVNFLAGS, HEX);
  Serial.print(F("SVN Rev   : ")); Serial.println((uint32_t)fi.SVNREV.U32, HEX);
  Serial.println(F("----------"));

  Serial.println(F("Start listening for RF"));
  Zeta.startListening();

  Serial.println(F("Setup done"));
  
}

void loop() {

  char data[ZETARF_PACKET_LENGTH];
  char txdata[ZETARF_PACKET_LENGTH] = "FEEBFEEB";

  Serial.println(F("Sending dummy packet"));
  Zeta.sendPacket(txdata);
  delay(1000); 

  Serial.println(F("Checking for RF packets"));
  if (Zeta.checkReceived()) {
    Serial.print(F("> "));
    Zeta.readPacket((uint8_t*)data);
    Serial.write(data, ZETARF_PACKET_LENGTH);
    Serial.print(F("\n"));
  }
  delay(1000); 
  
  const Si4455_ModemStatus &ms = Zeta.readModemStatus(0);
  Serial.println(F("Modem status"));
  Serial.print(F("Pending   : ")); Serial.println(ms.MODEM_PEND);
  Serial.print(F("Status    : ")); Serial.println(ms.MODEM_STATUS);
  Serial.print(F("RSSI      : ")); Serial.println(ms.CURR_RSSI);
  Serial.print(F("Latch RSSI: ")); Serial.println(ms.LATCH_RSSI);
  Serial.print(F("ANT1 RSSI : ")); Serial.println(ms.ANT1_RSSI);
  Serial.print(F("ANT2 RSSI : ")); Serial.println(ms.ANT2_RSSI);
  Serial.println(F("----------"));
  delay(1000); 
  
  const Si4455_DeviceState &ds = Zeta.requestDeviceState();
  Serial.println(F("Device State"));
  Serial.print(F("State  : ")); Serial.println(ds.CURR_STATE);
  Serial.print(F("Channel: ")); Serial.println(ds.CURRENT_CHANNEL);
  Serial.println(F("----------"));
  delay(1000); 
  
  const Si4455_PhStatus &ps = Zeta.readPhStatus(0);
  Serial.println(F("Phy Status"));
  Serial.print(F("Pending: ")); Serial.println(ps.PH_PEND);
  Serial.print(F("Status : ")); Serial.println(ps.PH_STATUS, HEX);
  Serial.println(F("----------"));
  delay(1000); 
  
  const Si4455_ChipStatus &cs = Zeta.readChipStatus(0);
  Serial.println(F("Chip Status"));
  Serial.print(F("Pending   : ")); Serial.println(cs.CHIP_PEND);
  Serial.print(F("Status    : ")); Serial.println(cs.CHIP_STATUS);
  Serial.print(F("Err Status: ")); Serial.println(cs.CMD_ERR_STATUS);
  Serial.println(F("----------"));
  delay(1000); 
  
  const Si4455_InterruptStatus &is = Zeta.readInterruptStatus(0,0,0);
  Serial.println(F("Interrupt Status"));
  Serial.print(F("Int Pending   : ")); Serial.println(is.INT_PEND);
  Serial.print(F("Int Status    : ")); Serial.println(is.INT_STATUS);
  Serial.print(F("Phy Pending   : ")); Serial.println(is.PH_PEND);
  Serial.print(F("Phy Status    : ")); Serial.println(is.PH_STATUS, HEX);
  Serial.print(F("Modem Pending : ")); Serial.println(is.MODEM_PEND);
  Serial.print(F("Modem Status  : ")); Serial.println(is.MODEM_STATUS);
  Serial.print(F("Chip Pending  : ")); Serial.println(is.CHIP_PEND);
  Serial.print(F("Chip Status   : ")); Serial.println(is.CHIP_STATUS);
  Serial.println(F("----------"));
  delay(1000); 

  //Serial.println("Updating screen");
  //u8g2.firstPage();
  //do {
  //  u8g2.setFont(u8g2_font_profont11_tr);
  //  u8g2.drawStr(0, 15, "BSides 2019");  // write a string at position X, Y
  //  u8g2.drawStr(0, 25, "Hey Kylie!");
  //  u8g2.drawStr(0, 35, "Hey Silvio!");
  //} while ( u8g2.nextPage() );
  //PORTD ^= 0xFF;
  delay(1000); 
}
