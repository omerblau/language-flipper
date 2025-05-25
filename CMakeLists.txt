cmake_minimum_required(VERSION 3.31)
project(language_flipper LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# ─── Statically link MinGW runtimes so no extra DLLs are needed ───
if (MINGW)
    # -static links the C runtime
    # -static-libgcc / -static-libstdc++ embed libgcc and libstdc++
    set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif ()

add_executable(language_flipper
        main.cpp
        utils.cpp
        utils.h
        config.h
)
