#include <werkzeugkiste/strings/strings.h>

#include <chrono>
#include <string>
#include <string_view>
#include <thread>

#include "../test_utils.h"

namespace wks = werkzeugkiste::strings;
using namespace std::string_view_literals;

// NOLINTBEGIN

TEST(StringUtilsTest, Suffix) {
  EXPECT_TRUE(wks::EndsWith("Some string"sv, "string"sv));
  EXPECT_FALSE(wks::EndsWith("Some string"sv, "String"sv));  // case mismatch
  EXPECT_TRUE(wks::EndsWith("Some string"sv, "ing"sv));
  EXPECT_TRUE(
      wks::EndsWith("Some string"sv, "g"sv));         // single character string
  EXPECT_TRUE(wks::EndsWith("Some string"sv, 'g'));   // character
  EXPECT_FALSE(wks::EndsWith("Some string"sv, 'G'));  // character
  EXPECT_FALSE(wks::EndsWith("Some string"sv, ""sv));  // empty string
  EXPECT_FALSE(wks::EndsWith(""sv, ""sv));
  EXPECT_FALSE(wks::EndsWith(""sv, "st"sv));
}

TEST(StringUtilsTest, Prefix) {
  EXPECT_TRUE(wks::StartsWith("Another test string"sv, "Another "sv));
  // Case mismatch:
  EXPECT_FALSE(wks::StartsWith("Another test string"sv, "another "sv));
  EXPECT_TRUE(wks::StartsWith("Another test string"sv, "An"sv));
  // Single character string vs. characters:
  EXPECT_TRUE(wks::StartsWith("Another test string"sv, "A"sv));
  EXPECT_TRUE(wks::StartsWith("Another test string"sv, 'A'));
  EXPECT_FALSE(wks::StartsWith("Another test string"sv, 'a'));

  EXPECT_FALSE(wks::StartsWith("Another test string"sv, ""sv));
  EXPECT_FALSE(wks::StartsWith(""sv, ""sv));
  EXPECT_FALSE(wks::StartsWith(""sv, "A"sv));
}

