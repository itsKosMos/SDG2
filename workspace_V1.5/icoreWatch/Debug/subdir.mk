################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../coreWatch.c \
../fsm.c \
../kbhit.c \
../reloj.c \
../tmr.c 

OBJS += \
./coreWatch.o \
./fsm.o \
./kbhit.o \
./reloj.o \
./tmr.o 

C_DEPS += \
./coreWatch.d \
./fsm.d \
./kbhit.d \
./reloj.d \
./tmr.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I"C:\SysGCC\Raspberry\include" -I"C:\SysGCC\Raspberry\include\wiringPi" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


