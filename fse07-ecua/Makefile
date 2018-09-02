AS=arm-none-eabi-as
CC=arm-none-eabi-gcc
CXX=arm-none-eabi-g++
OBJCOPY=arm-none-eabi-objcopy
SIZE=arm-none-eabi-size

ARCHFLAGS=-mcpu=cortex-m3 -mthumb -mfloat-abi=soft
COMMONFLAGS=$(ARCHFLAGS) -DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD -DSTM32F103xB -DTX_WITH_CANDB=1 -I Inc -I Drivers/StdPeriph/Inc -I Drivers/Cmsis -O2 -Wall -Werror=switch -ffunction-sections
ASFLAGS=$(ARCHFLAGS)
CFLAGS=$(COMMONFLAGS)
CXXFLAGS=$(COMMONFLAGS) -std=c++17 -fno-exceptions -fno-rtti
LDFLAGS=-specs=nosys.specs -specs=nano.specs -T STM32F103C8Tx_FLASH.ld -Wl,--gc-sections -fno-exceptions -fno-rtti

HEXFILE=fse07-ecua.hex
ELF=fse07-ecua.elf
OBJS=\
        Drivers/StdPeriph/Src/misc.o \
        Drivers/StdPeriph/Src/stm32f10x_can.o \
        Drivers/StdPeriph/Src/stm32f10x_flash.o \
        Drivers/StdPeriph/Src/stm32f10x_gpio.o \
        Drivers/StdPeriph/Src/stm32f10x_rcc.o \
        Drivers/StdPeriph/Src/stm32f10x_spi.o \
        Drivers/StdPeriph/Src/stm32f10x_tim.o \
        Drivers/StdPeriph/Src/stm32f10x_usart.o \
        Src/BMS/bq76.o \
        Src/BMS/bq76_crc.o \
        Src/CANdb/can_ECUA.o \
        Src/CANdb/tx2_can.o \
        Src/CANdb/tx2_ringbuf.o \
        Src/ecua/airs.o \
        Src/ecua/balancing.o \
        Src/ecua/bms.o \
        Src/ecua/bmstemperaturemap.o \
        Src/ecua/ecuacan.o \
        Src/ecua/flash.o \
        Src/ecua/main_ecua4.o \
        Src/ecua/peripherals.o \
        Src/ecua/sdc.o \
        Src/ecua/voltagesense.o \
        Src/can.o \
        Src/gpio.o \
        Src/main.o \
        Src/mcp3302.o \
        Src/timer.o \
        startup/startup_stm32f103xb.o

all: $(ELF) $(HEXFILE)

clean:
	rm -f $(ELF) $(HEXFILE) ./**.o

$(ELF): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)
	$(SIZE) $@

$(HEXFILE): $(ELF)
	$(OBJCOPY) -O ihex $< $@
