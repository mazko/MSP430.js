OBJECTS=blink.o ssd1306.o images.o

GCC_DIR =  ~/ti/msp430_gcc/bin
SUPPORT_FILE_DIRECTORY = ~/ti/msp430_gcc/include

DEVICE  = msp430f1611
CC      = $(GCC_DIR)/msp430-elf-gcc

CFLAGS = -I $(SUPPORT_FILE_DIRECTORY) -mmcu=$(DEVICE) -O2
LFLAGS = -L $(SUPPORT_FILE_DIRECTORY)

all: ${OBJECTS}
	$(CC) $(CFLAGS) $(LFLAGS) $? -o $(DEVICE).out
