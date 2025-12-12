project(TELEIOS_TESTS)

# Define test source files
set(TEST_SOURCES
    test_main.c
    test_strings.c
    test_memory.c
    test_container.c
    test_number.c
    test_event.c
    test_logger.c
)

# Define test headers
set(TEST_HEADERS
    test_framework.h
)

# Define test executable
add_executable(teleios_test
    ${TEST_SOURCES}
    ${TEST_HEADERS}
)

# Set test properties
set_target_properties(teleios_test PROPERTIES
    C_STANDARD          11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS        OFF
)

# Link against engine library
target_link_libraries(teleios_test PRIVATE engine_lib)

# Include directories (using parent directory paths)
target_include_directories(teleios_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler flags for tests (inherit from engine_lib via PUBLIC)
if(MSVC)
    target_compile_options(teleios_test PRIVATE
        /W4                      # Warning level 4
        /std:c11                 # C11 standard
        /experimental:c11atomics # Enable C11 atomics support
    )
    target_link_options(teleios_test PRIVATE
        /SUBSYSTEM:CONSOLE
    )
endif()

# Copy application.yml to test directory
add_custom_command(TARGET teleios_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/sandbox/application.yml
        $<TARGET_FILE_DIR:teleios_test>/application.yml
    COMMENT "Copying application.yml to test directory"
)

# Register tests with CTest
add_test(NAME AllTests COMMAND teleios_test
    WORKING_DIRECTORY $<TARGET_FILE_DIR:teleios_test>
)

# Create custom target to run tests automatically after build
add_custom_target(run_tests ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --test-dir ${CMAKE_BINARY_DIR}
    DEPENDS teleios_test
    COMMENT "Running unit tests..."
)

# Ensure run_tests is executed after teleios_test is built
add_dependencies(run_tests teleios_test)
