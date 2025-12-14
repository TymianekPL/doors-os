file(GLOB TEST_SOURCES "*.cpp")

foreach(src ${TEST_SOURCES})
     get_filename_component(name ${src} NAME_WE)

     add_executable(${name} ${src})   
     add_dependencies(${name} Kernel BootEFI)   
     enablePCH(${name})
     target_compile_features(${name} PRIVATE cxx_std_23)

     add_test(NAME ${name} COMMAND ${name})
endforeach()
