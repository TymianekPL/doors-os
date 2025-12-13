#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

constexpr std::size_t PageBits = 12;
constexpr std::size_t PageSize = 1 << PageBits;

enum struct Colour : std::uint32_t
{
	Black = 0,
	White = 0xffffff,
	Grey = 0x111111,
	Red = 0xff0000
};

enum struct ColourComponent : std::uint8_t
{
	Red = 0,
	Green = 8,
	Blue = 16,
	Alpha = 24 // reserved/unused
};

enum struct PFNUse : std::uint8_t
{
	Unused,
	ProcessPrivate,
	MappedFile,
	DriverLocked,
	UserStack,
	KernelStack,
	KernelHeap,
	MetaFile,
	NonPagedPool,
	PagedPool,
	PTE,
	Shareable,
	PageTable,
	FSCache
};

// clang-format off
enum struct PFNRegion : std::uint8_t
{
	Active  = 0b01, // CRITICAL: MUST BE ODD  (Least Significant Bit SET)
	Standby = 0b11, // CRITICAL: MUST BE ODD  (Least Significant Bit SET)
	Zero    = 0b10, // CRITICAL: MUST BE EVEN (Least Significant Bit CLEAR)
	Free    = 0b00  // CRITICAL: MUST BE EVEN (Least Significant Bit CLEAR)
};
// clang-format on

struct alignas(4) MMPFNEntry
{
	PFNUse use{};
	PFNRegion region{};
	std::uint16_t referenceCount{};
};
static_assert(sizeof(MMPFNEntry) == 4);

namespace Colours
{
	template <ColourComponent TComponent>
	constexpr std::uint8_t Extract(const Colour from)
	{
		return (std::to_underlying(from) >> std::to_underlying(TComponent)) & UCHAR_MAX;
	}
	template <ColourComponent TComponent>
	constexpr void Modify(Colour colour, std::uint8_t newValue)
	{
		colour = static_cast<Colour>(std::to_underlying(colour) | (newValue << std::to_underlying(TComponent)));
	}
} // namespace Colours
