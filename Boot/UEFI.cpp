#include "UEFI.h"
#include <AtomicOperations.h>
#include <COM0dbg.h>
#include <Defn.h>
#include <Protocol\GraphicsOutput.h>
#include <intrin.h>
#include <memory>
#include <utility>

EFI_GUID gEfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

namespace BootLoader
{
	void Uefi::Reset(ResetType type, EfiStatus status)
	{
		this->_systemTable->RuntimeServices->ResetSystem(static_cast<EFI_RESET_TYPE>(std::to_underlying(type)), status, 0, nullptr);
		while (true) __halt();
	}

	void BasicPhysicalMemoryAllocator::Initialise(BootServices& bootServices)
	{
		UINTN memoryMapSize{};
		UINTN memoryMapKey{};
		UINTN descriptorSize{};
		UINT32 descriptorVersion{};
		std::unique_ptr<EFI_MEMORY_DESCRIPTOR[], BootServicesDeleter> efiMemoryMap{};
		EFI_STATUS status =
		    bootServices._bootServices->GetMemoryMap(&memoryMapSize, nullptr, &memoryMapKey, &descriptorSize, &descriptorVersion);

		if (status != EFI_BUFFER_TOO_SMALL) return;

		memoryMapSize += PageSize;
		// status = bootServices._bootServices->AllocatePool(EfiLoaderData, memoryMapSize, reinterpret_cast<void**>(&efiMemoryMap[0]));
		status = bootServices._bootServices->AllocatePool(EfiLoaderData, memoryMapSize, std::out_ptr(efiMemoryMap));
		if (EFI_ERROR(status))
		{
#ifdef CHECKED_BUILD
			Testing::Print("[AllocatePool] Error = {:x}", status);
#endif
			return;
		}

		status = bootServices._bootServices->GetMemoryMap(&memoryMapSize, efiMemoryMap.get(), &memoryMapKey, &descriptorSize,
												&descriptorVersion);
		if (EFI_ERROR(status))
		{
#ifdef CHECKED_BUILD
			Testing::Print("[GetMemoryMap] Error = {:x}", status);
#endif
			return;
		}

		const auto memorySize = GetUsableMemory(efiMemoryMap.get(), memoryMapSize, descriptorSize);

		this->bitmapSize = ((memorySize / PageSize) + CHAR_BIT - 1) / CHAR_BIT;
		this->bitmapSize = (this->bitmapSize + PageSize - 1) & ~(PageSize - 1);
		auto* descriptor = efiMemoryMap.get();
		for (UINTN i = 0; i < memoryMapSize / descriptorSize; ++i)
		{
			if (descriptor->Type == EfiConventionalMemory && descriptor->NumberOfPages * PageSize >= this->bitmapSize && !this->bitmap)
			{
				EFI_PHYSICAL_ADDRESS bitmapPhysical = descriptor->PhysicalStart;
				bootServices._bootServices->AllocatePages(AllocateAddress, EfiLoaderData, (bitmapSize + PageSize - 1) >> PageBits,
												  &bitmapPhysical);

				this->bitmap.reset(reinterpret_cast<std::uint8_t*>(bitmapPhysical));
				break;
			}

			descriptor = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(reinterpret_cast<std::uintptr_t>(descriptor) + descriptorSize);
		}

		if (!this->bitmap) return; // TODO: Fail

		__stosb(this->bitmap.get(), UCHAR_MAX, bitmapSize);

		const auto bitmapStart = reinterpret_cast<std::uintptr_t>(this->bitmap.get());
		const auto bitmapEnd = bitmapStart + this->bitmapSize;
		descriptor = efiMemoryMap.get();
		for (UINTN i = 0; i < memoryMapSize / descriptorSize; ++i)
		{
			if (descriptor->Type == EfiConventionalMemory)
			{
				const auto regionStart = descriptor->PhysicalStart;

				for (std::size_t pageIndex = 0; pageIndex < descriptor->NumberOfPages; pageIndex++)
				{
					const std::size_t pageStart = regionStart + pageIndex * PageSize;
					const std::size_t pageEnd = pageStart + PageSize;

					// Skip pages overlapping bitmap
					if (pageEnd <= bitmapStart || pageStart >= bitmapEnd)
					{
						const std::size_t pageIndex = pageStart / PageSize;
						if (pageIndex < this->bitmapSize) this->bitmap[pageIndex / CHAR_BIT] &= ~(1 << (pageIndex & (CHAR_BIT - 1)));
					}
				}
			}
			descriptor = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(reinterpret_cast<std::uintptr_t>(descriptor) + descriptorSize);

			this->bitmap[0] |= 1; // first page
		}
	}
	std::uintptr_t BasicPhysicalMemoryAllocator::AllocatePage(void)
	{
		std::uint64_t* vBitmap = reinterpret_cast<std::uint64_t*>(this->bitmap.get());

		const std::size_t numEntries = this->bitmapSize / sizeof(std::uint64_t);
		for (std::size_t i = 0; i < numEntries; i++)
		{
			if (vBitmap[i] != ~0ui64)
			{
				unsigned long index{};
				if (Atomic::BitScanForward(index, ~vBitmap[i]) != 0u)
				{
					std::uintptr_t page = (i * sizeof(std::uint64_t) * CHAR_BIT + index) * PageSize;
					vBitmap[i] |= (1ULL << index);
					return page;
				}
			}
		}

		return ~0;
	}
	void BasicPhysicalMemoryAllocator::FreePageByIndex(std::uintptr_t index)
	{
		std::size_t bitmapIndex = index / CHAR_BIT;
		std::size_t bit = index & (CHAR_BIT - 1);
		if (bitmapIndex >= this->bitmapSize) return;

		this->bitmap[bitmapIndex] &= ~(1 << bit);
	}
	void BasicPhysicalMemoryAllocator::FreePageByAddress(std::uintptr_t address)
	{
		std::size_t index = address >> PageBits;
		std::size_t bitmapIndex = index / CHAR_BIT;
		std::size_t bit = index & (CHAR_BIT - 1);
		if (bitmapIndex >= this->bitmapSize) return;

		this->bitmap[bitmapIndex] &= ~(1 << bit);
	}
	std::uint16_t BasicPhysicalMemoryAllocator::ReferencePage(std::uintptr_t address)
	{
		return ++(*this)[BootLoader::address, address].referenceCount;
	}
	std::uint16_t BasicPhysicalMemoryAllocator::DereferencePage(std::uintptr_t address)
	{
		const auto newCount = --(*this)[BootLoader::address, address].referenceCount;
		if (newCount != 0) return newCount;

		FreePageByAddress(address);
		return 0;
	}
	void Framebuffer::Initialise(BootServices& bootServices)
	{
		EFI_GRAPHICS_OUTPUT_PROTOCOL* graphicsOutput = nullptr;

		auto status = bootServices._bootServices->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, nullptr,
													  reinterpret_cast<void**>(&graphicsOutput));
		if (EFI_ERROR(status)) return; // TODO: Error

		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* modeInfo{};
		UINTN sizeOfModeInfo{};
		UINTN currentMode{};

