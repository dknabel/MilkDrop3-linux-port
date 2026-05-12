# End-to-end performance tests
add_executable(e2e_performance_test test_e2e_performance.cpp)

target_link_libraries(e2e_performance_test
  gtest_main
  visualization
  profiler
)

target_include_directories(e2e_performance_test PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/include
)

add_test(NAME E2EPerformance COMMAND e2e_performance_test)

# Copy baseline.json to build directory for tests
configure_file(baseline.json baseline.json COPYONLY)

# Performance regression test
add_executable(regression_test
  test_regression.cpp
)

target_link_libraries(regression_test
  gtest_main
  profiler
  performance_baseline
)

target_include_directories(regression_test PRIVATE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/include
)

add_test(NAME RegressionTest COMMAND regression_test)
