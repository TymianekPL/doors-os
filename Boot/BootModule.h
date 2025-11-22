#pragma once

#include "UEFI.h"

namespace BootLoader
{
	void Main(Uefi& uefi) noexcept
	{
		uefi.Reset(ResetType::Cold, 0);
	}
} // namespace BootLoader
