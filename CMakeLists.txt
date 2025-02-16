cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(jitBrainsForth VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# Link SDL2 library
find_library(SDL2_LIBRARY SDL2 c:/projects/SDL2/lib)
find_library(SDL2_MAIN_LIBRARY SDL2main c:/projects/SDL2/lib)
set(CMAKE_CXX_FLAGS_RELEASE "-O0 -s -DNDEBUG")

# Set compiler flags for release mode


# Add the executable
add_executable(jitBrainsForth
        main.cpp
        jitContext.cpp
        jitContext.h
        ForthDictionary.cpp
        ForthDictionary.h
        StackManager.h
        JitGenerator.h
        interpreter.h
        utility.h
        tests.h
        quit.cpp
        StringInterner.h
        Compiler.h
        CompilerUtility.h
        UtilitySDL.h
        jitLabels.h
)

# Copy the start.f file after build
add_custom_command(TARGET jitBrainsForth POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/start.f $<TARGET_FILE_DIR:jitBrainsForth>/start.f
)

# Include directories for asmjit
target_include_directories(jitBrainsForth
        PRIVATE ${PROJECT_SOURCE_DIR}/include
        PRIVATE c:/projects/SDL2/include/SDL2
        PRIVATE c:/projects/SDL2/include/SDL2/
        PRIVATE c:/projects/SDL2/include/
        PRIVATE c:/projects/SDL2/include)

# Link libraries (note: using PRIVATE for linking)
target_link_libraries(jitBrainsForth PRIVATE
        ${PROJECT_SOURCE_DIR}/libs/libasmjit.dll.a
        ${SDL2_LIBRARY} ${SDL2_MAIN_LIBRARY}

)