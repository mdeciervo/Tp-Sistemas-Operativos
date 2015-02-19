################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ansisop_prim.c \
../config_cpu.c \
../cpu.c 

OBJS += \
./ansisop_prim.o \
./config_cpu.o \
./cpu.o 

C_DEPS += \
./ansisop_prim.d \
./config_cpu.d \
./cpu.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp/ansisop-parser" -I"/home/utnso/git/tp/commons" -I../../ansisop-parser -I../../commons -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


