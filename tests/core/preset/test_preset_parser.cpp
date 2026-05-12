#include <gtest/gtest.h>
#include "core/preset/preset_parser.h"

namespace milkdrop {

class PresetParserTest : public ::testing::Test {
protected:
  PresetParser parser;
};

// RED: Test empty preset parsing
TEST_F(PresetParserTest, ParseEmptyPreset) {
  std::string content = "[per_frame_eqs_0]\n";
  auto result = parser.parsePreset(content, "test.milk");

  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result->filename, "test.milk");
}

// RED: Test per-frame equations parsing
TEST_F(PresetParserTest, ParsePerFrameEquations) {
  std::string content = R"(
[per_frame_eqs_0]
zoom = 1.0 + sin(time) * 0.1;
rotation = time * 0.5;
)";

  auto result = parser.parsePreset(content, "test.milk");

  EXPECT_TRUE(result.has_value());
  EXPECT_FALSE(result->state.per_frame_eqs_code.empty());
  EXPECT_NE(result->state.per_frame_eqs_code.find("zoom"), std::string::npos);
}

// RED: Test warp shader code parsing
TEST_F(PresetParserTest, ParseWarpShaderCode) {
  std::string content = R"(
[warp_shader_code]
float warp = sin(uv.x * 10.0) * 0.1;
)";

  auto result = parser.parsePreset(content, "test.milk");

  EXPECT_TRUE(result.has_value());
  EXPECT_FALSE(result->warp_shader_code.empty());
}

// RED: Test shape parsing
TEST_F(PresetParserTest, ParseShapes) {
  std::string content = R"(
[shape_0_init]
x = 0.5;
y = 0.5;

[shape_0_per_frame]
x = 0.5 + 0.1 * sin(time);
)";

  auto result = parser.parsePreset(content, "test.milk");

  EXPECT_TRUE(result.has_value());
  EXPECT_TRUE(result->shapes.size() > 0);
  EXPECT_TRUE(result->shapes[0].enabled);
  EXPECT_FALSE(result->shapes[0].init_code.empty());
  EXPECT_FALSE(result->shapes[0].per_frame_code.empty());
}

// RED: Test wave parsing
TEST_F(PresetParserTest, ParseWaves) {
  std::string content = R"(
[wave_0_init]
y = 0.0;

[wave_0_per_point]
y = sample(index) * 0.5;
)";

  auto result = parser.parsePreset(content, "test.milk");

  EXPECT_TRUE(result.has_value());
  EXPECT_TRUE(result->waves.size() > 0);
  EXPECT_TRUE(result->waves[0].enabled);
  EXPECT_FALSE(result->waves[0].init_code.empty());
}

// RED: Test multiple shapes
TEST_F(PresetParserTest, ParseMultipleShapes) {
  std::string content = R"(
[shape_0_init]
x = 0.3;

[shape_1_init]
x = 0.7;

[shape_2_init]
x = 0.5;
)";

  auto result = parser.parsePreset(content, "test.milk");

  EXPECT_TRUE(result.has_value());
  EXPECT_TRUE(result->shapes.size() > 2);
  EXPECT_TRUE(result->shapes[0].enabled);
  EXPECT_TRUE(result->shapes[1].enabled);
  EXPECT_TRUE(result->shapes[2].enabled);
}

// RED: Test multiple waves
TEST_F(PresetParserTest, ParseMultipleWaves) {
  std::string content = R"(
[wave_0_init]
y = 0.0;

[wave_1_init]
y = 0.5;

[wave_2_init]
y = 1.0;
)";

  auto result = parser.parsePreset(content, "test.milk");

  EXPECT_TRUE(result.has_value());
  EXPECT_TRUE(result->waves.size() > 2);
  EXPECT_TRUE(result->waves[0].enabled);
  EXPECT_TRUE(result->waves[1].enabled);
  EXPECT_TRUE(result->waves[2].enabled);
}

// RED: Test comment filtering
TEST_F(PresetParserTest, FilterComments) {
  std::string content = R"(
[per_frame_eqs_0]
; This is a comment
zoom = 1.0;
; Another comment
rotation = 0.0;
)";

  auto result = parser.parsePreset(content, "test.milk");

  EXPECT_TRUE(result.has_value());
  EXPECT_NE(result->state.per_frame_eqs_code.find("zoom"), std::string::npos);
  // The code should not contain comment lines starting with semicolon
  EXPECT_EQ(result->state.per_frame_eqs_code.find("; This"), std::string::npos);
  EXPECT_EQ(result->state.per_frame_eqs_code.find("; Another"), std::string::npos);
  // But should contain the actual code lines
  EXPECT_NE(result->state.per_frame_eqs_code.find("rotation"), std::string::npos);
}

// RED: Test missing sections don't break parsing
TEST_F(PresetParserTest, MissingSectionsDontBreak) {
  std::string content = R"(
[per_frame_eqs_0]
zoom = 1.0;
)";

  auto result = parser.parsePreset(content, "test.milk");

  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result->warp_shader_code, "");
  EXPECT_EQ(result->shapes.size(), 16);
  EXPECT_FALSE(result->shapes[0].enabled);
}

}  // namespace milkdrop
