################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/DataBuf.cpp \
../src/chunk.cpp \
../src/main.cpp \
../src/types.cpp \
../src/utils.cpp 

OBJS += \
./src/DataBuf.o \
./src/chunk.o \
./src/main.o \
./src/types.o \
./src/utils.o 

CPP_DEPS += \
./src/DataBuf.d \
./src/chunk.d \
./src/main.d \
./src/types.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