		currentMode = graphicsOutput->Mode->Mode;
		status = graphicsOutput->QueryMode(graphicsOutput, static_cast<std::uint32_t>(currentMode), &sizeOfModeInfo, &modeInfo);
		if (EFI_ERROR(status)) return; // TODO: Error

		framebufferPhysicalAddress = graphicsOutput->Mode->FrameBufferBase;
		frameBufferSize = graphicsOutput->Mode->FrameBufferSize;
		horizontalResolution = modeInfo->HorizontalResolution;
		verticalResolution = modeInfo->VerticalResolution;

		this->framebuffer.reset(reinterpret_cast<std::uint32_t*>(framebufferPhysicalAddress));
	}
	void Framebuffer::SetPixel(std::uint32_t xPosition, std::uint32_t yPosition, Colour colour) const
	{
		framebuffer[yPosition * horizontalResolution + xPosition] = std::to_underlying(colour);
	}
	void Framebuffer::DrawRectangle(std::uint32_t xPosition, std::uint32_t yPosition, std::uint32_t width, std::uint32_t height,
							  Colour colour) const
	{
		for (std::uint32_t i = 0; i < height; i++)
		{
			for (std::uint32_t j = 0; j < width; j++) SetPixel(xPosition + j, yPosition + i, colour);
		}
	}
	void Framebuffer::Clear(Colour colour) const
	{
		for (std::uint32_t i = 0; i < this->verticalResolution; i++)
		{
			// who cares it's undefined, not my fault some people actually use long lol
			__stosd(reinterpret_cast<unsigned long*>(&framebuffer[i * this->horizontalResolution]), std::to_underlying(colour),
				   this->horizontalResolution);
		}
	}
} // namespace BootLoader
