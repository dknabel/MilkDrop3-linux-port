# Preset parser tests
add_executable(test_preset_parser test_preset_parser.cpp)

target_link_libraries(test_preset_parser
  gtest_main
  preset_parser
)

target_include_directories(test_preset_parser PRIVATE
  ${CMAKE_SOURCE_DIR}/src
)

add_test(NAME PresetParserTests COMMAND test_preset_parser)
