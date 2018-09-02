#include "flash.hpp"

#include <stm32f10x_flash.h>

bool Flash::ErasePage(uint32_t pageAddress) {
	__disable_irq();
	FLASH_Unlock();
	FLASH->SR = FLASH_FLAG_EOP | FLASH_FLAG_WRPRTERR | FLASH_FLAG_PGERR;

	FLASH_ErasePage(pageAddress);
	__enable_irq();

	FLASH_Lock();
	return true;
}

bool Flash::Write(uint32_t flashAddress, const void* data, size_t length) {
	const size_t wordsToFlush = (length + 3) / 4;

	// FIXME: check page overflow
	//if ((uintptr_t)s_pointer + wordsToFlush * 4 >= s_readAddress + FLIGHT_DIARY_AREA_SIZE)
	//	return;

	__disable_irq();
	FLASH_Unlock();
	FLASH->SR = FLASH_SR_EOP | FLASH_FLAG_PGERR;

	const uint32_t* words = (uint32_t*) data;

	for (size_t i = 0; i < wordsToFlush; i++) {
		FLASH_ProgramWord(flashAddress, *words++);
		flashAddress += 4;
	}

	__enable_irq();
	FLASH_Lock();
	return true;
}
