################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
driverlib/MSP430i2xx/%.obj: ../driverlib/MSP430i2xx/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs1260/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/bin/cl430" -vmsp --use_hw_mpy=16 --include_path="C:/ti/ccs1260/ccs/ccs_base/msp430/include" --include_path="C:/Users/AES/Documents/GitHub/HVB_Prototype/driverlib/MSP430i2xx" --include_path="C:/Users/AES/Documents/GitHub/HVB_Prototype" --include_path="C:/ti/ccs1260/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/include" --advice:power="all" --advice:hw_config="1" --define=__MSP430i2021__ -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="driverlib/MSP430i2xx/$(basename $(<F)).d_raw" --obj_directory="driverlib/MSP430i2xx" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


