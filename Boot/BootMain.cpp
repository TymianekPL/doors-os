#include <Uefi.h>

import BootModule;

extern "C" EFI_STATUS EFIAPI EfiEntry(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) noexcept
{
	BootLoader::Uefi uefi{ImageHandle, SystemTable};
	BootLoader::Main(uefi);
	for (;;);
}
