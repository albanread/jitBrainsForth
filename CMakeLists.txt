cmake_minimum_required(VERSION 3.10)

# Set the project name
project(jitBrainsForth VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS_RELEASE "-O0 -DNDEBUG")

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


)

add_custom_command(TARGET jitBrainsForth POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/start.f $<TARGET_FILE_DIR:jitBrainsForth>/start.f)

# Include directories for asmjit
target_include_directories(jitBrainsForth PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(jitBrainsForth PRIVATE ${PROJECT_SOURCE_DIR}/libs/libasmjit.dll.a)