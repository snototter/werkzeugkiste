#include <chrono>
#include <thread>
#include <string>
#include <map>


#include <gtest/gtest.h>

#include <werkzeugkiste/sort/sort.h>

namespace wks = werkzeugkiste::sort;

TEST(SortUtilsTest, MapKeys) {
  std::map<int, int> m1{{0, -1}, {17, -3}, {42, 9}, {-100, 3}};
  auto k1 = wks::GetMapKeys(m1);
  EXPECT_EQ(m1.size(), 4);
  EXPECT_EQ(k1.size(), 4);
  EXPECT_TRUE(wks::Contains(m1, 0));
  EXPECT_TRUE(wks::Contains(m1, 17));
  EXPECT_TRUE(wks::Contains(m1, 42));
  EXPECT_TRUE(wks::Contains(m1, -100));
  EXPECT_FALSE(wks::Contains(m1, -1));

  std::map<std::string, int> m2{{"foo", -1}, {"Bar", 10}, {"A B C", 1}};
  auto k2 = wks::GetMapKeys(m2);
  EXPECT_EQ(m2.size(), 3);
  EXPECT_EQ(k2.size(), 3);
  EXPECT_TRUE(wks::Contains(m2, "foo"));
  EXPECT_TRUE(wks::Contains(m2, "Bar"));
  EXPECT_TRUE(wks::Contains(m2, "A B C"));
  EXPECT_FALSE(wks::Contains(m2, "Foo"));
  EXPECT_FALSE(wks::Contains(m2, "bar"));


  std::map<std::pair<int, std::string>,
      int> m3{{{1, "foo"}, -1}, {{99, "Bar"}, 10},
              {{2, "foo"}, 0}};
  auto k3 = wks::GetMapKeys(m3);
  EXPECT_EQ(m3.size(), 3);
  EXPECT_EQ(k3.size(), 3);
  EXPECT_TRUE(wks::Contains(m3, std::make_pair(1, "foo")));
  EXPECT_TRUE(wks::Contains(m3, std::make_pair(2, "foo")));
  EXPECT_FALSE(wks::Contains(m3, std::make_pair(3, "foo")));
  EXPECT_FALSE(wks::Contains(m3, std::make_pair(1, "Foo")));
  EXPECT_TRUE(wks::Contains(m3, std::make_pair(99, "Bar")));
  EXPECT_FALSE(wks::Contains(m3, std::make_pair(98, "Bar")));
  EXPECT_FALSE(wks::Contains(m3, std::make_pair(99, "bar")));
}


//TEST(StringUtilsTest, Prefix) {
//  EXPECT_TRUE(wks::StartsWith("Another test string", "Another "));
//  EXPECT_FALSE(wks::StartsWith("Another test string", "another "));  // case mismatch
//  EXPECT_TRUE(wks::StartsWith("Another test string", "An"));
//  EXPECT_TRUE(wks::StartsWith("Another test string", "A"));  // single character string
//  EXPECT_TRUE(wks::StartsWith("Another test string", 'A'));  // character
//  EXPECT_FALSE(wks::StartsWith("Another test string", 'a'));  // character
//  EXPECT_FALSE(wks::StartsWith("Another test string", ""));  // empty string
//  EXPECT_FALSE(wks::StartsWith("", ""));
//  EXPECT_FALSE(wks::StartsWith("", "A"));
//}


//TEST(StringUtilsTest, CaseConversion) {
//  EXPECT_EQ(wks::Upper("abcdefghijklmnopqrstuvwxyz"),
//            "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
//  EXPECT_EQ(wks::Upper("ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
//            "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
//  EXPECT_EQ(wks::Upper("1234567890+*~#'-_.:,;´`\\?}=])[({/&%$§3!^°@<|>"),
//            "1234567890+*~#'-_.:,;´`\\?}=])[({/&%$§3!^°@<|>");

//  EXPECT_EQ(wks::Lower("abcdefghijklmnopqrstuvwxyz"),
//            "abcdefghijklmnopqrstuvwxyz");
//  EXPECT_EQ(wks::Lower("ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
//            "abcdefghijklmnopqrstuvwxyz");
//  EXPECT_EQ(wks::Lower("1234567890+*~#'-_.:,;´`\\?}=])[({/&%$§3!^°@<|>"),
//            "1234567890+*~#'-_.:,;´`\\?}=])[({/&%$§3!^°@<|>");
//}


