CC=avr-gcc
OBJCOPY=avr-objcopy
FUSES=-U lfuse:w:0xD6:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m -U lock:w:0xFF:m
PROGRAMMER=avrisp2
CFLAGS=-Os -DF_CPU=16000000UL -mmcu=atmega328p \
       -Iu8g2/csrc -Wall -Wextra -funsigned-char \
       -funsigned-bitfields -fpack-struct -fshort-enums \
       -ffunction-sections -fdata-sections -Wl,--gc-sections \
       -Wl,--relax -mcall-prologues -std=gnu99 -Wstrict-prototypes

# source files
SRC          := src
OBJ          := obj
U8G2_OBJ     := obj/u8g2
U8G2_SRC     := u8g2/csrc
SOURCES      := $(wildcard $(SRC)/*.c)
OBJECTS      := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))
U8G2_SOURCES := $(wildcard $(U8G2_SRC)/*.c)
U8G2_OBJECTS := $(patsubst $(U8G2_SRC)/%.c, $(U8G2_OBJ)/%.o, $(U8G2_SOURCES))

# default target

default: bsides2019.hex

# objects

$(OBJ)/%.o: $(SRC)/%.c
	${CC} ${CFLAGS} -c $< -o $@

$(U8G2_OBJ)/%.o: $(U8G2_SRC)/%.c
	${CC} ${CFLAGS} -c $< -o $@


# convert to intel hex for avrdude

bsides2019.hex: bsides2019.elf
	${OBJCOPY} -O ihex -R .eeprom bsides2019.elf bsides2019.hex

# link objects into elf binary

bsides2019.elf: $(U8G2_OBJECTS) $(OBJECTS)
	${CC} ${CFLAGS} -o bsides2019.elf \
	    ${U8G2_OBJECTS} \
	    ${OBJECTS}

# clean it all up for rebuild

clean:
	rm *.elf *.hex ${OBJECTS} ${U8G2_OBJECTS} 

# flash via an avrisp2 programmer

flash: bsides2019.hex
	avrdude -c ${PROGRAMMER} -p atmega328p -U flash:w:bsides2019.hex -B 10

# set the fuses for 16mhz external full-swing crystal (important!)

fuses:
	avrdude -c ${PROGRAMMER} -p atmega328p ${FUSES}
