################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include parser/subdir.mk
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
all: libansisop-parser.so

dependents:
	-cd /home/utnso/git/tp/commons/Debug && $(MAKE) clean all

# Tool invocations
libansisop-parser.so: $(OBJS) $(USER_OBJS) /home/utnso/git/tp/commons/Debug/libcommons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L../../commons/Debug -shared -o "libansisop-parser.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(LIBRARIES) libansisop-parser.so
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/git/tp/commons/Debug/libcommons.so:

-include ../makefile.targets
