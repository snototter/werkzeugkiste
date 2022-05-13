#include <chrono>
#include <thread>
#include <string>

#include <gtest/gtest.h>

#include <werkzeugkiste/strings/strings.h>

namespace wks = werkzeugkiste::strings;

TEST(StringUtilsTest, Suffix) {
  EXPECT_TRUE(wks::EndsWith("Some string", "string"));
  EXPECT_FALSE(wks::EndsWith("Some string", "String"));  // case mismatch
  EXPECT_TRUE(wks::EndsWith("Some string", "ing"));
  EXPECT_TRUE(wks::EndsWith("Some string", "g"));  // single character string
  EXPECT_TRUE(wks::EndsWith("Some string", 'g'));  // character
  EXPECT_FALSE(wks::EndsWith("Some string", 'G'));  // character
  EXPECT_FALSE(wks::EndsWith("Some string", ""));  // empty string
  EXPECT_FALSE(wks::EndsWith("", ""));
  EXPECT_FALSE(wks::EndsWith("", "st"));
}


TEST(StringUtilsTest, Prefix) {
  EXPECT_TRUE(wks::StartsWith("Another test string", "Another "));
  EXPECT_FALSE(wks::StartsWith("Another test string", "another "));  // case mismatch
  EXPECT_TRUE(wks::StartsWith("Another test string", "An"));
  EXPECT_TRUE(wks::StartsWith("Another test string", "A"));  // single character string
  EXPECT_TRUE(wks::StartsWith("Another test string", 'A'));  // character
  EXPECT_FALSE(wks::StartsWith("Another test string", 'a'));  // character
  EXPECT_FALSE(wks::StartsWith("Another test string", ""));  // empty string
  EXPECT_FALSE(wks::StartsWith("", ""));
  EXPECT_FALSE(wks::StartsWith("", "A"));
}


TEST(StringUtilsTest, CaseConversion) {
  EXPECT_EQ(wks::Upper("abcdefghijklmnopqrstuvwxyz"),
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  EXPECT_EQ(wks::Upper("ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  EXPECT_EQ(wks::Upper("1234567890+*~#'-_.:,;´`\\?}=])[({/&%$§3!^°@<|>"),
            "1234567890+*~#'-_.:,;´`\\?}=])[({/&%$§3!^°@<|>");

  EXPECT_EQ(wks::Lower("abcdefghijklmnopqrstuvwxyz"),
            "abcdefghijklmnopqrstuvwxyz");
  EXPECT_EQ(wks::Lower("ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
            "abcdefghijklmnopqrstuvwxyz");
  EXPECT_EQ(wks::Lower("1234567890+*~#'-_.:,;´`\\?}=])[({/&%$§3!^°@<|>"),
            "1234567890+*~#'-_.:,;´`\\?}=])[({/&%$§3!^°@<|>");
}


TEST(StringUtilsTest, Trimming) {
  // Tab \t, Line feed \n, vertical tab \v, form feed \f, space, ...
  EXPECT_EQ(wks::Trim(" \t\r\n\v\fabc \t\r\n\v\f123 \t\r\n\v\f"),
            "abc \t\r\n\v\f123");

  EXPECT_EQ(wks::LTrim(" \t\r\n\v\fabc \t\r\n\v\f123 \t\r\n\v\f"),
            "abc \t\r\n\v\f123 \t\r\n\v\f");

  EXPECT_EQ(wks::RTrim(" \t\r\n\v\fabc \t\r\n\v\f123 \t\r\n\v\f"),
            " \t\r\n\v\fabc \t\r\n\v\f123");
}


TEST(StringUtilsTest, IsNumeric) {
  EXPECT_TRUE(wks::IsNumeric("0"));
  EXPECT_TRUE(wks::IsNumeric("+0"));
  EXPECT_TRUE(wks::IsNumeric("-0"));
  EXPECT_TRUE(wks::IsNumeric("1234"));
  EXPECT_TRUE(wks::IsNumeric("-3"));
  EXPECT_TRUE(wks::IsNumeric("+42"));
  EXPECT_FALSE(wks::IsNumeric("0!"));
  EXPECT_FALSE(wks::IsNumeric("a!"));

  EXPECT_FALSE(wks::IsNumeric(""));
  EXPECT_FALSE(wks::IsNumeric("-"));
  EXPECT_FALSE(wks::IsNumeric("+e"));
  EXPECT_FALSE(wks::IsNumeric("+e-7"));

  EXPECT_TRUE(wks::IsNumeric("10e3"));
  EXPECT_TRUE(wks::IsNumeric(".0"));
  EXPECT_TRUE(wks::IsNumeric("0.42"));
  EXPECT_TRUE(wks::IsNumeric("-2.3"));
  EXPECT_TRUE(wks::IsNumeric("1e-7"));
}


TEST(StringUtilsTest, Tokenize) {
  auto tokens = wks::Split("A;Line ;\tto;be;split ;;", ';');
  EXPECT_EQ(tokens.size(), 6);
  EXPECT_EQ(tokens[0], "A");
  EXPECT_EQ(tokens[1], "Line ");
  EXPECT_EQ(tokens[2], "\tto");
  EXPECT_EQ(tokens[3], "be");
  EXPECT_EQ(tokens[4], "split ");
  EXPECT_TRUE(tokens[5].empty());

  auto s = "Another;string;for;tokenization;";
  tokens = wks::Split(s, '!');
  EXPECT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0], s);
}


TEST(StringUtilsTest, Replace) {
  //TODO
  EXPECT_TRUE(wks::Replace("", "", "").empty());
  EXPECT_TRUE(wks::Replace("", "abc", "def").empty());
  EXPECT_TRUE(wks::Replace("", "", "def").empty());

  // Nothing changes if search string is empty
  EXPECT_EQ(wks::Replace("ABC123abc;:_", "", "!!!!!"),
            "ABC123abc;:_");

  // We can also remove substrings
  EXPECT_EQ(wks::Replace("ABC123abc;:_", "bc", ""),
            "ABC123a;:_");

  EXPECT_EQ(wks::Replace("ABC123abc;:_", "a", "!!"),
            "ABC123!!bc;:_");

  EXPECT_EQ(wks::Replace("ABC123abc;:_", "abcdef", "!!"),
            "ABC123abc;:_");

  EXPECT_EQ(wks::Replace("ABC123abc;:_", "BC", ""),
            "A123abc;:_");
}


TEST(StringUtilsTest, Remove) {
  //TODO
}


TEST(StringUtilsTest, URL) {
  //TODO
}


TEST(StringUtilsTest, Slug) {
  //TODO
  EXPECT_EQ(wks::Slug("nothing-to-be-slugged"),
            "nothing-to-be-slugged");

  EXPECT_EQ(wks::Slug(" replace:\tsome_spaces and underscores  _-"),
            "-replace-some-spaces-and-underscores----");

  EXPECT_EQ(wks::Slug(" \r\n\t\v\f"),
            "------");

  EXPECT_TRUE(wks::Slug(":#!").empty());
}
