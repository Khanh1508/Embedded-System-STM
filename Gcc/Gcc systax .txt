all:
	arm-none-eabi-gcc -mcpu=cortex-m4 -c -std=gnu11 main.c -o main.o
	arm-none-eabi-gcc -mcpu=cortex-m4 -c -x assembler-with-cpp startup_stm32f411vetx.s -o start.o
	arm-none-eabi-gcc -o project.elf main.o start.o -mcpu=cortex-m4 -T"STM32F411VETX_FLASH.ld" -Wl,-Map="file.map" -Wl,--gc-sections -static
	arm-none-eabi-objcopy -O binary project.elf project.bin
clean:
	-rm *.bin
	-rm *.elf
	-rm *.o