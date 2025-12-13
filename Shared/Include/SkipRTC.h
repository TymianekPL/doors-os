#pragma once

#include <intrin.h>
#include <cstddef>
#include <cstdint>

#pragma comment(linker, "/alternatename:_CrtDbgReport=placeholder")
#pragma comment(linker, "/alternatename:__imp__CrtDbgReport=placeholder")

extern "C" void* __cdecl memset(void* pTarget, int value, size_t cbTarget); // NOLINT(readability-identifier-naming)
#pragma intrinsic(memset)

extern "C" void* __cdecl memcpy(void*, const void*, unsigned __int64); // NOLINT(readability-identifier-naming)
#pragma intrinsic(memcpy)

extern "C" void* __cdecl memmove(void*, const void*, unsigned __int64); // NOLINT(readability-identifier-naming)
#pragma intrinsic(memmove)

extern "C" void _RTC_CheckStackVars(void) // NOLINT(readability-identifier-naming) NOLINT(bugprone-reserved-identifier)
{
}
extern "C" void _RTC_InitBase(void) // NOLINT(readability-identifier-naming) NOLINT(bugprone-reserved-identifier)
{
}
extern "C" void _RTC_Shutdown(void) // NOLINT(readability-identifier-naming) NOLINT(bugprone-reserved-identifier)
{
}
extern "C" void placeholder(void) // NOLINT(readability-identifier-naming)
{
}

#pragma function(memset)
extern "C" void* __cdecl memset(_Out_ void* pTarget, _In_ int value, _In_ size_t cbTarget) // NOLINT(readability-identifier-naming)
{
	constexpr std::size_t unrollBytes = 4 * sizeof(__m128i);

	auto* destination = static_cast<std::uint8_t*>(pTarget);

	const std::uint8_t byteValue = static_cast<std::uint8_t>(value);
	const std::uint32_t value32 = byteValue * 0x01010101;

	while (((reinterpret_cast<std::uintptr_t>(destination) & (sizeof(__m128i) - 1)) != 0) && cbTarget > 0)
	{
		*destination++ = static_cast<std::uint8_t>(value);
		--cbTarget;
	}

	if (byteValue == 0)
	{
		const __m128i zero = _mm_setzero_si128();

		while (cbTarget >= unrollBytes)
		{
			_mm_stream_si128(reinterpret_cast<__m128i*>(destination + 0), zero);
			_mm_stream_si128(reinterpret_cast<__m128i*>(destination + 16), zero);
			_mm_stream_si128(reinterpret_cast<__m128i*>(destination + 32), zero);
			_mm_stream_si128(reinterpret_cast<__m128i*>(destination + 48), zero);
			destination += unrollBytes;
			cbTarget -= unrollBytes;
		}

		while (cbTarget >= sizeof(__m128i))
		{
			_mm_stream_si128(reinterpret_cast<__m128i*>(destination), zero);
			destination += sizeof(__m128i);
			cbTarget -= sizeof(__m128i);
		}

		while (cbTarget >= sizeof(std::uint64_t))
		{
			*reinterpret_cast<std::uint64_t*>(destination) = 0;
			destination += sizeof(std::uint64_t);
			cbTarget -= sizeof(std::uint64_t);
		}

		while (cbTarget-- > 0) *destination++ = 0;

		_mm_sfence();
		return pTarget;
	}

	const __m128i fill = _mm_set1_epi8(static_cast<char>(value));

	while (cbTarget >= unrollBytes)
	{
		_mm_stream_si128(reinterpret_cast<__m128i*>(destination + 0), fill);
		_mm_stream_si128(reinterpret_cast<__m128i*>(destination + 16), fill);
		_mm_stream_si128(reinterpret_cast<__m128i*>(destination + 32), fill);
		_mm_stream_si128(reinterpret_cast<__m128i*>(destination + 48), fill);
		destination += unrollBytes;
		cbTarget -= unrollBytes;
	}

	while (cbTarget >= sizeof(__m128i))
	{
		_mm_stream_si128(reinterpret_cast<__m128i*>(destination), fill);
		destination += sizeof(__m128i);
		cbTarget -= sizeof(__m128i);
	}

	while (cbTarget >= sizeof(std::uint32_t))
	{
		*reinterpret_cast<std::uint32_t*>(destination) = value32;
		destination += sizeof(std::uint32_t);
		cbTarget -= sizeof(std::uint32_t);
	}

	while (cbTarget-- > 0) *destination++ = static_cast<std::uint8_t>(value);

	_mm_sfence();

	return pTarget;
}

