MCU = attiny13
F_CPU = 9600000UL
CC = avr-gcc
OBJ_DUMP = avr-objdump
OBJCOPY = avr-objcopy
IDIR = ~/Desktop/Avr/avr/include
PRJ_DIR = ${shell pwd}
USR_LIB_DIR = ${PRJ_DIR}/include

CFLAGS = -std=c99 -Wall -g -Os -mmcu=${MCU} -DF_CPU=${F_CPU} -I. -I${IDIR} -I${USR_LIB_DIR}

TARGET = main
SRCS = main.c $(wildcard include/*.c)
LINUX_PORT = /dev/ttyACM0

# All compilation and flashing process
all: ${TARGET}.hex ${TARGET}.elf
	@echo "Build Complete!"

# Compile and generate hex file
${TARGET}.hex: ${SRCS}
	${CC} ${CFLAGS} -o ${TARGET}.bin ${SRCS}
	${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.bin ${TARGET}.hex

# Generate the ELF file
${TARGET}.elf: ${SRCS}
	${CC} ${CFLAGS} -o ${TARGET}.elf ${SRCS}

# Dump the ELF file to disassembly listing
dump: ${TARGET}.elf
	${OBJ_DUMP} -h -S ${TARGET}.elf > ${TARGET}.lst
	@echo "Disassembly written to ${TARGET}.lst"

# Flash the hex file to the MCU
flash: ${TARGET}.hex
	avrdude -c usbasp -p $(MCU) -U flash:w:$(TARGET).hex:i

# Clean the build files
clean:
	rm -f *.bin *.hex *.elf *.o *.lst
