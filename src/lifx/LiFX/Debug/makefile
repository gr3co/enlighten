################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.2

ORDERED_OBJS += \
"./blinky.obj" \
"./cx2538_setup.obj" \
"./cx2538_startup_ccs.obj" \
"./lifx_bulb.obj" \
"./driverlib/adc.obj" \
"./driverlib/cpu.obj" \
"./driverlib/debug.obj" \
"./driverlib/flash.obj" \
"./driverlib/gpio.obj" \
"./driverlib/gptimer.obj" \
"./driverlib/i2c.obj" \
"./driverlib/interrupt.obj" \
"./driverlib/ioc.obj" \
"./driverlib/pka.obj" \
"./driverlib/sleepmode.obj" \
"./driverlib/ssi.obj" \
"./driverlib/sys_ctrl.obj" \
"./driverlib/systick.obj" \
"./driverlib/uart.obj" \
"./driverlib/udma.obj" \
"./driverlib/watchdog.obj" \
"../cx2538xf53_ccs.cmd" \
$(GEN_CMDS__FLAG) \
-l"libc.a" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include driverlib/subdir_vars.mk
-include subdir_rules.mk
-include driverlib/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
LiFX.out \

EXE_OUTPUTS__QUOTED += \
"LiFX.out" \

BIN_OUTPUTS += \
LiFX.hex \

BIN_OUTPUTS__QUOTED += \
"LiFX.hex" \


# All Target
all: LiFX.out

# Tool invocations
LiFX.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM Linker'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.2/bin/armcl" -mv7M3 --code_state=16 --abi=eabi -me -g --define=DEBUG --define=CC2538_USE_ALTERNATE_INTERRUPT_MAP --display_error_number --diag_warning=225 --diag_wrap=off --gen_func_subsections=on --ual -z -m"LiFX.map" --heap_size=0 --stack_size=256 -i"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.2/lib" -i"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.2/include" --reread_libs --warn_sections --display_error_number --diag_wrap=off --xml_link_info="LiFX_linkInfo.xml" --rom_model -o "LiFX.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

LiFX.hex: $(EXE_OUTPUTS)
	@echo 'Invoking: ARM Hex Utility'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.2/bin/armhex"  -o "LiFX.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)$(BIN_OUTPUTS__QUOTED)
	-$(RM) "blinky.pp" "cx2538_setup.pp" "cx2538_startup_ccs.pp" "lifx_bulb.pp" "driverlib\adc.pp" "driverlib\cpu.pp" "driverlib\debug.pp" "driverlib\flash.pp" "driverlib\gpio.pp" "driverlib\gptimer.pp" "driverlib\i2c.pp" "driverlib\interrupt.pp" "driverlib\ioc.pp" "driverlib\pka.pp" "driverlib\sleepmode.pp" "driverlib\ssi.pp" "driverlib\sys_ctrl.pp" "driverlib\systick.pp" "driverlib\uart.pp" "driverlib\udma.pp" "driverlib\watchdog.pp" 
	-$(RM) "blinky.obj" "cx2538_setup.obj" "cx2538_startup_ccs.obj" "lifx_bulb.obj" "driverlib\adc.obj" "driverlib\cpu.obj" "driverlib\debug.obj" "driverlib\flash.obj" "driverlib\gpio.obj" "driverlib\gptimer.obj" "driverlib\i2c.obj" "driverlib\interrupt.obj" "driverlib\ioc.obj" "driverlib\pka.obj" "driverlib\sleepmode.obj" "driverlib\ssi.obj" "driverlib\sys_ctrl.obj" "driverlib\systick.obj" "driverlib\uart.obj" "driverlib\udma.obj" "driverlib\watchdog.obj" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