#pragma function(memcpy)
extern "C" void* __cdecl memcpy(_Out_writes_bytes_all_(size) void* destinationVoid, _In_reads_bytes_(size) void const* sourceVoid,
						  _In_ size_t size) // NOLINT(readability-identifier-naming)
{
	constexpr std::size_t unrollBytes = 4 * sizeof(__m128i);

	auto* destination = reinterpret_cast<std::uint8_t*>(destinationVoid);
	const auto* source = reinterpret_cast<const std::uint8_t*>(sourceVoid);

	std::size_t index = 0;

	while (index < size && (reinterpret_cast<std::uintptr_t>(destination + index) & 0xf) != 0)
	{
		destination[index] = source[index];
		index++;
	}

	const bool alignedSrc = (reinterpret_cast<std::uintptr_t>(source + index) & 15) == 0;

#ifdef STREAMING_MEMCPY
	for (; i + unrollBytes <= size; i += unrollBytes)
	{
		_mm_prefetch(reinterpret_cast<const char*>(src + i + unrollBytes), _MM_HINT_T0);

		_mm_stream_si128(reinterpret_cast<__m128i*>(destination + i + 0),
					  _mm_stream_load_si128(reinterpret_cast<const __m128i*>(src + i + 0)));
		_mm_stream_si128(reinterpret_cast<__m128i*>(destination + i + 16),
					  _mm_stream_load_si128(reinterpret_cast<const __m128i*>(src + i + 16)));
		_mm_stream_si128(reinterpret_cast<__m128i*>(destination + i + 32),
					  _mm_stream_load_si128(reinterpret_cast<const __m128i*>(src + i + 32)));
		_mm_stream_si128(reinterpret_cast<__m128i*>(destination + i + 48),
					  _mm_stream_load_si128(reinterpret_cast<const __m128i*>(src + i + 48)));
	}
	for (; i + sizeof(__m128i) <= size; i += sizeof(__m128i))
	{
		_mm_stream_si128(reinterpret_cast<__m128i*>(destination + i), _mm_stream_load_si128(reinterpret_cast<const __m128i*>(src + i)));
	}
#else
	const bool alignedDst = (reinterpret_cast<std::uintptr_t>(destination + index) & 15) == 0;

	if (alignedSrc && alignedDst)
	{
		for (; index + unrollBytes <= size; index += unrollBytes)
		{
			_mm_prefetch(reinterpret_cast<const char*>(source + index + unrollBytes), _MM_HINT_T0);

			_mm_store_si128(reinterpret_cast<__m128i*>(destination + index + 0),
						 _mm_load_si128(reinterpret_cast<const __m128i*>(source + index + 0)));
			_mm_store_si128(reinterpret_cast<__m128i*>(destination + index + 16),
						 _mm_load_si128(reinterpret_cast<const __m128i*>(source + index + 16)));
			_mm_store_si128(reinterpret_cast<__m128i*>(destination + index + 32),
						 _mm_load_si128(reinterpret_cast<const __m128i*>(source + index + 32)));
			_mm_store_si128(reinterpret_cast<__m128i*>(destination + index + 48),
						 _mm_load_si128(reinterpret_cast<const __m128i*>(source + index + 48)));
		}
		for (; index + sizeof(__m128i) <= size; index += sizeof(__m128i))
		{
			_mm_store_si128(reinterpret_cast<__m128i*>(destination + index),
						 _mm_load_si128(reinterpret_cast<const __m128i*>(source + index)));
		}
	}
	else
	{
		for (; index + unrollBytes <= size; index += unrollBytes)
		{
			_mm_storeu_si128(reinterpret_cast<__m128i*>(destination + index + 0),
						  _mm_loadu_si128(reinterpret_cast<const __m128i*>(source + index + 0)));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(destination + index + 16),
						  _mm_loadu_si128(reinterpret_cast<const __m128i*>(source + index + 16)));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(destination + index + 32),
						  _mm_loadu_si128(reinterpret_cast<const __m128i*>(source + index + 32)));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(destination + index + 48),
						  _mm_loadu_si128(reinterpret_cast<const __m128i*>(source + index + 48)));
		}
		for (; index + sizeof(__m128i) <= size; index += sizeof(__m128i))
		{
			_mm_storeu_si128(reinterpret_cast<__m128i*>(destination + index),
						  _mm_loadu_si128(reinterpret_cast<const __m128i*>(source + index)));
		}
	}
