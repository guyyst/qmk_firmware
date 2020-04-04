# MCU name
MCU = STM32F072
BOARD = ST_STM32F072B_DISCOVERY

# Build Options
#   comment out to disable the options.
#

SRC += satisfaction_oled.c

#BOOTMAGIC_ENABLE = yes	# Virtual DIP switch configuration
MOUSEKEY_ENABLE = yes	# Mouse keys
EXTRAKEY_ENABLE = yes	# Audio control and System control
CONSOLE_ENABLE = yes	# Console for debug
COMMAND_ENABLE = yes    # Commands for debug and configuration
SLEEP_LED_ENABLE = yes  # Breathing sleep LED during USB suspend
NKRO_ENABLE = yes	    # USB Nkey Rollover
CUSTOM_MATRIX = no # Custom matrix file
ENCODER_ENABLE = yes
QWIIC_ENABLE += MICRO_OLED

DEFAULT_FOLDER = cannonkeys/satisfaction75/rev1


# Enter lower-power sleep mode when on the ChibiOS idle thread
OPT_DEFS += -DCORTEX_ENABLE_WFI_IDLE=TRUE



# Enable another OLED mode to track key statistics and display a key history
OPT_DEFS += -DENABLE_STAT_TRACKING



# Enable snake mode c:
OPT_DEFS += -DENABLE_SNAKE_MODE
