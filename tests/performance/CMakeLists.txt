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
