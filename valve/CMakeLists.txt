cmake_minimum_required(VERSION 3.16)

# set up toolchain
find_program(AVR_GCC     avr-gcc     REQUIRED)
find_program(AVR_STRIP   avr-strip   REQUIRED)
find_program(AVR_OBJCOPY avr-objcopy REQUIRED)
find_program(AVR_OBJDUMP avr-objdump REQUIRED)
find_program(AVRDUDE     avrdude     REQUIRED)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER  ${AVR_GCC})

set(PROJECT_NAME valve)
project(${PROJECT_NAME} VERSION 0.2 LANGUAGES C)

# set up for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE sources src/*.c inc/*.h)
file(GLOB_RECURSE data resources/*)

set(MCU attiny2313)
set(AVRDUDE_MCU t2313)

# for some reason just "date '+%-H;%-M;%-S'" did not work
execute_process(COMMAND "./resources/current_time.sh" 
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} 
  OUTPUT_VARIABLE CURRENT_TIME_LST
)

list(GET CURRENT_TIME_LST 0 CT_HOUR)
list(GET CURRENT_TIME_LST 1 CT_MINUTE)
list(GET CURRENT_TIME_LST 2 CT_SECOND)

# target settings
set(COMPILE_OPTIONS
  -mmcu=${MCU}
  -g0
  -Os
  -Wall
  -Wextra
  -Wundef
  -Wstrict-prototypes
  -ffunction-sections
  -fdata-sections
  -fstack-usage
  -flto
)

set(COMPILE_DEFINITIONS
  __AVR_ATtiny2313__
  __CT_HOUR=${CT_HOUR}
  __CT_MINUTE=${CT_MINUTE}
  __CT_SECOND=${CT_SECOND}
)

set(LINK_OPTIONS
  -mmcu=${MCU}
  -Wl,--gc-sections
  -Wl,--print-memory-usage
  -flto
)

add_executable(${PROJECT_NAME} ${sources} ${data})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.elf")
target_include_directories(${PROJECT_NAME} PRIVATE inc)
target_compile_features(${PROJECT_NAME} PRIVATE c_std_11)
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTIONS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMPILE_DEFINITIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${LINK_OPTIONS})

file(COPY ${data} DESTINATION resources)

# Custom targets
add_custom_target(strip   ALL ${AVR_STRIP} "${PROJECT_NAME}.elf" DEPENDS ${PROJECT_NAME})
add_custom_target(hex     ALL ${AVR_OBJCOPY} -R .eeprom -O ihex "${PROJECT_NAME}.elf" "${PROJECT_NAME}.hex" DEPENDS strip)
add_custom_target(lst     ALL ${AVR_OBJDUMP} -h -S "${PROJECT_NAME}.elf" > "${PROJECT_NAME}.lst" DEPENDS strip)
add_custom_target(eeprom      ${AVR_OBJCOPY} -j .eeprom --change-section-lma .eeprom=0 -O ihex "${PROJECT_NAME}.elf" "${PROJECT_NAME}.eeprom" DEPENDS strip)

add_custom_target(program      ${AVRDUDE} -c usbasp -p ${AVRDUDE_MCU} -U flash:w:${PROJECT_NAME}.hex DEPENDS hex)
add_custom_target(program_eeprom ${AVRDUDE} -c usbasp -p ${AVRDUDE_MCU} -U eeprom:w:${PROJECT_NAME}.eeprom DEPENDS eeprom)
add_custom_target(program_all  ${AVRDUDE} -c usbasp -p ${AVRDUDE_MCU} -U flash:w:${PROJECT_NAME}.hex -U eeprom:w:${PROJECT_NAME}.eeprom DEPENDS hex eeprom)
add_custom_target(program_fuse ${AVRDUDE} -c usbasp -p ${AVRDUDE_MCU} -U lfuse:w:0xcf:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m)
