################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: CPU

dependents:
	-cd /home/utnso/git/tp/commons/Debug && $(MAKE) all
	-cd /home/utnso/git/tp/ansisop-parser/Debug && $(MAKE) all

# Tool invocations
CPU: $(OBJS) $(USER_OBJS) /home/utnso/git/tp/commons/Debug/libcommons.so /home/utnso/git/tp/ansisop-parser/Debug/libansisop-parser.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L../../ansisop-parser/Debug -L../../commons/Debug -o "CPU" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) CPU
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/git/tp/commons/Debug/libcommons.so:
/home/utnso/git/tp/ansisop-parser/Debug/libansisop-parser.so:

-include ../makefile.targets
