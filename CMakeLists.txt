cmake_minimum_required(VERSION 4.2)
project(obsl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(OBSL_BUILD_RUNTIME "Build the ObSL runtime executable" ON)

include(FetchContent)

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)

set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(nlohmann_json)

add_library(obsl STATIC
        src/ScriptEntry.cpp
        src/Lexer/Lexer.cpp
        src/Parser/Parser.cpp
        src/Interpreter/Interpreter.cpp
        src/Interpreter/Environment.cpp
        src/StdLib/StdLib.cpp
        src/GarbageCollector.cpp
        src/ScriptWorker.cpp
        src/ScriptRuntime.cpp
)

target_include_directories(obsl
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(obsl
        PRIVATE
        nlohmann_json::nlohmann_json
)

if (OBSL_BUILD_RUNTIME)
    add_executable(obsl_runtime
            src/ObSLCoreMain.cpp
    )
    target_link_libraries(obsl_runtime PRIVATE obsl)
endif ()