#pragma once

#include <intrin.h>
#include <cstddef>
#include <cstdint>

namespace CPU
{
	template <std::size_t N>
	std::size_t ReadControlRegister(void) noexcept;

	template <std::size_t N>
	void WriteControlRegister(std::size_t value) noexcept;

	template <>
	std::size_t ReadControlRegister<0>(void) noexcept
	{
#ifdef __llvm__
		std::size_t value{};
		asm volatile("mov %%cr0, %0" : "=r"(value));
		return value;
#else
		return __readcr0();
#endif
	}

	template <>
	std::size_t ReadControlRegister<2>(void) noexcept
	{
#ifdef __llvm__
		std::size_t value{};
		asm volatile("mov %%cr2, %0" : "=r"(value));
		return value;
#else
		return __readcr2();
#endif
	}
	template <>
	std::size_t ReadControlRegister<3>(void) noexcept
	{
#ifdef __llvm__
		std::size_t value{};
		asm volatile("mov %%cr3, %0" : "=r"(value));
		return value;
#else
		return __readcr3();
#endif
	}
	template <>
	std::size_t ReadControlRegister<4>(void) noexcept
	{
#ifdef __llvm__
		std::size_t value{};
		asm volatile("mov %%cr4, %0" : "=r"(value));
		return value;
#else
		return __readcr4();
#endif
	}

	template <>
	void WriteControlRegister<0>(std::size_t value) noexcept
	{
#ifdef __llvm__
		asm volatile("mov %0, %%cr0" ::"r"(value) : "memory");
#else
		__writecr0(value);
#endif
	}

	template <>
	void WriteControlRegister<2>(std::size_t value) noexcept
	{
#ifdef __llvm__
		asm volatile("mov %0, %%cr2" ::"r"(value) : "memory");
#else
		__writecr2(value);
#endif
	}

	template <>
	void WriteControlRegister<3>(std::size_t value) noexcept
	{
#ifdef __llvm__
		asm volatile("mov %0, %%cr3" ::"r"(value) : "memory");
#else
		__writecr3(value);
#endif
	}

	template <>
	void WriteControlRegister<4>(std::size_t value) noexcept
	{
#ifdef __llvm__
		asm volatile("mov %0, %%cr4" ::"r"(value) : "memory");
#else
		__writecr4(value);
#endif
	}
} // namespace CPU
