#include <Uefi.h>

#include "BootModule.h"

#include <SkipRTC.h>

extern "C" EFI_STATUS EFIAPI EfiEntry(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) noexcept
{
	BootLoader::Uefi uefi{ImageHandle, SystemTable};
	BootLoader::Main(uefi);
	for (;;);
}
