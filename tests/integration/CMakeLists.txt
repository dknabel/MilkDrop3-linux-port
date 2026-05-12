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

add_executable(test_real_presets
  test_real_presets.cpp
)

target_link_libraries(test_real_presets
  gtest_main
  visualization
  preset_parser
  expression_evaluator
)

target_include_directories(test_real_presets PRIVATE
  ${CMAKE_SOURCE_DIR}/src
)

add_test(NAME RealPresets COMMAND test_real_presets)

add_executable(test_application
  test_application.cpp
  ${CMAKE_SOURCE_DIR}/src/audio/audio_analyzer.cpp
)

target_include_directories(test_application PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(test_application
  gtest gtest_main
  ui
  visualization
  preset_parser
  expression_evaluator
  kissfft::kissfft
)

add_test(NAME Application COMMAND test_application)