#endif

	for (; index + 4 <= size; index += 4)
		*reinterpret_cast<std::uint32_t*>(destination + index) = *reinterpret_cast<const std::uint32_t*>(source + index);
	for (; index < size; ++index) destination[index] = source[index];

	return destinationVoid;
}

extern "C" void* __cdecl memcpyAVX(_Out_writes_bytes_all_(size) void* destinationVoid, // NOLINT(readability-identifier-naming)
							_In_reads_bytes_(size) const void* sourceVoid, _In_ size_t size)
{
#ifdef __AVX__
	auto* destination = reinterpret_cast<std::uint8_t*>(destinationVoid);
	const auto* source = reinterpret_cast<const std::uint8_t*>(sourceVoid);

	std::size_t index = 0;

	while (index < size && (reinterpret_cast<std::uintptr_t>(destination + index) & 15) != 0)
	{
		destination[index] = source[index];
		index++;
	}

	const bool alignedSrc = (reinterpret_cast<std::uintptr_t>(source + index) & 15) == 0;

#ifdef STREAMING_MEMCPY
	if (alignedSrc)
	{
		for (; i + 128 <= size; i += 128)
		{
			_mm256_stream_si256(reinterpret_cast<__m256i*>(destination + i + 0),
							_mm256_load_si256(reinterpret_cast<const __m256i*>(source + i + 0)));
			_mm256_stream_si256(reinterpret_cast<__m256i*>(destination + i + 32),
							_mm256_load_si256(reinterpret_cast<const __m256i*>(source + i + 32)));
			_mm256_stream_si256(reinterpret_cast<__m256i*>(destination + i + 64),
							_mm256_load_si256(reinterpret_cast<const __m256i*>(source + i + 64)));
			_mm256_stream_si256(reinterpret_cast<__m256i*>(destination + i + 96),
							_mm256_load_si256(reinterpret_cast<const __m256i*>(source + i + 96)));
		}
		for (; i + 32 <= size; i += 32)
		{
			_mm256_stream_si256(reinterpret_cast<__m256i*>(destination + i),
							_mm256_load_si256(reinterpret_cast<const __m256i*>(source + i)));
		}
	}
	else
	{
		for (; i + 128 <= size; i += 128)
		{
			_mm256_stream_si256(reinterpret_cast<__m256i*>(destination + i + 0),
							_mm256_loadu_si256(reinterpret_cast<const __m256i*>(source + i + 0)));
			_mm256_stream_si256(reinterpret_cast<__m256i*>(destination + i + 32),
							_mm256_loadu_si256(reinterpret_cast<const __m256i*>(source + i + 32)));
			_mm256_stream_si256(reinterpret_cast<__m256i*>(destination + i + 64),
							_mm256_loadu_si256(reinterpret_cast<const __m256i*>(source + i + 64)));
			_mm256_stream_si256(reinterpret_cast<__m256i*>(destination + i + 96),
							_mm256_loadu_si256(reinterpret_cast<const __m256i*>(source + i + 96)));
		}
		for (; i + 32 <= size; i += 32)
		{
			_mm256_stream_si256(reinterpret_cast<__m256i*>(destination + i),
							_mm256_loadu_si256(reinterpret_cast<const __m256i*>(source + i)));
		}
	}
#else
	const bool alignedDst = (reinterpret_cast<std::uintptr_t>(destination + index) & 15) == 0;

	if (alignedSrc && alignedDst)
	{
		for (; index + 128 <= size; index += 128)
		{
			_mm256_store_si256(reinterpret_cast<__m256i*>(destination + index + 0),
						    _mm256_load_si256(reinterpret_cast<const __m256i*>(source + index + 0)));
			_mm256_store_si256(reinterpret_cast<__m256i*>(destination + index + 32),
						    _mm256_load_si256(reinterpret_cast<const __m256i*>(source + index + 32)));
			_mm256_store_si256(reinterpret_cast<__m256i*>(destination + index + 64),
						    _mm256_load_si256(reinterpret_cast<const __m256i*>(source + index + 64)));
			_mm256_store_si256(reinterpret_cast<__m256i*>(destination + index + 96),
						    _mm256_load_si256(reinterpret_cast<const __m256i*>(source + index + 96)));
		}
		for (; index + 32 <= size; index += 32)
		{
			_mm256_store_si256(reinterpret_cast<__m256i*>(destination + index),
						    _mm256_load_si256(reinterpret_cast<const __m256i*>(source + index)));
		}
	}
	else
	{
		for (; index + 128 <= size; index += 128)
		{
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(destination + index + 0),
							_mm256_loadu_si256(reinterpret_cast<const __m256i*>(source + index + 0)));
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(destination + index + 32),
							_mm256_loadu_si256(reinterpret_cast<const __m256i*>(source + index + 32)));
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(destination + index + 64),
							_mm256_loadu_si256(reinterpret_cast<const __m256i*>(source + index + 64)));
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(destination + index + 96),
							_mm256_loadu_si256(reinterpret_cast<const __m256i*>(source + index + 96)));
		}
		for (; index + 32 <= size; index += 32)
		{
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(destination + index),
							_mm256_loadu_si256(reinterpret_cast<const __m256i*>(source + index)));
		}
	}
