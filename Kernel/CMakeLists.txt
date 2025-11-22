add_executable(Kernel
  Main/KernelMain.cpp
)           
enablePCH(Kernel)

target_compile_features(Kernel PUBLIC cxx_std_23)
target_include_directories(Kernel PRIVATE ${CMAKE_SOURCE_DIR}/Shared/Include)

target_compile_options(Kernel PRIVATE                                                                    
  $<$<C_COMPILER_ID:MSVC>:/GR- /GS- /Gs9999999 /Zl /Zc:externConstexpr /Zc:preprocessor>
  $<$<CXX_COMPILER_ID:MSVC>:/GR- /GS- /Gs9999999 /Zl /Zc:externConstexpr /Zc:preprocessor>

  $<$<C_COMPILER_ID:Clang>:/GR- /GS- /Gs9999999 /Zl>
  $<$<CXX_COMPILER_ID:Clang>:/GR- /GS- /Gs9999999 /Zl>
)

target_link_options(Kernel PRIVATE
  $<$<C_COMPILER_ID:MSVC>:/SUBSYSTEM:NATIVE /MACHINE:X64 /ENTRY:KernelMain /NODEFAULTLIB>
  $<$<CXX_COMPILER_ID:MSVC>:/SUBSYSTEM:NATIVE /MACHINE:X64 /ENTRY:KernelMain /NODEFAULTLIB>
  $<$<C_COMPILER_ID:Clang>:/SUBSYSTEM:NATIVE /MACHINE:X64 /ENTRY:KernelMain /NODEFAULTLIB>
  $<$<CXX_COMPILER_ID:Clang>:/SUBSYSTEM:NATIVE /MACHINE:X64 /ENTRY:KernelMain /NODEFAULTLIB>
)

set_target_properties(Kernel PROPERTIES
  RUNTIME_OUTPUT_NAME "Kernel"
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/kernel
)

set_source_files_properties(KernelModule.ixx PROPERTIES LANGUAGE CXX)
