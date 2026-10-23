cmake_minimum_required(VERSION 3.21)
project(obsl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(OBSL_BUILD_RUNTIME "Build the ObSL runtime executable" ON)
option(OBSL_BUILD_FORMATTER "Build the ObSL formatter executable" ON)

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
        src/ASTSerializer.cpp
        src/ASTDeserializer.cpp
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


if (OBSL_BUILD_FORMATTER)
    add_executable(obsl_fmt
            src/Formatter/FmtMain.cpp
            src/Formatter/Formatter.cpp
            src/Formatter/Skeleton.cpp
    )
    target_link_libraries(obsl_fmt PRIVATE obsl)

    # Formatter smoke tests: every tests/fmt_fixtures/*.in must format to its
    # byte-exact *.out golden file, and the result must be idempotent.
    enable_testing()
    add_test(NAME obsl_fmt_fixture_list
             COMMAND ${CMAKE_COMMAND}
                     -DOBSL_FMT=$<TARGET_FILE:obsl_fmt>
                     -DFIXTURE_DIR=${CMAKE_SOURCE_DIR}/tests/fmt_fixtures
                     -DSKIP_IF_NOT_DIR=1
                     -P ${CMAKE_SOURCE_DIR}/tests/fmt_fixtures_test.cmake)
endif ()