//TEST(StringUtilsTest, Trimming) {
//  // Tab \t, Line feed \n, vertical tab \v, form feed \f, space, ...
//  EXPECT_EQ(wks::Trim(" \t\r\n\v\fabc \t\r\n\v\f123 \t\r\n\v\f"),
//            "abc \t\r\n\v\f123");

//  EXPECT_EQ(wks::LTrim(" \t\r\n\v\fabc \t\r\n\v\f123 \t\r\n\v\f"),
//            "abc \t\r\n\v\f123 \t\r\n\v\f");

//  EXPECT_EQ(wks::RTrim(" \t\r\n\v\fabc \t\r\n\v\f123 \t\r\n\v\f"),
//            " \t\r\n\v\fabc \t\r\n\v\f123");
//}


//TEST(StringUtilsTest, IsNumeric) {
//  EXPECT_TRUE(wks::IsNumeric("0"));
//  EXPECT_TRUE(wks::IsNumeric("+0"));
//  EXPECT_TRUE(wks::IsNumeric("-0"));
//  EXPECT_TRUE(wks::IsNumeric("1234"));
//  EXPECT_TRUE(wks::IsNumeric("-3"));
//  EXPECT_TRUE(wks::IsNumeric("+42"));
//  EXPECT_FALSE(wks::IsNumeric("0!"));
//  EXPECT_FALSE(wks::IsNumeric("a!"));
//  EXPECT_FALSE(wks::IsNumeric("123 456"));

//  EXPECT_TRUE(wks::IsNumeric("1234567890000000000000000000"));
//  EXPECT_TRUE(wks::IsNumeric("12345678900000000000000000000000000000"));
//  EXPECT_TRUE(wks::IsNumeric("12345678900000000000000000000000000000.123456"));

//  EXPECT_FALSE(wks::IsNumeric(""));
//  EXPECT_FALSE(wks::IsNumeric("-"));
//  EXPECT_FALSE(wks::IsNumeric("+e"));
//  EXPECT_FALSE(wks::IsNumeric("+e-7"));

//  EXPECT_TRUE(wks::IsNumeric("10e3"));
//  EXPECT_TRUE(wks::IsNumeric(".0"));
//  EXPECT_TRUE(wks::IsNumeric("0.42"));
//  EXPECT_TRUE(wks::IsNumeric("-2.3"));
//  EXPECT_TRUE(wks::IsNumeric("1e-7"));
//}


//TEST(StringUtilsTest, Tokenize) {
//  auto tokens = wks::Split("A;Line ;\tto;be;split ;;", ';');
//  EXPECT_EQ(tokens.size(), 6);
//  EXPECT_EQ(tokens[0], "A");
//  EXPECT_EQ(tokens[1], "Line ");
//  EXPECT_EQ(tokens[2], "\tto");
//  EXPECT_EQ(tokens[3], "be");
//  EXPECT_EQ(tokens[4], "split ");
//  EXPECT_TRUE(tokens[5].empty());

//  auto s = "Another;string;for;tokenization;";
//  tokens = wks::Split(s, '!');
//  EXPECT_EQ(tokens.size(), 1);
//  EXPECT_EQ(tokens[0], s);
//}


//TEST(StringUtilsTest, Replace) {
//  EXPECT_TRUE(wks::Replace("", "", "").empty());
//  EXPECT_TRUE(wks::Replace("", "abc", "def").empty());
//  EXPECT_TRUE(wks::Replace("", "", "def").empty());

//  // Nothing changes if search string is empty
//  EXPECT_EQ(wks::Replace("ABC123abc;:_", "", "!!!!!"),
//            "ABC123abc;:_");

//  EXPECT_EQ(wks::Replace("ABC123abc;:_", "a", "!!"),
//            "ABC123!!bc;:_");

//  EXPECT_EQ(wks::Replace("ABC123abc;:_", "abcdef", "!!"),
//            "ABC123abc;:_");

//  EXPECT_EQ(wks::Replace("ABC123abc;:_", "BC", ""),
//            "A123abc;:_");

//  // All occurrences should be replaced
//  EXPECT_EQ(wks::Replace("ABC123abc123ABC123abc123", "BC", ".."),
//            "A..123abc123A..123abc123");

