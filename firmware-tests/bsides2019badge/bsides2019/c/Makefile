CC=avr-gcc
OBJCOPY=avr-objcopy
#FUSES=-U lfuse:w:0x96:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m -U lock:w:0xFF:m
FUSES=-U lfuse:w:0xD6:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m -U lock:w:0xFF:m 
CFLAGS=-Os -DF_CPU=16000000UL -mmcu=atmega328p

bsides2019.hex: bsides2019.elf
	${OBJCOPY} -O ihex -R .eeprom bsides2019.elf bsides2019.hex

bsides2019.elf: bsides2019.o
	${CC} ${CFLAGS} -o bsides2019.elf bsides2019.o

bsides2019.o: bsides2019.c 
	${CC} ${CFLAGS} -c -o bsides2019.o bsides2019.c

clean:
	rm *.elf *.hex *.o

flash: bsides2019.hex
	avrdude -c avrisp2 -p atmega328p -U flash:w:bsides2019.hex -B 10

fuses:
	avrdude -c avrisp2 -p atmega328p ${FUSES}