TEST(StringUtilsTest, CaseConversion) {
  EXPECT_EQ(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ", wks::Upper("abcdefghijklmnopqrstuvwxyz"));
  EXPECT_EQ(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ", wks::Upper("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
  EXPECT_EQ("1234567890+*~#'-_.:,;´`\\?}=])[({/&%$§3!^°@<|>",
      wks::Upper("1234567890+*~#'-_.:,;´`\\?}=])[({/&%$§3!^°@<|>"));

  EXPECT_EQ(
      "abcdefghijklmnopqrstuvwxyz", wks::Lower("abcdefghijklmnopqrstuvwxyz"));
  EXPECT_EQ(
      "abcdefghijklmnopqrstuvwxyz", wks::Lower("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
  EXPECT_EQ("1234567890+*~#'-_.:,;´`\\?}=])[({/&%$§3!^°@<|>",
      wks::Lower("1234567890+*~#'-_.:,;´`\\?}=])[({/&%$§3!^°@<|>"));
}

TEST(StringUtilsTest, Trimming) {
  // Tab \t, Line feed \n, vertical tab \v, form feed \f, space, ...
  EXPECT_EQ("abc \t\r\n\v\f123",
      wks::Trim(" \t\r\n\v\fabc \t\r\n\v\f123 \t\r\n\v\f"));

  EXPECT_EQ("abc \t\r\n\v\f123 \t\r\n\v\f",
      wks::LTrim(" \t\r\n\v\fabc \t\r\n\v\f123 \t\r\n\v\f"));

  EXPECT_EQ(" \t\r\n\v\fabc \t\r\n\v\f123",
      wks::RTrim(" \t\r\n\v\fabc \t\r\n\v\f123 \t\r\n\v\f"));
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
  EXPECT_FALSE(wks::IsNumeric("123 456"));

  EXPECT_TRUE(wks::IsNumeric("1234567890000000000000000000"));
  EXPECT_TRUE(wks::IsNumeric("12345678900000000000000000000000000000"));
  EXPECT_TRUE(wks::IsNumeric("12345678900000000000000000000000000000.123456"));

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

TEST(StringUtilsTest, IsInteger) {
  EXPECT_TRUE(wks::IsInteger("1"sv));
  EXPECT_TRUE(wks::IsInteger("+1"sv));
  EXPECT_TRUE(wks::IsInteger("-1"sv));
  EXPECT_TRUE(wks::IsInteger("0"sv));
  EXPECT_TRUE(wks::IsInteger("+0"sv));
  EXPECT_TRUE(wks::IsInteger("-0"sv));

  EXPECT_TRUE(wks::IsInteger("+123456789"sv));
  EXPECT_TRUE(wks::IsInteger("-123456789"sv));

  EXPECT_FALSE(wks::IsInteger("0."));
  EXPECT_FALSE(wks::IsInteger(".0"));
  EXPECT_FALSE(wks::IsInteger("0.0"));
  EXPECT_FALSE(wks::IsInteger("+0.0"));
  EXPECT_FALSE(wks::IsInteger("-0.0"));

  EXPECT_FALSE(wks::IsInteger("1.2"));
  EXPECT_FALSE(wks::IsInteger("-1.2"));
  EXPECT_FALSE(wks::IsInteger("1e3"));
  EXPECT_FALSE(wks::IsInteger("+1e3"));

  EXPECT_FALSE(wks::IsInteger("test"));
  EXPECT_FALSE(wks::IsInteger("a1"));
  EXPECT_FALSE(wks::IsInteger("!3"));
}

TEST(StringUtilsTest, Tokenize) {
  auto tokens = wks::Split("A;Line ;\tto;be;split ;;", ';');
  EXPECT_EQ(6, tokens.size());
  EXPECT_EQ("A", tokens[0]);
  EXPECT_EQ("Line ", tokens[1]);
  EXPECT_EQ("\tto", tokens[2]);
  EXPECT_EQ("be", tokens[3]);
  EXPECT_EQ("split ", tokens[4]);
  EXPECT_TRUE(tokens[5].empty());

  auto s = "Another;string;for;tokenization;";
  tokens = wks::Split(s, '!');
  EXPECT_EQ(1, tokens.size());
  EXPECT_EQ(s, tokens[0]);

  // Split
  auto tokens_spl = wks::Split("a-b", '-');
  EXPECT_EQ(2, tokens_spl.size());
  EXPECT_EQ("a", tokens_spl[0]);
  EXPECT_EQ("b", tokens_spl[1]);
  // `Split` skips the FINAL empty token
  tokens_spl = wks::Split("a-b-", '-');
  EXPECT_EQ(2, tokens_spl.size());
  EXPECT_EQ("a", tokens_spl[0]);
  EXPECT_EQ("b", tokens_spl[1]);
  // But *only* the last
  tokens_spl = wks::Split("-a--b--", '-');
  EXPECT_EQ(5, tokens_spl.size());
  EXPECT_TRUE(tokens_spl[0].empty());
  EXPECT_EQ("a", tokens_spl[1]);
  EXPECT_TRUE(tokens_spl[2].empty());
  EXPECT_EQ("b", tokens_spl[3]);
  EXPECT_TRUE(tokens_spl[4].empty());

  // Tokenize
  auto tokens_tok = wks::Tokenize("a-b", "-");
  EXPECT_EQ(2, tokens_tok.size());
  EXPECT_EQ("a", tokens_tok[0]);
  EXPECT_EQ("b", tokens_tok[1]);
  // `Tokenize` skips ALL empty tokens
  tokens_tok = wks::Tokenize("a-b-", "-");
  EXPECT_EQ(2, tokens_tok.size());
  EXPECT_EQ("a", tokens_tok[0]);
  EXPECT_EQ("b", tokens_tok[1]);

  tokens_tok = wks::Tokenize("-a--b--", "-");
  EXPECT_EQ(2, tokens_tok.size());
  EXPECT_EQ("a", tokens_tok[0]);
  EXPECT_EQ("b", tokens_tok[1]);

  // Beware of the different behaviors:
  tokens_spl = wks::Split("1.2.3", '.');
  tokens_tok = wks::Tokenize("1.2.3", ".");
  EXPECT_EQ(3, tokens_spl.size()) << Stringify(tokens_spl);
  EXPECT_EQ(3, tokens_tok.size()) << Stringify(tokens_tok);

  tokens_spl = wks::Split("1..2.3", '.');
  tokens_tok = wks::Tokenize("1..2.3", ".");
  EXPECT_EQ(4, tokens_spl.size()) << Stringify(tokens_spl);
  EXPECT_EQ(3, tokens_tok.size()) << Stringify(tokens_tok);

  tokens_spl = wks::Split(".1.2.3", '.');
  tokens_tok = wks::Tokenize(".1.2.3", ".");
  EXPECT_EQ(4, tokens_spl.size()) << Stringify(tokens_spl);
  EXPECT_EQ(3, tokens_tok.size()) << Stringify(tokens_tok);

  tokens_spl = wks::Split("1.2.3.", '.');
  tokens_tok = wks::Tokenize("1.2.3.", ".");
  EXPECT_EQ(3, tokens_spl.size()) << Stringify(tokens_spl);
  EXPECT_EQ(3, tokens_tok.size()) << Stringify(tokens_tok);

  tokens_spl = wks::Split("1.2.3..", '.');
  tokens_tok = wks::Tokenize("1.2.3..", ".");
  EXPECT_EQ(4, tokens_spl.size()) << Stringify(tokens_spl);
  EXPECT_EQ(3, tokens_tok.size()) << Stringify(tokens_tok);

  tokens_spl = wks::Split("1.2.3.4", '.');
  tokens_tok = wks::Tokenize("1.2.3.4", ".");
  EXPECT_EQ(4, tokens_spl.size()) << Stringify(tokens_spl);
  EXPECT_EQ(4, tokens_tok.size()) << Stringify(tokens_tok);
}

TEST(StringUtilsTest, Replace) {
  EXPECT_TRUE(wks::Replace("", "", "").empty());
  EXPECT_TRUE(wks::Replace("", "abc", "def").empty());
  EXPECT_TRUE(wks::Replace("", "", "def").empty());

  // Nothing changes if search string is empty
  EXPECT_EQ("ABC123abc;:_", wks::Replace("ABC123abc;:_", "", "!!!!!"));

  EXPECT_EQ("ABC123!!bc;:_", wks::Replace("ABC123abc;:_", "a", "!!"));

  EXPECT_EQ("ABC123abc;:_", wks::Replace("ABC123abc;:_", "abcdef", "!!"));

  EXPECT_EQ("A123abc;:_", wks::Replace("ABC123abc;:_", "BC", ""));

  // Nothing changes if replacement string is search string
  EXPECT_EQ("abacad", wks::Replace("abacad", 'a', 'a'));
  EXPECT_EQ("abacad", wks::Replace("abacad", "a", "a"));
  EXPECT_EQ("\\\"quotes\\\"", wks::Replace("\"quotes\"", "\"", "\\\""));

  // All occurrences should be replaced
  EXPECT_EQ("A..123abc123A..123abc123",
      wks::Replace("ABC123abc123ABC123abc123", "BC", ".."));

  // Use 'Replace' to 'Remove' a substring
  EXPECT_EQ("ABC123a;:_", wks::Replace("ABC123abc;:_", "bc", ""));

  // Also, the character-only version should be tested
  EXPECT_EQ("ABC1230bc;:_", wks::Replace("ABC123abc;:_", 'a', '0'));

  EXPECT_EQ("!BC1!3abc;:_", wks::Replace("ABC1A3abc;:_", 'A', '!'));
}

TEST(StringUtilsTest, Remove) {
  EXPECT_EQ("1234567890+*~#'-_.:,;´`?}=])[({/&%$§3!^°@<|>",
      wks::Remove("1234567890+*~#'-_.:,;´`\\?}=])[({/&%$§3!^°@<|>", '\\'));

  EXPECT_EQ("bcDEFghiABCdefGHIbc", wks::Remove("abcDEFghiABCdefGHIabc", 'a'));

  EXPECT_EQ("cDEFghiABdefGHIc",
      wks::Remove("abcDEFghiABCdefGHIabc", {'a', 'b', 'C'}));
}

TEST(StringUtilsTest, URL) {
  // Simplistic URL parsing (downstream I need to be able
  // to distinguish web URLs from file paths, e.g. to properly
  // load a camera's SDP description)
  std::string protocol, remainder;
  EXPECT_TRUE(wks::GetUrlProtocol("file://foo.txt", protocol, remainder));
  EXPECT_EQ("file://", protocol);
  EXPECT_EQ("foo.txt", remainder);

  EXPECT_TRUE(wks::GetUrlProtocol(
      "UnChecked://SomeU.R.I:?asdf=foo", protocol, remainder));
  EXPECT_EQ("UnChecked://", protocol);
  EXPECT_EQ("SomeU.R.I:?asdf=foo", remainder);

  EXPECT_FALSE(wks::GetUrlProtocol("foo.txt", protocol, remainder));
  EXPECT_TRUE(protocol.empty());
  EXPECT_EQ("foo.txt", remainder);

  // When logging connection strings, I want to hide
  // any potential authentication information (but still
  // know that it was actually provided in the URL string):
  EXPECT_EQ("file://foobar", wks::ObscureUrlAuthentication("file://foobar"));

  EXPECT_EQ("http://<auth>@foo.bar",
      wks::ObscureUrlAuthentication("http://user:pass@foo.bar"));

  EXPECT_EQ("rtsp://<auth>@foo.bar:12345",
      wks::ObscureUrlAuthentication("rtsp://user:pass@foo.bar:12345"));

  EXPECT_EQ("https://<auth>@192.168.0.1:8080/cam.cgi",
      wks::ObscureUrlAuthentication("https://user@192.168.0.1:8080/cam.cgi"));

  EXPECT_EQ("<auth>@some.thing:12345",
      wks::ObscureUrlAuthentication("user:pass@some.thing:12345"));

  // If we want to strip the subpaths and parameters of a URL:
  EXPECT_EQ("https://<auth>@192.168.0.1:8080",
      wks::ClipUrl("https://root@192.168.0.1:8080/cam.cgi"));

  EXPECT_EQ("https://192.168.0.1:8080",
      wks::ClipUrl("https://192.168.0.1:8080?image=still&overlay=off"));

  EXPECT_EQ("file:///a/file/needs/special/handling.txt",
      wks::ClipUrl("file:///a/file/needs/special/handling.txt"));

  EXPECT_EQ(
      "<auth>@192.168.0.1:8080", wks::ClipUrl("root@192.168.0.1:8080/cam.cgi"));

  EXPECT_EQ("smb://<auth>@192.168.0.1/some/share",
      wks::ClipUrl("smb://root@192.168.0.1/some/share"));
}

TEST(StringUtilsTest, Indent) {
  EXPECT_EQ("   ", wks::Indent(""sv, 3, ' '));
  EXPECT_EQ("", wks::Indent(""sv, 0, ' '));

  EXPECT_EQ(" Foo", wks::Indent("Foo"sv, 1, ' '));
  EXPECT_EQ("**Foo", wks::Indent("Foo"sv, 2, '*'));
}

TEST(StringUtilsTest, Slug) {
  EXPECT_EQ("nothing-to-be-slugged", wks::Slug("nothing-to-be-slugged"));

  EXPECT_EQ("replace-some-spaces-and-underscores",
      wks::Slug(" replace:\tsome_spaces  and UNDERSCORES  _- "));

  EXPECT_EQ("", wks::Slug(" \r\n\t\v\f"));
  EXPECT_EQ("a", wks::Slug("a \r\n\t\v\f"));
  EXPECT_EQ("b", wks::Slug(" \r\n\t\v\fb"));
  EXPECT_EQ("a-b", wks::Slug("A \r\n\t\v\fB"));

  EXPECT_EQ("nr2-pm23pc", wks::Slug("#2 \u00b123%"));
  EXPECT_TRUE(wks::Slug(":?`!").empty());
  EXPECT_TRUE(wks::Slug("").empty());

  EXPECT_EQ("oesterreich", wks::Slug("Österreich!"));
  EXPECT_EQ("euro-dollar-mu", wks::Slug("€   $ \t \n µ   \t"));
  EXPECT_EQ("aeaeoeoeueue", wks::Slug("ÄäÖöÜü"));

  // TODO test all replacement characters!
}

TEST(StringUtilsTest, Shorten) {
  // Edge cases: empty & desired length 0 or longer than string
  EXPECT_EQ("", wks::Shorten("", 4));
  EXPECT_EQ("", wks::Shorten("abc", 0));
  EXPECT_EQ("abc", wks::Shorten("abc", 3));
  EXPECT_EQ("abc", wks::Shorten("abc", 10));

  // Desired length shorter than (custom) ellipsis
  EXPECT_THROW(wks::Shorten("abc", 2), std::invalid_argument);
  EXPECT_THROW(
      wks::Shorten("0123456789", 3, -1, "abcd"), std::invalid_argument);

  // Ellipsis left
  EXPECT_EQ("...", wks::Shorten("0123456789", 3, -1));
  EXPECT_EQ("...9", wks::Shorten("0123456789", 4, -1));
  EXPECT_EQ("...89", wks::Shorten("0123456789", 5, -1));
  EXPECT_EQ("_789", wks::Shorten("0123456789", 4, -1, "_"));
  EXPECT_EQ("_6789", wks::Shorten("0123456789", 5, -1, "_"));

  // Ellipsis centered
  EXPECT_EQ("...", wks::Shorten("0123456789", 3, 0));
  EXPECT_EQ("...9", wks::Shorten("0123456789", 4, 0));
  EXPECT_EQ("0...9", wks::Shorten("0123456789", 5, 0));
  EXPECT_EQ("_", wks::Shorten("0123456789", 1, 0, "_"));
  EXPECT_EQ("_9", wks::Shorten("0123456789", 2, 0, "_"));
  EXPECT_EQ("0_9", wks::Shorten("0123456789", 3, 0, "_"));
  EXPECT_EQ("0_89", wks::Shorten("0123456789", 4, 0, "_"));
  EXPECT_EQ("01_89", wks::Shorten("0123456789", 5, 0, "_"));

  // Ellipsis right
  EXPECT_EQ("...", wks::Shorten("0123456789", 3, 1));
  EXPECT_EQ("0...", wks::Shorten("0123456789", 4, 1));
  EXPECT_EQ("01...", wks::Shorten("0123456789", 5, 1));
  EXPECT_EQ("012_", wks::Shorten("0123456789", 4, 1, "_"));
  EXPECT_EQ("0123_", wks::Shorten("0123456789", 5, 1, "_"));
}

TEST(StringUtilsTest, Levenshtein) {
  EXPECT_EQ(0, wks::LevenshteinDistance(""sv, ""sv));
  EXPECT_EQ(0, wks::LevenshteinDistance("Frobmorten"sv, "Frobmorten"sv));
  EXPECT_EQ(7, wks::LevenshteinDistance("Frambozzle"sv, "Frobmorten"sv));
  EXPECT_EQ(3, wks::LevenshteinDistance("kitten"sv, "sitting"sv));
  EXPECT_EQ(3, wks::LevenshteinDistance("Kitten"sv, "sitting"sv));
  EXPECT_EQ(1, wks::LevenshteinDistance("my-key"sv, "my-keY"sv));
  EXPECT_EQ(3, wks::LevenshteinDistance("Hello"sv, "halo"sv));
  EXPECT_EQ(6, wks::LevenshteinDistance("my-key"sv, ""sv));
}

// NOLINTEND
