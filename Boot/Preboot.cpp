#include "Preboot.h"

namespace BootLoader
{
	void InitialisePreboot(Uefi& uefi) noexcept
	{
		auto bootServices = uefi.AcquireBootServices();
		bootServices.Print(u"Hello!\r\n");
	}
} // namespace BootLoader
