cmake_minimum_required(VERSION 3.31)
project(language_flipper LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# ─── Statically link MinGW runtimes so no extra DLLs are needed ───
if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif ()

add_executable(language_flipper
        main.cpp
        utils.cpp
        config.cpp
)

set_target_properties(language_flipper PROPERTIES OUTPUT_NAME "Language Flipper")

if (WIN32)
    target_sources(language_flipper PRIVATE app_icon.rc)
endif ()

add_custom_command(
        TARGET language_flipper POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/config.json
        $<TARGET_FILE_DIR:language_flipper>
)
