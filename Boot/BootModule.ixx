export module BootModule;

export import UEFI;

namespace BootLoader
{
	export void Main(Uefi& uefi) noexcept
	{
		uefi.Reset(ResetType::Cold, 0);
	}
}
