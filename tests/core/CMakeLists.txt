# Core module tests
add_subdirectory(preset)

# Profiler tests
add_executable(test_profiler test_profiler.cpp)

target_link_libraries(test_profiler
  gtest_main
  profiler
)

target_include_directories(test_profiler PRIVATE
  ${CMAKE_SOURCE_DIR}/src
)

add_test(NAME ProfilerTest COMMAND test_profiler)
