add_executable(BuildTool main.cpp )

add_dependencies(BuildTool Kernel BootEFI)

enablePCH(BuildTool)

add_custom_command(
    TARGET BuildTool POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/sources.list
        $<TARGET_FILE_DIR:BuildTool>/sources.list
    COMMENT "Copying sources.list to build directory"
)

add_custom_command(
    TARGET BuildTool POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/OVMF.fd
        $<TARGET_FILE_DIR:BuildTool>/OVMF.fd
    COMMENT "Copying OVMF to build directory"
)

set_target_properties(BuildTool PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)
