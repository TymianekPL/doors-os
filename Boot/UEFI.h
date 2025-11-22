#pragma once
#include <Uefi.h>

namespace BootLoader
{
	enum struct ResetType
	{
		Cold = EfiResetCold,
		Warm = EfiResetWarm,
		Shutdown = EfiResetShutdown
	};

	using EfiStatus = EFI_STATUS;

	class Uefi
	{
	   public:
		explicit Uefi(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) : _imageHandle(ImageHandle), _systemTable(SystemTable) {}

		[[noreturn]] void Reset(ResetType type, EfiStatus status);

	   private:
		EFI_HANDLE _imageHandle;
		EFI_SYSTEM_TABLE* _systemTable;
	};
} // namespace BootLoader
