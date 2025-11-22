#pragma once
#include <Uefi.h>

#include <cstdint>
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

		~BootServices(void) { this->_bootServices->ExitBootServices(this->_imageHandle, this->_mapKey); }

	private:
		explicit BootServices(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
		    : _imageHandle(imageHandle), _systemTable(systemTable), _bootServices(this->_systemTable->BootServices)
		{
		}

		EFI_HANDLE _imageHandle;
		EFI_SYSTEM_TABLE* _systemTable;
		EFI_BOOT_SERVICES* _bootServices;
		UINTN _mapKey{};

		friend class Uefi;
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

	private:
		mutable bool _bootServicesAcquired{};
		EFI_HANDLE _imageHandle;
		EFI_SYSTEM_TABLE* _systemTable;
	};
} // namespace BootLoader