//  // We can also remove substrings
//  EXPECT_EQ(wks::Replace("ABC123abc;:_", "bc", ""),
//            "ABC123a;:_");
//}


//TEST(StringUtilsTest, Remove) {
//  EXPECT_EQ(wks::Remove("1234567890+*~#'-_.:,;´`\\?}=])[({/&%$§3!^°@<|>", '\\'),
//            "1234567890+*~#'-_.:,;´`?}=])[({/&%$§3!^°@<|>");

//  EXPECT_EQ(wks::Remove("abcDEFghiABCdefGHIabc", 'a'),
//            "bcDEFghiABCdefGHIbc");

//  EXPECT_EQ(wks::Remove("abcDEFghiABCdefGHIabc", {'a', 'b', 'C'}),
//            "cDEFghiABdefGHIc");
//}


//TEST(StringUtilsTest, URL) {
//  // Simplistic URL parsing (downstream I need to be able
//  // to distinguish web URLs from file paths, e.g. to properly
//  // load a camera's SDP description)
//  std::string protocol, remainder;
//  EXPECT_TRUE(wks::GetUrlProtocol("file://foo.txt", protocol, remainder));
//  EXPECT_EQ(protocol, "file://");
//  EXPECT_EQ(remainder, "foo.txt");

//  EXPECT_TRUE(wks::GetUrlProtocol("UnChecked://SomeU.R.I:?asdf=foo",
//                                  protocol, remainder));
//  EXPECT_EQ(protocol, "UnChecked://");
//  EXPECT_EQ(remainder, "SomeU.R.I:?asdf=foo");

//  EXPECT_FALSE(wks::GetUrlProtocol("foo.txt", protocol, remainder));
//  EXPECT_TRUE(protocol.empty());
//  EXPECT_EQ(remainder, "foo.txt");

//  // When logging connection strings, I want to hide
//  // any potential authentication information (but still
//  // know that it was actually provided in the URL string):
//  EXPECT_EQ(wks::ObscureUrlAuthentication("file://foobar"),
//            "file://foobar");

//  EXPECT_EQ(wks::ObscureUrlAuthentication("http://user:pass@foo.bar"),
//            "http://<auth>@foo.bar");

//  EXPECT_EQ(wks::ObscureUrlAuthentication("rtsp://user:pass@foo.bar:12345"),
//            "rtsp://<auth>@foo.bar:12345");

//  EXPECT_EQ(wks::ObscureUrlAuthentication("https://user@192.168.0.1:8080/cam.cgi"),
//            "https://<auth>@192.168.0.1:8080/cam.cgi");

//  EXPECT_EQ(wks::ObscureUrlAuthentication("user:pass@some.thing:12345"),
//            "<auth>@some.thing:12345");


//  // If we want to strip the subpaths and parameters of a URL:
//  EXPECT_EQ(wks::ClipUrl("https://root@192.168.0.1:8080/cam.cgi"),
//            "https://<auth>@192.168.0.1:8080");

//  EXPECT_EQ(wks::ClipUrl("https://192.168.0.1:8080?image=still&overlay=off"),
//            "https://192.168.0.1:8080");

//  EXPECT_EQ(wks::ClipUrl("file:///a/file/needs/special/handling.txt"),
//            "file:///a/file/needs/special/handling.txt");
//}


//TEST(StringUtilsTest, Slug) {
//  EXPECT_EQ(wks::Slug("nothing-to-be-slugged"),
//            "nothing-to-be-slugged");

//  EXPECT_EQ(wks::Slug(" replace:\tsome_spaces  and UNDERSCORES  _- "),
//            "replace-some-spaces-and-underscores-");

//  EXPECT_EQ(wks::Slug(" \r\n\t\v\f"), "");
//  EXPECT_EQ(wks::Slug("a \r\n\t\v\f"), "a");
//  EXPECT_EQ(wks::Slug(" \r\n\t\v\fb"), "b");
//  EXPECT_EQ(wks::Slug("A \r\n\t\v\fB"), "a-b");

//  EXPECT_EQ(wks::Slug(":?`!"), "-");
//  EXPECT_EQ(wks::Slug("#2"), "nr2");
//  EXPECT_TRUE(wks::Slug("").empty());
//}
