#include "BootModule.h"

namespace BootLoader
{
	void Main(Uefi& uefi) noexcept
	{
		InitialisePreboot(uefi);

		uefi.Reset(ResetType::Shutdown, 0);
	}
} // namespace BootLoader