#endif

	for (; index + 4 <= size; index += 4)
		*reinterpret_cast<std::uint32_t*>(destination + index) = *reinterpret_cast<const std::uint32_t*>(source + index);
	for (; index < size; ++index) destination[index] = source[index];

	return destination;
#else
	return memcpy(destinationVoid, sourceVoid, size);
#endif
}

#pragma function(strcmp)
extern "C" int __cdecl strcmp(_In_ const char* str1, _In_ const char* str2) // NOLINT(readability-identifier-naming)
{
	while (*str1 != '\0' && *str2 != '\0')
	{
		if (*str1 != *str2) return static_cast<int>(*str1) - static_cast<int>(*str2);

		++str1;
		++str2;
	}

	return static_cast<int>(*str1) - static_cast<int>(*str2);
}

#pragma function(memmove)
extern "C" void* __cdecl memmove(_Out_writes_bytes_all_(size) void* destinationVoid, _In_reads_bytes_(size) const void* sourceVoid,
						   _In_ size_t size)
{
	auto* destination = static_cast<std::uint8_t*>(destinationVoid);
	const auto* source = static_cast<const std::uint8_t*>(sourceVoid);

	if (destination == source || size == 0) return destinationVoid;

	if (destination < source || destination >= source + size)
	{
		// Forward copy (no overlap or destination before source)
		return memcpy(destination, source, size);
	}
	// Backward copy
	std::size_t index = size;

#ifdef STREAMING_MEMCPY
	while (i >= 64)
	{
		i -= 64;
		_mm_prefetch(reinterpret_cast<const char*>(src + i - 64), _MM_HINT_T0);

		_mm_stream_si128(reinterpret_cast<__m128i*>(destination + i + 0),
					  _mm_stream_load_si128(reinterpret_cast<const __m128i*>(src + i + 0)));
		_mm_stream_si128(reinterpret_cast<__m128i*>(destination + i + sizeof(__m128i)),
					  _mm_stream_load_si128(reinterpret_cast<const __m128i*>(src + i + sizeof(__m128i))));
		_mm_stream_si128(reinterpret_cast<__m128i*>(destination + i + 2 * sizeof(__m128i)),
					  _mm_stream_load_si128(reinterpret_cast<const __m128i*>(src + i + 2 * sizeof(__m128i))));
		_mm_stream_si128(reinterpret_cast<__m128i*>(destination + i + 3 * sizeof(__m128i)),
					  _mm_stream_load_si128(reinterpret_cast<const __m128i*>(src + i + 3 * sizeof(__m128i))));
	}
	while (i >= 16)
	{
		i -= 16;
		_mm_stream_si128(reinterpret_cast<__m128i*>(destination + i), _mm_stream_load_si128(reinterpret_cast<const __m128i*>(src + i)));
	}
#else
	const bool alignedSrc = (reinterpret_cast<std::uintptr_t>(source + index) & 15) == 0;
	const bool alignedDst = (reinterpret_cast<std::uintptr_t>(destination + index) & 15) == 0;

	if (alignedSrc && alignedDst)
	{
		while (index >= 64)
		{
			index -= 64;
			_mm_prefetch(reinterpret_cast<const char*>(source + index - 64), _MM_HINT_T0);

			_mm_store_si128(reinterpret_cast<__m128i*>(destination + index + 0),
						 _mm_load_si128(reinterpret_cast<const __m128i*>(source + index + 0)));
			_mm_store_si128(reinterpret_cast<__m128i*>(destination + index + 16),
						 _mm_load_si128(reinterpret_cast<const __m128i*>(source + index + 16)));
			_mm_store_si128(reinterpret_cast<__m128i*>(destination + index + 32),
						 _mm_load_si128(reinterpret_cast<const __m128i*>(source + index + 32)));
			_mm_store_si128(reinterpret_cast<__m128i*>(destination + index + 48),
						 _mm_load_si128(reinterpret_cast<const __m128i*>(source + index + 48)));
		}
		while (index >= 16)
		{
			index -= 16;
			_mm_store_si128(reinterpret_cast<__m128i*>(destination + index),
						 _mm_load_si128(reinterpret_cast<const __m128i*>(source + index)));
		}
	}
	else
	{
		while (index >= 64)
		{
			index -= 64;
			_mm_storeu_si128(reinterpret_cast<__m128i*>(destination + index + 0),
						  _mm_loadu_si128(reinterpret_cast<const __m128i*>(source + index + 0)));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(destination + index + 16),
						  _mm_loadu_si128(reinterpret_cast<const __m128i*>(source + index + 16)));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(destination + index + 32),
						  _mm_loadu_si128(reinterpret_cast<const __m128i*>(source + index + 32)));
			_mm_storeu_si128(reinterpret_cast<__m128i*>(destination + index + 48),
						  _mm_loadu_si128(reinterpret_cast<const __m128i*>(source + index + 48)));
		}
		while (index >= 16)
		{
			index -= 16;
			_mm_storeu_si128(reinterpret_cast<__m128i*>(destination + index),
						  _mm_loadu_si128(reinterpret_cast<const __m128i*>(source + index)));
		}
	}
#endif

	while (index >= 4)
	{
		index -= 4;
		*reinterpret_cast<std::uint32_t*>(destination + index) = *reinterpret_cast<const std::uint32_t*>(source + index);
	}
	while (index > 0)
	{
		--index;
		destination[index] = source[index];
	}

	return destination;
}
