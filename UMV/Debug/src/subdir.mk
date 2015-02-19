################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/UMV.c \
../src/configUMV.c \
../src/consola.c \
../src/consolaLoca.c \
../src/memoria.c \
../src/sincro.c \
../src/testsUmv.c 

OBJS += \
./src/UMV.o \
./src/configUMV.o \
./src/consola.o \
./src/consolaLoca.o \
./src/memoria.o \
./src/sincro.o \
./src/testsUmv.o 

C_DEPS += \
./src/UMV.d \
./src/configUMV.d \
./src/consola.d \
./src/consolaLoca.d \
./src/memoria.d \
./src/sincro.d \
./src/testsUmv.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"../../commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


