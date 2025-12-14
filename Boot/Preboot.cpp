#include "Preboot.h"
#include "CPU.h"

namespace BootLoader
{
	// CR0 flags
	constexpr unsigned __int64 CR0_EM = 1ULL << 2; // Emulation
	constexpr unsigned __int64 CR0_MP = 1ULL << 1; // Monitor Coprocessor
	constexpr unsigned __int64 CR0_NE = 1ULL << 5; // Numeric Error

	// CR4 flags
	constexpr unsigned __int64 CR4_OSFXSR = 1ULL << 9;	 // OS supports FXSAVE/FXRSTOR
	constexpr unsigned __int64 CR4_OSXMMEXCPT = 1ULL << 10; // OS supports unmasked SIMD exceptions

	void EnableAVX(void) noexcept
	{
		auto cr0 = CPU::ReadControlRegister<0>();
		cr0 &= ~CR0_EM;	    // clear EM
		cr0 |= CR0_MP | CR0_NE; // set MP and NE
		CPU::WriteControlRegister<0>(cr0);

		auto cr4 = CPU::ReadControlRegister<4>();
		cr4 |= CR4_OSFXSR | CR4_OSXMMEXCPT;
		CPU::WriteControlRegister<4>(cr4);
	}

	void InitialisePreboot(Uefi& uefi) noexcept
	{
		auto bootServices = uefi.AcquireBootServices();
		EnableAVX();

		uefi.framebuffer.Initialise(bootServices);
		uefi.allocator.Initialise(bootServices);

		const auto page1 = uefi.allocator.AllocatePage();
		const auto page2 = uefi.allocator.AllocatePage();
		if (page1 == page2 || page1 == ~0ui64 || page2 == ~0ui64) return uefi.framebuffer.Clear(Colour::Red);

		uefi.framebuffer.Clear(Colour::Grey);
	}
} // namespace BootLoader
