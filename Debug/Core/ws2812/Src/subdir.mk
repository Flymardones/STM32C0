################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/ws2812/Src/ws2812.c \
../Core/ws2812/Src/ws2812_pwm.c \
../Core/ws2812/Src/ws2812_spi.c \
../Core/ws2812/Src/ws2812_uart.c 

OBJS += \
./Core/ws2812/Src/ws2812.o \
./Core/ws2812/Src/ws2812_pwm.o \
./Core/ws2812/Src/ws2812_spi.o \
./Core/ws2812/Src/ws2812_uart.o 

C_DEPS += \
./Core/ws2812/Src/ws2812.d \
./Core/ws2812/Src/ws2812_pwm.d \
./Core/ws2812/Src/ws2812_spi.d \
./Core/ws2812/Src/ws2812_uart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/ws2812/Src/%.o Core/ws2812/Src/%.su Core/ws2812/Src/%.cyclo: ../Core/ws2812/Src/%.c Core/ws2812/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32C011xx -c -I../Core/Inc -I../Core/ws2812/Inc -I../Drivers/STM32C0xx_HAL_Driver/Inc -I../Drivers/STM32C0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32C0xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include/ -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -I../Middlewares/Third_Party/CMSIS/RTOS2/Include/ -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0/ -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-ws2812-2f-Src

clean-Core-2f-ws2812-2f-Src:
	-$(RM) ./Core/ws2812/Src/ws2812.cyclo ./Core/ws2812/Src/ws2812.d ./Core/ws2812/Src/ws2812.o ./Core/ws2812/Src/ws2812.su ./Core/ws2812/Src/ws2812_pwm.cyclo ./Core/ws2812/Src/ws2812_pwm.d ./Core/ws2812/Src/ws2812_pwm.o ./Core/ws2812/Src/ws2812_pwm.su ./Core/ws2812/Src/ws2812_spi.cyclo ./Core/ws2812/Src/ws2812_spi.d ./Core/ws2812/Src/ws2812_spi.o ./Core/ws2812/Src/ws2812_spi.su ./Core/ws2812/Src/ws2812_uart.cyclo ./Core/ws2812/Src/ws2812_uart.d ./Core/ws2812/Src/ws2812_uart.o ./Core/ws2812/Src/ws2812_uart.su

.PHONY: clean-Core-2f-ws2812-2f-Src

