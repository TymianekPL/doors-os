add_executable(Kernel
  Main/KernelMain.cpp
)           
enablePCH(Kernel)

target_compile_features(Kernel PUBLIC cxx_std_23)
target_include_directories(Kernel PRIVATE ${CMAKE_SOURCE_DIR}/Shared/Include)

string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-")

target_compile_options(Kernel PRIVATE                                                                    
  $<$<C_COMPILER_ID:MSVC>:/GR- /Gm- /GS- /Gs9999999 /Zl /Zc:externConstexpr /Zc:preprocessor>
  $<$<CXX_COMPILER_ID:MSVC>:/GR- /Gm- /GS- /Gs9999999 /Zl /Zc:externConstexpr /Zc:preprocessor>

  $<$<C_COMPILER_ID:Clang>:/GR- /Gm- /GS- /Gs9999999 /Zl>
  $<$<CXX_COMPILER_ID:Clang>:/GR- /Gm- /GS- /Gs9999999 /Zl>
)

target_link_options(Kernel PRIVATE
  $<$<C_COMPILER_ID:MSVC>:/SUBSYSTEM:NATIVE /MACHINE:X64 /ENTRY:KernelMain /NODEFAULTLIB /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:MSVCRTD>
  $<$<CXX_COMPILER_ID:MSVC>:/SUBSYSTEM:NATIVE /MACHINE:X64 /ENTRY:KernelMain /NODEFAULTLIB /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:MSVCRTD>
  $<$<C_COMPILER_ID:Clang>:/SUBSYSTEM:NATIVE /MACHINE:X64 /ENTRY:KernelMain /NODEFAULTLIB /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:MSVCRTD>
  $<$<CXX_COMPILER_ID:Clang>:/SUBSYSTEM:NATIVE /MACHINE:X64 /ENTRY:KernelMain /NODEFAULTLIB /NODEFAULTLIB:LIBCMT /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:MSVCRTD>
)

set_target_properties(Kernel PROPERTIES
  RUNTIME_OUTPUT_NAME "Kernel"
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/kernel
)

set_source_files_properties(KernelModule.ixx PROPERTIES LANGUAGE CXX)

string(REGEX REPLACE "/RTC[su0-9]*" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
