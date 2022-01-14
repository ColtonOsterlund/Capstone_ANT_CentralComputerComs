PROJECT_NAME := demo_gateway

export OUTPUT_FILENAME
MAKEFILE_NAME := $(MAKEFILE_LIST)
MAKEFILE_DIR := $(dir $(MAKEFILE_NAME) )

SDK_ROOT := ../../../nrf5_sdk
PROJ_DIR := ..

TEMPLATE_PATH := ../../../toolchain/gcc
LINKER_SCRIPT_PATH = $(SDK_ROOT)/components/toolchain/gcc

ifeq ($(OS),Windows_NT)
include $(TEMPLATE_PATH)/Makefile.windows
else
include $(TEMPLATE_PATH)/Makefile.posix
endif

MK := mkdir
RM := rm -rf

# Toolchain commands
CC              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gcc'
AS              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-as'
AR              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ar' -r
LD              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ld'
NM              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-nm'
OBJDUMP         := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objdump'
OBJCOPY         := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objcopy'
SIZE            := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-size'

#function for removing duplicates in a list
remduplicates = $(strip $(if $1,$(firstword $1) $(call remduplicates,$(filter-out $(firstword $1),$1))))

# Include paths
INC_PATHS = -I$(PROJ_DIR)/inc
INC_PATHS += -I$(PROJ_DIR)/config
INC_PATHS += -I$(PROJ_DIR)/../../inc
INC_PATHS += -I$(SDK_ROOT)/components/ant/ant_channel_config
INC_PATHS += -I$(SDK_ROOT)/components/ant/ant_search_config
INC_PATHS += -I$(SDK_ROOT)/components/ant/ant_stack_config
INC_PATHS += -I$(SDK_ROOT)/components/drivers_nrf/common
INC_PATHS += -I$(SDK_ROOT)/components/drivers_nrf/config
INC_PATHS += -I$(SDK_ROOT)/components/drivers_nrf/delay
INC_PATHS += -I$(SDK_ROOT)/components/drivers_nrf/gpiote
INC_PATHS += -I$(SDK_ROOT)/components/drivers_nrf/hal
INC_PATHS += -I$(SDK_ROOT)/components/libraries/button
INC_PATHS += -I$(SDK_ROOT)/components/libraries/scheduler
INC_PATHS += -I$(SDK_ROOT)/components/libraries/timer
INC_PATHS += -I$(SDK_ROOT)/components/libraries/util
INC_PATHS += -I$(SDK_ROOT)/components/libraries/log
INC_PATHS += -I$(SDK_ROOT)/components/libraries/log/src
INC_PATHS += -I$(SDK_ROOT)/components/libraries/strerror
INC_PATHS += -I$(SDK_ROOT)/components/libraries/bsp
INC_PATHS += -I$(SDK_ROOT)/components/boards
INC_PATHS += -I$(SDK_ROOT)/components/softdevice/common/softdevice_handler
INC_PATHS += -I$(SDK_ROOT)/components/toolchain
INC_PATHS += -I$(SDK_ROOT)/components/toolchain/gcc
INC_PATHS += -I$(SDK_ROOT)/external/segger_rtt
INC_PATHS += -I$(SDK_ROOT)/components/softdevice/s332/headers
INC_PATHS += -I$(SDK_ROOT)/components/softdevice/s332/headers/nrf52
INC_PATHS += -I$(SDK_ROOT)/components/device
INC_PATHS += -I$(SDK_ROOT)/components/toolchain/CMSIS/Include

