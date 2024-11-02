cmake_minimum_required(VERSION 3.10)

# Set the project name
project(jitBrainsForth VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(jitBrainsForth
        main.cpp
        jitContext.cpp
        jitContext.h
        ForthDictionary.cpp
        ForthDictionary.h
        StackManager.cpp
        StackManager.h
        JitGenerator.h
        interpreter.cpp
        interpreter.h

)

# Include directories for asmjit
target_include_directories(jitBrainsForth PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(jitBrainsForth PRIVATE ${PROJECT_SOURCE_DIR}/libs/libasmjit.dll.a)