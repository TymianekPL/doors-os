#include "UEFI.h"
#include <intrin.h>
#include <utility>

namespace BootLoader
{
	void Uefi::Reset(ResetType type, EfiStatus status)
	{
		this->_systemTable->RuntimeServices->ResetSystem(static_cast<EFI_RESET_TYPE>(std::to_underlying(type)), status, 0, nullptr);
		while (true) __halt();
	}
} // namespace BootLoader
