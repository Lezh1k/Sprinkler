# tools
AVR_PREFIX = avr-
CC = $(AVR_PREFIX)gcc
OBJCOPY = $(AVR_PREFIX)objcopy
OBJDUMP = $(AVR_PREFIX)objdump
AS = $(AVR_PREFIX)as
LD = $(AVR_PREFIX)ld

# libraries and defines
LIBS := 
DEFS := -D__AVR_ATtiny2313__ \
    -D__SECONDS_TO_00__=$(shell ./seconds_to_00.sh)
   
# Directories
BUILD_DIR = build
OBJ_DIR=$(BUILD_DIR)/obj
SRC_DIR = src
BIN_DIR = bin

#device and program
PRG=valve
MCU=mcu=attiny2313
MMCU = -m$(MCU)
INCS := -Iinc

# flags
WARN_LEVEL = -Wall -Wextra -Wundef -fno-common # -Werror
CDIALECT = -std=c11
EXTRA_FLAGS = -ffunction-sections -fdata-sections -fstack-usage
CFLAGS := $(CDIALECT) $(INCS) $(MMCU) $(DEFS) $(WARN_LEVEL) $(EXTRA_FLAGS)
ASFLAGS = -f coff
LDFLAGS := -Wl,-Map,$(BIN_DIR)/$(PRG).map $(MMCU) -Wl,--start-group \
					 -Wl,--end-group -Wl,--gc-sections -Wl,-print-memory-usage 

# objects
SRC_C := $(wildcard $(SRC_DIR)/*.c)
SRC_A := $(wildcard $(SRC_DIR)/*.s)

OBJECTS := $(SRC_C:%.c=$(OBJ_DIR)/%.o)
OBJECTS += $(SRC_A:%.s=$(OBJ_DIR)/%.o)

#targets
.PHONY: debug
debug: CFLAGS += -O0 -g3
debug: DEFS += __VALVE_DEBUG__
debug: build

.PHONY: release
release: CFLAGS += -Os
release: build

.PHONY: all
all: debug

.PHONY: build
build: directories $(PRG)

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

.PHONY: directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)/*
	@rm -rf $(BIN_DIR)/*

.PHONY: mrproper
mrproper:
	@rm -rf $(BUILD_DIR) 
	@rm -rf $(BIN_DIR)

.PHONY: program
program:
	@avrdude -c usbasp -p t2313 -U flash:w:$(BIN_DIR)/$(PRG).hex

.PHONY: fuse_program
fuse_program:
	@avrdude -p t2313 -c usbasp -U lfuse:w:0x6f:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
