##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O2 -ggdb -fomit-frame-pointer -falign-functions=16
  USE_OPT += -fno-stack-protector -ftracer -ftree-loop-distribute-patterns
  USE_OPT += -frename-registers -freorder-blocks -fconserve-stack -g3
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT  = -fno-rtti
  USE_CPPOPT += -std=gnu++11
  USE_CPPOPT += -fno-exceptions -fno-unwind-tables -fno-threadsafe-statics
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT = 
endif

# Enable this if you want link time optimizations (LTO)
ifeq ($(USE_LTO),)
  USE_LTO = no
endif

# If enabled, this option allows to compile the application in THUMB mode.
ifeq ($(USE_THUMB),)
  USE_THUMB = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Enables the use of FPU on Cortex-M4 (no, softfp, hard).
ifeq ($(USE_FPU),)
  USE_FPU = hard
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Define project name here
PROJECT = ch

# Imported source files and paths
CHIBIOS = ChibiOS
include $(CHIBIOS)/os/hal/ports/STM32/STM32F4xx/platform.mk
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/rt/ports/ARMCMx/compilers/GCC/mk/port_stm32f4xx.mk
include $(CHIBIOS)/os/rt/rt.mk
include ${CHIBIOS}/os/hal/osal/rt/osal.mk
include $(CHIBIOS)/test/rt/test.mk
include $(CHIBIOS)/os/various/cpp_wrappers/chcpp.mk
include src/board/board.mk
include src/src.mk

LDSCRIPT= $(PORTLD)/STM32F407xG.ld
# LDSCRIPT= STM32F407xG.ld

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(PORTSRC) \
       $(KERNSRC) \
       $(TESTSRC) \
       $(HALSRC) \
       $(PLATFORMSRC) \
       $(CHIBIOS)/os/hal/lib/streams/chprintf.c \
       $(CHIBIOS)/os/various/syscalls.c \
       $(BOARDSRC) \
       $(addprefix src/, $(PROJCSRC))

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC += $(CHCPPSRC) $(addprefix src/, $(PROJCPPSRC))

# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC =

# List ASM source files here
ASMSRC = $(PORTASM)

INCDIR += $(PORTINC) $(KERNINC) $(TESTINC) \
          $(HALINC) $(PLATFORMINC) $(BOARDINC) \
          $(OSALINC) \
          $(CHIBIOS)/os/various \
          $(CHCPPINC) \
          $(CHIBIOS)/os/hal/lib/streams/ \
          ./src

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU  = cortex-m4

#TRGT = arm-elf-
TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
#LD   = $(TRGT)gcc
LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# ARM-specific options here
AOPT =

# THUMB-specific options here
TOPT = -mthumb -DTHUMB

# Define C warning options here
CWARN = -Wall -Wextra -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra

#
# Compiler settings
##############################################################################

##############################################################################
# Start of default section
#

# List all default C defines here, like -D_DEBUG=1
DDEFS =

# List all default ASM defines here, like -D_DEBUG=1
DADEFS =

# List all default directories to look for include files here
DINCDIR =

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS =

#
# End of default section
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS  = -DUAVCAN_TOSTRING=0 \
		 -DUAVCAN_STM32_NUM_IFACES=1 \
		 -DUAVCAN_STM32_TIMER_NUMBER=3 \
		 -DUAVCAN_TINY=0

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR = src/

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

#
# UAVCAN
#
include uavcan/libuavcan/include.mk
include src/can-driver/include.mk

CPPSRC += $(LIBUAVCAN_SRC) $(LIBUAVCAN_STM32_SRC)
UINCDIR += $(LIBUAVCAN_INC) $(LIBUAVCAN_STM32_INC) ./dsdlc_generated

# run uavcan dsdl compiler
$(info $(shell $(LIBUAVCAN_DSDLC) uavcan-dsdl/uavcan))

#
# End of user defines
##############################################################################

RULESPATH = $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC
include $(RULESPATH)/rules.mk

flash: build/$(PROJECT).elf
	openocd -f oocd.cfg -f flash_script.cfg
