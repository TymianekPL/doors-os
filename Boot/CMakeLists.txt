add_executable(BootEFI)

target_include_directories(BootEFI PRIVATE
    "${CMAKE_SOURCE_DIR}/extern/edk2/MdePkg/Include"
    "${CMAKE_SOURCE_DIR}/extern/edk2/MdePkg/Include/X64"
    "${CMAKE_SOURCE_DIR}/extern/edk2/MdePkg/Include/Uefi"
)

target_sources(BootEFI
    PUBLIC
       FILE_SET cxx_modules TYPE CXX_MODULES FILES
            BootModule.ixx
            UEFI.ixx
    PRIVATE
        BootMain.cpp
        UEFI.cpp
)

enablePCH(BootEFI)

string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-")

set_property(TARGET BootEFI PROPERTY IMPORTED_CXX_MODULES_COMPILE_FEATURES cxx_std_23)
target_compile_features(BootEFI PUBLIC cxx_std_23)
target_include_directories(BootEFI PRIVATE ${CMAKE_SOURCE_DIR}/Shared/Include)

target_compile_options(BootEFI PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/wd4162>
)

target_compile_options(BootEFI PRIVATE                                                                    
  $<$<C_COMPILER_ID:MSVC>:/GR- /GS- /Gs9999999 /Zl /Zc:externConstexpr /Zc:preprocessor>
  $<$<CXX_COMPILER_ID:MSVC>:/GR- /GS- /Gs9999999 /Zl /Zc:externConstexpr /Zc:preprocessor>

  $<$<C_COMPILER_ID:Clang>:/GR- /GS- /Gs9999999 /Zl>
  $<$<CXX_COMPILER_ID:Clang>:/GR- /GS- /Gs9999999 /Zl>
)

target_link_options(BootEFI PRIVATE
  $<$<C_COMPILER_ID:MSVC>:/SUBSYSTEM:EFI_APPLICATION /MACHINE:X64 /MANIFEST:NO /DYNAMICBASE:NO /ENTRY:EfiEntry /NODEFAULTLIB>
  $<$<CXX_COMPILER_ID:MSVC>:/SUBSYSTEM:EFI_APPLICATION /MACHINE:X64 /MANIFEST:NO /DYNAMICBASE:NO /ENTRY:EfiEntry /NODEFAULTLIB>
  $<$<C_COMPILER_ID:Clang>:/SUBSYSTEM:EFI_APPLICATION /MACHINE:X64 /MANIFEST:NO /DYNAMICBASE:NO /ENTRY:EfiEntry /NODEFAULTLIB>
  $<$<CXX_COMPILER_ID:Clang>:/SUBSYSTEM:EFI_APPLICATION /MACHINE:X64 /MANIFEST:NO /DYNAMICBASE:NO /ENTRY:EfiEntry /NODEFAULTLIB>
)

set_target_properties(BootEFI PROPERTIES
    LINK_FLAGS "/MANIFEST:NO /SUBSYSTEM:EFI_APPLICATION"
)

set_target_properties(BootEFI PROPERTIES
  RUNTIME_OUTPUT_NAME "BOOTx64"
  SUFFIX ".EFI"
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/boot     
  MSVC_ENABLE_EXCEPTIONS OFF
)

