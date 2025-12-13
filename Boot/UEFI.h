#pragma once
#include <Uefi.h>

#include <Defn.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

namespace BootLoader
{
	enum struct ResetType : std::uint8_t
	{
		Cold = EfiResetCold,
		Warm = EfiResetWarm,
		Shutdown = EfiResetShutdown
	};

	using EfiStatus = EFI_STATUS;

	struct BootServices
	{
		explicit BootServices(void) = delete;
		BootServices(const BootServices&) = delete;
		BootServices(BootServices&&) = delete;
		BootServices& operator=(const BootServices&) = delete;
		BootServices& operator=(BootServices&&) = delete;

		void Print(const char16_t* string) const noexcept
		{
			this->_systemTable->ConOut->OutputString(this->_systemTable->ConOut,
											 reinterpret_cast<CHAR16*>(const_cast<char16_t*>(string)));
		}
		void Print(const wchar_t* string) const noexcept
		{
			this->_systemTable->ConOut->OutputString(this->_systemTable->ConOut,
											 reinterpret_cast<CHAR16*>(const_cast<wchar_t*>(string)));
		}
		void Print(const CHAR16* string) const noexcept
		{
			this->_systemTable->ConOut->OutputString(this->_systemTable->ConOut, const_cast<CHAR16*>(string));
		}

		~BootServices(void)
		{
			this->_bootServices->ExitBootServices(this->_imageHandle, this->_mapKey);
			BootServices::_bootServicesReference = nullptr;
		}

	private:
		explicit BootServices(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
		    : _imageHandle(imageHandle), _systemTable(systemTable), _bootServices(this->_systemTable->BootServices)
		{
			BootServices::_bootServicesReference = this->_bootServices;
		}

		EFI_HANDLE _imageHandle;
		EFI_SYSTEM_TABLE* _systemTable;
		EFI_BOOT_SERVICES* _bootServices;
		UINTN _mapKey{};

		inline static EFI_BOOT_SERVICES* _bootServicesReference;

		friend class Uefi;
		friend struct BasicPhysicalMemoryAllocator;
		friend struct BootServicesDeleter;
		friend struct Framebuffer;
	};

	struct EmptyDeleter
	{
		constexpr static void operator()(void*) noexcept {} // NOLINT(readability-named-parameter)
	};

	struct PFNIndexTag
	{
	};
	struct PFNAddressTag
	{
	};
	constexpr inline PFNIndexTag index{};
	constexpr inline PFNAddressTag address{};

	struct BasicPhysicalMemoryAllocator
	{
		std::unique_ptr<std::uint8_t[], EmptyDeleter> bitmap = nullptr;
		std::unique_ptr<MMPFNEntry[], EmptyDeleter> pfnDatabase = nullptr;
		std::size_t bitmapSize = 0;

		void Initialise(BootServices& bootServices);
		std::uintptr_t AllocatePage(void);
		void FreePageByIndex(std::uintptr_t index);
		void FreePageByAddress(std::uintptr_t address);

		void MarkPFNIndex(const std::uintptr_t page, const PFNUse use, const PFNRegion region)
		{
			this->pfnDatabase[page].referenceCount = std::to_underlying(region) & 1;
			this->pfnDatabase[page].use = use;
			this->pfnDatabase[page].region = region;
		}
		void MarkPFNAddress(const std::uintptr_t address, const PFNUse use, const PFNRegion region)
		{
			const auto page = address >> PageBits;
			this->pfnDatabase[page].referenceCount = std::to_underlying(region) & 1;
			this->pfnDatabase[page].use = use;
			this->pfnDatabase[page].region = region;
		}

		MMPFNEntry& operator[](PFNIndexTag, const std::uintptr_t index) noexcept { return this->pfnDatabase[index]; }
		MMPFNEntry& operator[](PFNAddressTag, const std::uintptr_t address) noexcept { return this->pfnDatabase[address]; }

		MMPFNEntry operator[](PFNIndexTag, const std::uintptr_t index) const noexcept { return this->pfnDatabase[index]; }
		MMPFNEntry operator[](PFNAddressTag, const std::uintptr_t address) const noexcept { return this->pfnDatabase[address]; }
		std::uint16_t ReferencePage(std::uintptr_t address);
		std::uint16_t DereferencePage(std::uintptr_t address);

	private:
		[[nodiscard]] static std::uint64_t GetUsableMemory(EFI_MEMORY_DESCRIPTOR* memoryMap, std::size_t mapSize,
												 std::size_t descriptorSize) noexcept
		{
			std::uint64_t total{};
			for (std::size_t i = 0; i < mapSize / descriptorSize; i++)
			{
				EFI_MEMORY_DESCRIPTOR* descriptor =
				    reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(reinterpret_cast<std::byte*>(memoryMap) + (i * descriptorSize));

				if (descriptor->Type == EfiConventionalMemory || descriptor->Type == EfiPersistentMemory)
					total += descriptor->NumberOfPages * PageSize;
			}
			return total;
		}
	};

	struct Framebuffer
	{
		std::uintptr_t framebufferPhysicalAddress{};
		std::unique_ptr<std::uint32_t[], EmptyDeleter> framebuffer{};
		std::size_t frameBufferSize{};
		std::size_t horizontalResolution{};
		std::size_t verticalResolution{};

		explicit Framebuffer(void) = default;
		void Initialise(BootServices& bootServices);
		void SetPixel(std::uint32_t xPosition, std::uint32_t yPosition, Colour colour) const;
		void DrawRectangle(std::uint32_t xPosition, std::uint32_t yPosition, std::uint32_t width, std::uint32_t height,
					    Colour colour) const;
		void Clear(Colour colour) const;
	};

	struct BootServicesDeleter
	{
		void operator()(void* buffer) { BootServices::_bootServicesReference->FreePool(buffer); }
	};

	class Uefi
	{
	public:
		explicit Uefi(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) : _imageHandle(ImageHandle), _systemTable(SystemTable) {}

		[[noreturn]] void Reset(ResetType type, EfiStatus status);

		[[nodiscard]] BootServices AcquireBootServices(void) const noexcept
		{
			if (std::exchange(this->_bootServicesAcquired, true))
			{
				// TODO: Error
			}
			return BootServices{this->_imageHandle, this->_systemTable};
		}

		Framebuffer framebuffer{};			  // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)
		BasicPhysicalMemoryAllocator allocator{}; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)

	private:
		mutable bool _bootServicesAcquired{};
		EFI_HANDLE _imageHandle;
		EFI_SYSTEM_TABLE* _systemTable;
	};
} // namespace BootLoader
