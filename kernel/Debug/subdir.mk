################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../config_kernel.c \
../glb_header_kernel.c \
../kernel.c \
../pcp.c \
../plp.c \
../print_kernel.c

OBJS += \
./config_kernel.o \
./glb_header_kernel.o \
./kernel.o \
./pcp.o \
./plp.o \
./print_kernel.o
C_DEPS += \
./config_kernel.d \
./glb_header_kernel.d \
./kernel.d \
./pcp.d \
./plp.d \
./print_kernel.d


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I../../commons -I../../ansisop-parser -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


