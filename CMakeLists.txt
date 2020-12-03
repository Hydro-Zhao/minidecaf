cmake_minimum_required(VERSION 3.10)
project(MiniDecaf)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Java)

include_directories(
    ${PROJECT_SOURCE_DIR}/grammar/
    ${PROJECT_SOURCE_DIR}/src/
	/usr/local/include/antlr4-runtime/
)

execute_process(
    WORKING_DIRECTORY
        ${PROJECT_SOURCE_DIR}/grammar/
    # Note that this command runs in pure shell
    COMMAND
        java -jar /usr/local/lib/antlr-4.8-complete.jar -Dlanguage=Cpp -no-listener -visitor MiniDecaf.g4
)

file(GLOB src
    ${PROJECT_SOURCE_DIR}/grammar/MiniDecafLexer.cpp
    ${PROJECT_SOURCE_DIR}/grammar/MiniDecafParser.cpp
    ${PROJECT_SOURCE_DIR}/grammar/MiniDecafVisitor.cpp
    ${PROJECT_SOURCE_DIR}/grammar/MiniDecafBaseVisitor.cpp
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_executable(MiniDecaf ${src})
target_link_libraries(MiniDecaf /usr/local/lib/libantlr4-runtime.so)

# Add custom target update_antlr, use `make update_antlr` to update your lexer & grammar rules anytime
add_custom_target(update_antlr
    WORKING_DIRECTORY
        ${PROJECT_SOURCE_DIR}/grammar/
    COMMAND 
        java -jar /usr/local/lib/antlr-4.8-complete.jar -Dlanguage=Cpp -no-listener -visitor MiniDecaf.g4
    COMMENT 
        "Auto updates for developing antlr grammar"
)