#source common to all targets
C_SOURCE_FILES += \
$(abspath $(PROJ_DIR)/src/main.c ) \
$(abspath $(SDK_ROOT)/components/libraries/bsp/bsp.c ) \
$(abspath $(SDK_ROOT)/components/boards/boards.c ) \
$(abspath $(SDK_ROOT)/components/ant/ant_stack_config/ant_stack_config.c ) \
$(abspath $(SDK_ROOT)/components/drivers_nrf/common/nrf_drv_common.c ) \
$(abspath $(SDK_ROOT)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c ) \
$(abspath $(SDK_ROOT)/components/libraries/button/app_button.c ) \
$(abspath $(SDK_ROOT)/components/libraries/util/app_error.c ) \
$(abspath $(SDK_ROOT)/components/libraries/util/app_util_platform.c ) \
$(abspath $(SDK_ROOT)/components/libraries/util/nrf_assert.c ) \
$(abspath $(SDK_ROOT)/components/libraries/util/app_error_weak.c ) \
$(abspath $(SDK_ROOT)/components/libraries/timer/app_timer.c ) \
$(abspath $(SDK_ROOT)/components/libraries/hardfault/nrf52/handler/hardfault_handler_gcc.c ) \
$(abspath $(SDK_ROOT)/components/libraries/hardfault/hardfault_implementation.c ) \
$(abspath $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_serial.c ) \
$(abspath $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c ) \
$(abspath $(SDK_ROOT)/external/segger_rtt/RTT_Syscalls_GCC.c ) \
$(abspath $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT.c ) \
$(abspath $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_printf.c ) \
$(abspath $(SDK_ROOT)/components/toolchain/system_nrf52.c) \

#assembly files common to all targets
ASM_SOURCE_FILES  = $(abspath $(SDK_ROOT)/components/toolchain/gcc/gcc_startup_nrf52.s)

OBJECT_DIRECTORY = _build
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY))

#C Compiler Flags
CFLAGS += -DANT_STACK_SUPPORT_REQD
CFLAGS += -DNRF52832
CFLAGS += -DS332
CFLAGS += -DBOARD_CUSTOM
CFLAGS += -DNRF52
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs --std=gnu99
CFLAGS += -Wall  -O3 -g3
#CFLAGS += -Wall -Werror -O3 -g3
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums

#Linker Flags
LDFLAGS += -Xlinker -Map=$(LISTING_DIRECTORY)/$(OUTPUT_FILENAME).map
LDFLAGS += -mthumb -mabi=aapcs -L $(LINKER_SCRIPT_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys

LIBS = -L$(PROJ_DIR)/../../bin -l:ANT_BLAZE_Gateway_Library_GCC.a

#Assembler Flags



ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DANT_STACK_SUPPORT_REQD
ASMFLAGS += -DDEBUG
ASMFLAGS += -DS332
ASMFLAGS += -DBOARD_CUSTOM
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DNRF52
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DSWI_DISABLE0

#default target - first one defined
default: clean demo_gateway

C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(call remduplicates, $(dir $(C_SOURCE_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )

ASM_SOURCE_FILE_NAMES = $(notdir $(ASM_SOURCE_FILES))
ASM_PATHS = $(call remduplicates, $(dir $(ASM_SOURCE_FILES) ))
ASM_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(ASM_SOURCE_FILE_NAMES:.s=.o) )

vpath %.c $(C_PATHS)
vpath %.s $(ASM_PATHS)

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

demo_gateway: OUTPUT_FILENAME := demo_gateway
demo_gateway: LINKER_SCRIPT=./demo_gateway_gcc_nrf52.ld

demo_gateway: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -lm -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e finalize

# Target to create build directories
$(BUILD_DIRECTORIES):
	echo $(MAKEFILE_NAME)
	$(MK) $@

# Create objects from C SRC files
$(OBJECT_DIRECTORY)/%.o: %.c
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<

# Assemble files
$(OBJECT_DIRECTORY)/%.o: %.s
	@echo Assembly file: $(notdir $<)
	$(NO_ECHO)$(CC) $(ASMFLAGS) $(INC_PATHS) -c -o $@ $<

#Link
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -lm -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out

## Create binary .bin file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex

finalize: genbin genhex echosize

genbin:
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file
genhex:
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex
    
echosize:
	-@echo ''
	$(NO_ECHO)$(SIZE) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	-@echo ''
    
clean:
	$(RM) $(BUILD_DIRECTORIES)