# Uses atmega board definitions from
# https://github.com/carlosefr/atmega

ARDUINO_DIR = ~/bin/arduino-1.8.5/
AVR_DIR = $(ARDUINO_DIR)/hardware/arduino/avr
ARDUINO_CORE_DIR = $(AVR_DIR)/cores/arduino
ARDUINO_BOARD_DIR = $(HOME)/.arduino15/packages/boards/avr/
ARDUINO_VARIANT_DIR = $(AVR_DIR)/variants/eightanaloginputs/

CC=avr-gcc
CXX=avr-gcc
OBJCOPY=avr-objcopy

F_CPU = 16000000L
ARCH = atmega328p
CFLAGS = -x c++ -mmcu=$(ARCH) -DF_CPU=$(F_CPU) \
	 -Os -fno-exceptions -ffunction-sections -fdata-sections \
	 -Wl,--gc-sections -g -Wall -Wextra
CORE_SRC = \
	$(ARDUINO_CORE_DIR)/wiring.c \
	$(ARDUINO_CORE_DIR)/wiring_digital.c
LIBRARIES = \
	-I $(AVR_DIR)/libraries/SPI/src \
	-I $(AVR_DIR)/libraries/Wire/src \
	-I ZetaRF/

COMPILE = $(CC) $(CFLAGS) $(LIBRARIES) -I $(ARDUINO_CORE_DIR) -I $(ARDUINO_VARIANT_DIR) 

all: bsides2019.elf

clean:
	-rm -f bsides2019.elf

install: bsides2019 .elf
	avrdude -p $(ARCH) -c avrisp2 -U flash:w:$<

ZetaRF.o:
	$(COMPILE) -c -o $@ ZetaRF/ZetaRF.cpp

bsides2019.elf: bsides2019.ino ZetaRF.o $(CORE_SRC)
	$(COMPILE) -o $@ bsides2019.ino -x none ZetaRF.o

