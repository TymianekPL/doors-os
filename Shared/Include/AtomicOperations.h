#pragma once

#include <intrin.h>
#include <cstdint>

namespace Atomic
{
	[[nodiscard]] std::uint8_t BitScanForward(unsigned long& index, const std::uint64_t value)
	{
		return _BitScanForward64(&index, value);
	}
} // namespace Atomic
