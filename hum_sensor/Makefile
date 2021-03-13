# tools
AVR_PREFIX = avr-

CC = $(AVR_PREFIX)gcc
OBJCOPY = $(AVR_PREFIX)objcopy
OBJDUMP = $(AVR_PREFIX)objdump
AR = $(AVR_PREFIX)ar
NM = $(AVR_PREFIX)nm
AS = $(AVR_PREFIX)as
LD = $(AVR_PREFIX)ld
SIZE = $(AVR_PREFIX)size

# libraries and defines
LIBS = 
DEFS = -DNDEBUG -D__AVR_ATtiny85__ 

# Directories
BUILD_DIR = build
OBJ_DIR=$(BUILD_DIR)/obj
SRC_DIR = src
BIN_DIR = bin

#device and program
PRG = hum-sensor
MCU=mcu=attiny85
MMCU = -m$(MCU)
INCS := -Iinc

# flags
CFLAGS := $(INCS) $(MMCU) $(DEFS) -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -std=gnu99 -Wall 
ASFLAGS = -f coff
LDFLAGS = -Wl,-Map,$(BIN_DIR)/$(PRG).map $(MMCU) -Wl,--start-group  -Wl,--end-group -Wl,--gc-sections -Wl,-print-memory-usage

# objects
SRC_C := $(wildcard $(SRC_DIR)/*.c)
SRC_A := $(wildcard $(SRC_DIR)/*.s)

OBJECTS := $(SRC_C:%.c=$(OBJ_DIR)/%.o)
OBJECTS += $(SRC_A:%.s=$(OBJ_DIR)/%.o)

#targets

debug: CFLAGS += -O0 -g3
debug: all
release: CFLAGS += -O2
release: all

all: directories $(PRG) 
$(PRG): $(BIN_DIR)/$(PRG).elf $(BIN_DIR)/lst $(BIN_DIR)/text 

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

$(BIN_DIR)/$(PRG).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BIN_DIR)/lst: $(BIN_DIR)/$(PRG).lst
$(BIN_DIR)/%.lst: $(BIN_DIR)/%.elf 
	$(OBJDUMP) -h -S $< > $@

$(BIN_DIR)/text: $(BIN_DIR)/hex $(BIN_DIR)/bin

$(BIN_DIR)/hex: $(BIN_DIR)/$(PRG).hex
$(BIN_DIR)/%.hex: $(BIN_DIR)/%.elf
	$(OBJCOPY) -O ihex $< $@

$(BIN_DIR)/bin: $(BIN_DIR)/$(PRG).bin
$(BIN_DIR)/%.bin: $(BIN_DIR)/%.elf
	$(OBJCOPY) -O binary $< $@

directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

clean:
	@rm -rf $(BUILD_DIR)/*
	@rm -rf $(BIN_DIR)/*

mrproper:
	@rm -rf $(BUILD_DIR) 
	@rm -rf $(BIN_DIR)

program:
	@avrdude -c usbasp -p t85 -U flash:w:$(BIN_DIR)/$(PRG).hex

fuse_program:
	@avrdude -c usbasp -p t85 -U lfuse:w:0xc6:m -U hfuse:w:0xdf:m -U efuse:w:0xfe:m  
