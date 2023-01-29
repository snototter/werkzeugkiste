#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/files/filesys.h>

#include <exception>

#include "../test_utils.h"

namespace wkc = werkzeugkiste::config;
namespace wkf = werkzeugkiste::files;

// NOLINTBEGIN

TEST(ConfigTest, LoadingToml) {
  const std::string fname1 =
      wkf::FullFile(wkf::DirName(__FILE__), "test-valid1.toml");
  const std::string fname2 =
      wkf::FullFile(wkf::DirName(__FILE__), "test-valid2.toml");

  auto config = wkc::Configuration::LoadTomlFile(fname1);

  EXPECT_THROW(wkc::Configuration::LoadTomlFile("this-does-not-exist.toml"),
               std::runtime_error);

  const std::string fname_invalid =
      wkf::FullFile(wkf::DirName(__FILE__), "test-invalid.toml");
  EXPECT_THROW(wkc::Configuration::LoadTomlFile(fname_invalid),
               std::runtime_error);
}

// NOLINTEND
