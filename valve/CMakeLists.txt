cmake_minimum_required(VERSION 3.16)

# set up for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set up toolchain
set(AVRCPP   avr-g++)
set(AVRC     avr-gcc)
set(AVRSTRIP avr-strip)
set(OBJCOPY  avr-objcopy)
set(OBJDUMP  avr-objdump)
set(AVRSIZE  avr-size)
set(AVRDUDE  avrdude)

set(CMAKE_SYSTEM_NAME     Generic)
set(CMAKE_CXX_COMPILER    ${AVRCPP})
set(CMAKE_C_COMPILER      ${AVRC})
set(CMAKE_ASM_COMPILER    ${AVRC})

set(PROJECT_NAME valve)
project(${PROJECT_NAME}) # this sets the project name

# These instructions search the directory tree when cmake is
# invoked and put all files that match the pattern in the variables 
# `sources` and `data`.
file(GLOB_RECURSE sources src/*.c inc/*.h)
file(GLOB_RECURSE data resources/*)
# You can use set(sources src/main.cpp) etc if you don't want to
# use globbing to find files automatically.

set(MCU attiny2313)

# Compiler flags
set(CSTANDARD "-std=c11")
set(CDEBUG    "-g3 -ggdb")
set(CWARN     "-Wall -Wextra -Wundef -Wstrict-prototypes -Wl,--gc-sections -Wl,--relax")
set(CTUNING   "-ffunction-sections -fdata-sections -fstack-usage")
set(COPT      "-O0")
set(CMCU      "-mmcu=${MCU}")

execute_process(COMMAND "./resources/seconds_to_00.sh" 
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} 
  OUTPUT_VARIABLE SECS_TO_00
)

set(CDEFS     "-D__AVR_ATtiny2313__ -D__SECONDS_TO_00__=${SECS_TO_00}")
message(${CDEFS})

set(CFLAGS   "${CMCU} ${CDEBUG} ${CDEFS} ${COPT} ${CWARN} ${CSTANDARD} ${CTUNING}")
set(ASMFLAGS "ASFLAGS = -f coff")

set(CMAKE_C_FLAGS     "${CFLAGS}")
set(CMAKE_ASM_FLAGS   "${ASMFLAGS}")

add_executable(valve ${sources} ${data})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.elf")
target_include_directories(valve PRIVATE inc)

file(COPY ${data} DESTINATION resources)
# Custom targets
add_custom_target(strip ALL ${AVRSTRIP} "${PROJECT_NAME}.elf" DEPENDS ${PROJECT_NAME})
add_custom_target(hex   ALL ${OBJCOPY} -R .eeprom -O ihex "${PROJECT_NAME}.elf" "${PROJECT_NAME}.hex" DEPENDS strip)
add_custom_target(eeprom    ${OBJCOPY} -j .eeprom --change-section-lma .eeprom=0 -O ihex "${PROJECT_NAME}.elf" "${PROJECT_NAME}.eeprom" DEPENDS strip)
add_custom_target(flash     ${AVRDUDE} -c usbasp -p ${MCU} -U flash:w:${PROJECT_NAME}.hex DEPENDS hex)
