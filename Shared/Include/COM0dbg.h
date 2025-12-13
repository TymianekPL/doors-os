#pragma once

#ifndef CHECKED_BUILD
#warning "Cannot use this header in a non-checked build"
#else

#include <intrin.h>
#include <array>
#include <charconv>

namespace Testing
{
	constexpr std::uint16_t COM0Port = 0x3F8;

	inline void OutByteString(std::uint16_t port, unsigned char* value, std::size_t length)
	{
#ifdef __llvm__
		asm volatile("rep outsb" : : "d"(port), "S"(value), "c"(length) : "memory");
#else
		__outbytestring(port, value, length);
#endif
	}

	inline void ComWrite(char c)
	{
		while ((__inbyte(COM0Port + 5) & 0x20) == 0) _mm_pause();

		__outbyte(COM0Port, static_cast<unsigned char>(c));
	}
	inline void ComWrite(const char* str)
	{
		while (*str) ComWrite(*str++);
	}
	inline void ComWrite(const char* str, std::size_t count)
	{
		for (std::size_t i = 0; i < count; i++) ComWrite(str[i]);
	}
	template <std::size_t N>
	void ComWrite2(const char (&str)[N])
	{
		if constexpr (N <= 1) return;
		else if constexpr (N == 2)
		{
			while ((__inbyte(COM0Port + 5) & 0x20) == 0) _mm_pause();
			__outbyte(COM0Port, str[0]);
		}
		else if constexpr (N < 16)
		{
			while ((__inbyte(COM0Port + 5) & 0x20) == 0) _mm_pause();
			OutByteString(COM0Port, const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(str)), N - 1);
		}
		else if constexpr (((N - 1) & 15) == 0)
		{
			for (std::size_t i = 0; i < (N - 1) / 16; i++)
			{
				while ((__inbyte(COM0Port + 5) & 0x20) == 0) _mm_pause();
				OutByteString(COM0Port, const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(str + (i * 16))), 16);
			}
		}
		else
			for (std::size_t i = 0; i < N - 1; i++) ComWrite(str[i]);
	}

	inline void Initialise(void)
	{
		ComWrite2("@comdbg\n");
	}

	inline void IssueCommand(const char* command)
	{
		ComWrite('@');
		ComWrite(command);
		ComWrite('\n');
	}

	template <std::size_t N>
	inline void IssueFixedCommand(const char (&command)[N])
	{
		ComWrite('@');
		ComWrite2(command);
		ComWrite('\n');
	}

	template <std::integral TInteger>
	void ComWrite(TInteger value)
	{
		std::array<char, 32> buffer{};
		const auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
		ComWrite(buffer.data(), result.ptr - buffer.data());
	}

	template <typename... TArgs>
	void Print(const char* format, TArgs&&... args)
	{
		const char* ptr = format;
		std::tuple<TArgs...> tupleArgs(std::forward<TArgs>(args)...);
		std::size_t argIndex = 0;

		auto printArg = [&](std::size_t index, bool hex)
		{
			std::apply(
			    [&]<typename... T>(T&&... unpackedArgs)
			    {
				    std::size_t i = 0;
				    ((i++ == index ? (void)(
                [&]() {
                    using U = std::decay_t<T>;
                    if constexpr (std::integral<U>)
                    {
                        std::array<char, 32> buffer{};
                        const auto result = hex
                            ? std::to_chars(buffer.data(), buffer.data() + buffer.size(), unpackedArgs, 16)
                            : std::to_chars(buffer.data(), buffer.data() + buffer.size(), unpackedArgs);
                        ComWrite(buffer.data(), result.ptr - buffer.data());
                    }
                    else
                    {
                        ComWrite(unpackedArgs);
                    }
                }()
            ) : (void)0), ...);
			    },
			    tupleArgs);
		};

		while (*ptr)
		{
			if (*ptr == '{')
			{
				bool hex = false;
				++ptr;
				if (*ptr == ':')
				{
					++ptr;
					if (*ptr == 'x')
					{
						hex = true;
						++ptr;
					}
				}

				if (*ptr == '}')
				{
					printArg(argIndex++, hex);
					++ptr;
				}
				else
				{
					// malformed, just print literally
					ComWrite('{');
					if (hex) ComWrite(':');
				}
			}
			else { ComWrite(*ptr++); }
		}
	}
} // namespace Testing

#endif // ^^^ checked
