################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs1260/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./main.obj" \
"./driverlib/MSP430i2xx/cs.obj" \
"./driverlib/MSP430i2xx/eusci_a_spi.obj" \
"./driverlib/MSP430i2xx/eusci_a_uart.obj" \
"./driverlib/MSP430i2xx/eusci_b_i2c.obj" \
"./driverlib/MSP430i2xx/eusci_b_spi.obj" \
"./driverlib/MSP430i2xx/flashctl.obj" \
"./driverlib/MSP430i2xx/gpio.obj" \
"./driverlib/MSP430i2xx/mpy.obj" \
"./driverlib/MSP430i2xx/pmm.obj" \
"./driverlib/MSP430i2xx/sd24.obj" \
"./driverlib/MSP430i2xx/sfr.obj" \
"./driverlib/MSP430i2xx/timer_a.obj" \
"./driverlib/MSP430i2xx/tlv.obj" \
"./driverlib/MSP430i2xx/wdt.obj" \
"../lnk_msp430i2021.cmd" \
$(GEN_CMDS__FLAG) \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include driverlib/MSP430i2xx/subdir_vars.mk
-include subdir_rules.mk
-include driverlib/MSP430i2xx/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
HVB_Prototype.out 

EXE_OUTPUTS__QUOTED += \
"HVB_Prototype.out" 


# All Target
all: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "HVB_Prototype.out"

# Tool invocations
HVB_Prototype.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: MSP430 Linker'
	"C:/ti/ccs1260/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/bin/cl430" -vmsp -O2 --use_hw_mpy=16 --advice:power="all" --define=__MSP430i2021__ --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number -z -m"HVB_Prototype.map" --heap_size=320 --stack_size=80 -i"C:/ti/ccs1260/ccs/ccs_base/msp430/include" -i"C:/ti/ccs1260/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/lib" -i"C:/ti/ccs1260/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="HVB_Prototype_linkInfo.xml" --use_hw_mpy=16 --rom_model -o "HVB_Prototype.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)
	-$(RM) "main.obj" "driverlib\MSP430i2xx\cs.obj" "driverlib\MSP430i2xx\eusci_a_spi.obj" "driverlib\MSP430i2xx\eusci_a_uart.obj" "driverlib\MSP430i2xx\eusci_b_i2c.obj" "driverlib\MSP430i2xx\eusci_b_spi.obj" "driverlib\MSP430i2xx\flashctl.obj" "driverlib\MSP430i2xx\gpio.obj" "driverlib\MSP430i2xx\mpy.obj" "driverlib\MSP430i2xx\pmm.obj" "driverlib\MSP430i2xx\sd24.obj" "driverlib\MSP430i2xx\sfr.obj" "driverlib\MSP430i2xx\timer_a.obj" "driverlib\MSP430i2xx\tlv.obj" "driverlib\MSP430i2xx\wdt.obj" 
	-$(RM) "main.d" "driverlib\MSP430i2xx\cs.d" "driverlib\MSP430i2xx\eusci_a_spi.d" "driverlib\MSP430i2xx\eusci_a_uart.d" "driverlib\MSP430i2xx\eusci_b_i2c.d" "driverlib\MSP430i2xx\eusci_b_spi.d" "driverlib\MSP430i2xx\flashctl.d" "driverlib\MSP430i2xx\gpio.d" "driverlib\MSP430i2xx\mpy.d" "driverlib\MSP430i2xx\pmm.d" "driverlib\MSP430i2xx\sd24.d" "driverlib\MSP430i2xx\sfr.d" "driverlib\MSP430i2xx\timer_a.d" "driverlib\MSP430i2xx\tlv.d" "driverlib\MSP430i2xx\wdt.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

