project(ortproxy LANGUAGES CXX VERSION 0.0.1.1)

add_subdirectory(dylib)

set(_src ortproxy.h shared_global.h)

if(WIN32)
    list(APPEND _src ortproxy_win.cpp)
else()
    list(APPEND _src ortproxy_unix.cpp)
endif()

add_library(${PROJECT_NAME} SHARED ${_src})

target_link_libraries(${PROJECT_NAME} PRIVATE dylib)

if(CONFIG_DISABLE_DEBUG)
    target_compile_definitions(${PROJECT_NAME} PUBLIC CONFIG_DISABLE_DEBUG)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME onnxruntime)

add_subdirectory(test)

if(WIN32)
    set(rc_name ${CMAKE_CURRENT_BINARY_DIR}/res.rc)

    # configure rc
    set(WIN32_EXPORT_NAME ${PROJECT_NAME})
    set(WIN32_COPYRIGHT_YEAR "2023")
    set(WIN32_AUTHOR_NAME "Sine Striker")
    set(WIN32_FILE_DESC "OnnxRuntime Proxy Library")
    set(WIN32_PRODUCT_NAME "${PROJECT_NAME}")
    configure_file(
        Win32DllRes.rc.in
        ${rc_name}
        @ONLY
    )
    target_sources(${PROJECT_NAME} PRIVATE ${rc_name})

    target_link_libraries(${PROJECT_NAME} PRIVATE shlwapi)
endif()

if(CMAKE_BUILD_TYPE MATCHES "(Release|MinSizeRel)")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}>
        ${CMAKE_CURRENT_SOURCE_DIR}/../csharp/TestSetSeed/libs
    )
endif()