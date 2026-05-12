add_executable(test_preset_library
  test_preset_library.cpp
)

target_link_libraries(test_preset_library
  gtest_main
  preset_parser
  preset_validator
)

target_include_directories(test_preset_library PRIVATE
  ${CMAKE_SOURCE_DIR}/src
)

add_test(NAME PresetLibrary COMMAND test_preset_library)
