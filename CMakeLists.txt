#         ___    ___
#  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
# / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
#_\__\___/___|  |___/\___|_||_/__/\___/_|__________________________________
# CO₂ Sensor for Caving -- https://github.com/keppler/co2
#
# Thanks to https://www.kuon.ch/post/2018-07-11-avr-cmake/ for important
# hints on using CMake with AVR toolchain! :)

cmake_minimum_required(VERSION 3.11)

project("co2-scd41")
enable_language(C ASM)

set(F_CPU 1000000UL)
set(MCU attiny85)

# Use AVR GCC toolchain
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_ASM_COMPILER avr-gcc)

# Pass defines to compiler
add_definitions(
        -DF_CPU=${F_CPU}
        -D__DELAY_BACKWARD_COMPATIBLE__  # see https://www.nongnu.org/avr-libc/user-manual/group__util__delay.html
)

# mmcu MUST be passed to both the compiler and linker; this handles the linker:
set(CMAKE_EXE_LINKER_FLAGS -mmcu=${MCU})

add_compile_options(
        -mmcu=${MCU} # MCU
        -std=c99 # C99 standard
        -Os # optimize for size
        -Wall # enable warnings
        -Wno-main
        -Wundef
        -pedantic
        -Wstrict-prototypes
        -Werror
        -Wfatal-errors
        -Wl,--relax,--gc-sections
        -g
        -gdwarf-2
        -funsigned-char # a few optimizations
        -funsigned-bitfields
        -fpack-struct
        -fshort-enums
        -ffunction-sections
        -fdata-sections
        -fno-split-wide-types
        -fno-tree-scev-cprop
)

# Create one target
add_executable(${PROJECT_NAME}
        button.c
        main.c
        menu.c
        i2cmaster.S
        timer.c
        SSD1306.c
        SCD4x.c
        VCC.c
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}.elf)

# Strip binary for upload
add_custom_target(strip ALL avr-strip ${PROJECT_NAME}.elf
        COMMAND avr-size -C --mcu=${MCU} ${PROJECT_NAME}.elf
        DEPENDS ${PROJECT_NAME}
)

# Transform binary into hex file, we ignore the eeprom segments in the step
add_custom_target(hex ALL avr-objcopy -R .eeprom -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
        DEPENDS strip
)

# Transform binary into hex file, this is the eeprom part (empty if you don't
# use eeprom static variables)
add_custom_target(eeprom avr-objcopy -j .eeprom  --set-section-flags=.eeprom="alloc,load"  --change-section-lma .eeprom=0 -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.eep
        DEPENDS strip)

# Clean extra files
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.hex;${PROJECT_NAME}.eep")
