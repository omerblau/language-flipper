cmake_minimum_required(VERSION 3.31)
project(first_cmake_project LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# ─── Statically link MinGW runtimes so no extra DLLs are needed ───
if (MINGW)
    # -static links the C runtime
    # -static-libgcc / -static-libstdc++ embed libgcc and libstdc++
    set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif ()

# ─── Your executable ───────────────────────────────────────────────
add_executable(first_cmake_project
        main.cpp
        utils.cpp
        utils.h    # headers don’t strictly need listing, but it’s harmless
        config.h
)
