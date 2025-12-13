#include "BootModule.h"
#ifdef CHECKED_BUILD
#include <COM0dbg.h>
#endif
#include <Version.hpp>

namespace BootLoader
{
	void Main(Uefi& uefi) noexcept
	{
#ifdef CHECKED_BUILD
		Testing::Initialise();
		Testing::IssueFixedCommand("set version=" DOORS_VERSION_STRING);
#endif
		InitialisePreboot(uefi);

		uefi.Reset(ResetType::Shutdown, 0);
	}
} // namespace BootLoader
