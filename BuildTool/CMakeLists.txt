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
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/copy_ovmf.cmake
    COMMENT "Copying OVMF to build directory (if present)"
)

file(WRITE ${CMAKE_BINARY_DIR}/copy_ovmf.cmake
"if(EXISTS \"${CMAKE_SOURCE_DIR}/OVMF.fd\")
    execute_process(
        COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different
            \"${CMAKE_SOURCE_DIR}/OVMF.fd\"
            \"${CMAKE_BINARY_DIR}/OVMF.fd\"
    )
else()
    message(WARNING \"OVMF.fd not found, skipping copy.\")
endif()
")

set_target_properties(BuildTool PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)
