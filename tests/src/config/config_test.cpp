#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/filesys.h>

#include <exception>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;
namespace wkf = werkzeugkiste::files;

// NOLINTBEGIN
TEST(ConfigTest, BasicAccessInt32) {
  const auto config = wkc::Configuration::LoadTomlString(R"toml(
    int32_1 = -123456
    int32_2 = +987654
    int32_3 = 2147483647  # Max value for int32
    int32_4 = -2147483647  # TODO
    )toml");
  EXPECT_EQ(-123456, config->GetInteger32("int32_1"));
  EXPECT_EQ(-1, config->GetInteger32OrDefault("test", -1));
  EXPECT_EQ(17, config->GetInteger32OrDefault("another", 17));
  EXPECT_THROW(config->GetInteger32("int32_0"), std::runtime_error);
}

TEST(ConfigTest, LoadingToml) {
  const std::string fname1 =
      wkf::FullFile(wkf::DirName(__FILE__), "test-valid1.toml");
  const std::string fname2 =
      wkf::FullFile(wkf::DirName(__FILE__), "test-valid2.toml");

  // Load valid TOML, then reload its string representation
  auto config = wkc::Configuration::LoadTomlFile(fname1);
  auto config_reloaded = wkc::Configuration::LoadTomlString(config->ToTOML());
  EXPECT_TRUE(config->Equals(config_reloaded.get()));
  EXPECT_TRUE(config_reloaded->Equals(config.get()));
  // Also the string representations should be equal
  EXPECT_EQ(config->ToTOML(), config_reloaded->ToTOML());

  EXPECT_THROW(wkc::Configuration::LoadTomlFile("this-does-not-exist.toml"),
               std::runtime_error);

  const std::string fname_invalid =
      wkf::FullFile(wkf::DirName(__FILE__), "test-invalid.toml");
  EXPECT_THROW(wkc::Configuration::LoadTomlFile(fname_invalid),
               std::runtime_error);
}

// NOLINTEND
