module;

#include <Uefi.h>

export module UEFI;

namespace BootLoader
{
	export enum struct ResetType
	{
		Cold = EfiResetCold,
		Warm = EfiResetWarm,
		Shutdown = EfiResetShutdown
	};

	export using EfiStatus = EFI_STATUS;

	export class Uefi
	{
	public:
		explicit Uefi(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
			: _imageHandle(ImageHandle), _systemTable(SystemTable)
		{
		}

		[[noreturn]] void Reset(ResetType type, EfiStatus status);

	private:
		EFI_HANDLE _imageHandle;
		EFI_SYSTEM_TABLE* _systemTable;
	};
}
