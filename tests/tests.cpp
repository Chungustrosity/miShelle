#include <gtest/gtest.h>
#include <vector>

#include "../src/string_utils.hpp"

#include "../src/Parser.hpp"

// TEST(string_utils_test, skip_white_space_handles_various_inputs) {
//   EXPECT_EQ(skipWhitespace("   hello"), 3);
//   EXPECT_EQ(skipWhitespace("hellow"), 0);
//   EXPECT_EQ(skipWhitespace(""), 0);
// }
//
// TEST(string_utils_test, expand_tilde) {
//   EXPECT_EQ(expandToken("~"), std::getenv("HOME"));
//   EXPECT_EQ(expandToken("~/bin"), std::string(std::getenv("HOME")) + "/bin");
//   EXPECT_EQ(expandToken("   ~  "),
//             "   " + std::string(std::getenv("HOME")) + "  ");
//   EXPECT_EQ(expandToken("~ /dev/"),
//             std::string(std::getenv("HOME")) + " /dev/");
// }
//
// TEST(ParserTest, extracts_command_and_args_correctly) {
//   std::string mockinput = " echo hellow world ";
//   std::vector<std::string> mockhistory;
//   Parser parser(mockinput, mockhistory);
//   EXPECT_EQ(parser.getCommand(), "echo");
//   EXPECT_EQ(parser.getArguments()[0], "hellow");
//   EXPECT_EQ(parser.getArguments()[1], "world");
//   EXPECT_FALSE(parser.isNoArguments());
// }
//
// TEST(ParserTest, tokenization) {
//   std::vector<std::string> mockhistory;
//   std::string empty = "  ";
//   std::string command = " cd ";
//   std::string full = " echo hello world ";
//   std::string full2 = " pwd ";
//   std::string full3 = " ls ./damn ";
//   std::string full4 = " cd ~/bin ";
//
//   Parser parser(empty, mockhistory);
//   EXPECT_EQ(parser.getTokens().empty(), true);
//
//   Parser parser2(command, mockhistory);
//   EXPECT_EQ(parser2.getTokens().size(), 1);
//   EXPECT_EQ(parser2.getTokens()[0], "cd");
//
//   Parser parser3(full, mockhistory);
//   EXPECT_EQ(parser3.getTokens().size(), 3);
//   EXPECT_EQ(parser3.getTokens()[0], "echo");
//   EXPECT_EQ(parser3.getTokens()[1], "hello");
//   EXPECT_EQ(parser3.getTokens()[2], "world");
//
//   Parser parser4(full2, mockhistory);
//   EXPECT_EQ(parser4.getTokens().size(), 1);
//   EXPECT_EQ(parser4.getTokens()[0], "pwd");
//
//   Parser parser5(full3, mockhistory);
//   EXPECT_EQ(parser5.getTokens().size(), 2);
//   EXPECT_EQ(parser5.getTokens()[0], "ls");
//   EXPECT_EQ(parser5.getTokens()[1], "./damn");
//
//   Parser parser6(full4, mockhistory);
//   EXPECT_EQ(parser6.getTokens().size(), 2);
//   EXPECT_EQ(parser6.getTokens()[0], "cd");
// }

TEST(ParserTest, expand_telde_correctly) {
  std::string mockinput = " echo ~/bin ";
  std::vector<std::string> mockhistory;
  Parser parser(mockinput, mockhistory);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
  EXPECT_EQ(parser.getTokens()[0], "echo");
  EXPECT_EQ(parser.getTokens()[1], "/home/panties/bin");

  std::string mockinput2 = " cd ~ ";
  Parser parser2(mockinput2, mockhistory);
  EXPECT_EQ(parser2.isMultiLineCommand(), false);
  EXPECT_EQ(parser2.getTokens()[0], "cd");
  EXPECT_EQ(parser2.getTokens()[1], "/home/panties");
}

TEST(parserTest, parse_single_quotes_literally) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo '~ boobs are good' ", mockhistory);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
  EXPECT_EQ(parser.getArguments()[0], "~ boobs are good");
  EXPECT_EQ(parser.isNoArguments(), false);
}

TEST(parserTest, join_two_touching_single_quoted_tokens) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo 'boobs ''are'' good' ", mockhistory);
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments()[0], "boobs are good");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(parserTest, concat_single_quoted_strings_with_the_previous_token) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo hi' boobs' ", mockhistory);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getArguments()[0], "hi boobs");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
  EXPECT_EQ(parser.isNoArguments(), false);
}
TEST(parserTest, concat_single_quoted_strings_with_tailing_token) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo 'boobs 'hi ", mockhistory);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getArguments()[0], "boobs hi");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
  EXPECT_EQ(parser.isNoArguments(), false);
}

TEST(parserTest, ignore_empty_single_quotes) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo '' hi' hello' 'boobs' '' ", mockhistory);
  EXPECT_EQ(parser.getArguments().size(), 2);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments()[0], "hi hello");
  EXPECT_EQ(parser.getArguments()[1], "boobs");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}

TEST(parserTest, parse_double_quotes_literally) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo \"~ boobs are good\" ", mockhistory);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
  EXPECT_EQ(parser.getArguments()[0], "~ boobs are good");
  EXPECT_EQ(parser.isNoArguments(), false);
}

TEST(parserTest, join_two_touching_double_quoted_tokens) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo \"boobs \"\"are\"\" good\" ", mockhistory);
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments()[0], "boobs are good");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(parserTest, concat_double_quoted_strings_with_the_previous_token) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo hi\" boobs\" ", mockhistory);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getArguments()[0], "hi boobs");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
  EXPECT_EQ(parser.isNoArguments(), false);
}
TEST(parserTest, concat_double_quoted_strings_with_tailing_token) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo \"boobs \"hi ", mockhistory);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getArguments()[0], "boobs hi");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
  EXPECT_EQ(parser.isNoArguments(), false);
}

TEST(parserTest, ignore_empty_double_quotes) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo \"\" hi\" hello\" \"boobs\" \"\" ", mockhistory);
  EXPECT_EQ(parser.getArguments().size(), 2);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments()[0], "hi hello");
  EXPECT_EQ(parser.getArguments()[1], "boobs");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
// TEST(ParserTest, single_quote_support){
//   std::string mockinput = "hello world"
// }
