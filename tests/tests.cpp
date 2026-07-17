#include <gtest/gtest.h>
#include <vector>

#include "../src/string_utils.hpp"

#include "../src/Parser.hpp"

TEST(string_utils_test, skip_white_space_handles_various_inputs) {
  EXPECT_EQ(skipWhitespace("   hello"), 3);
  EXPECT_EQ(skipWhitespace("hellow"), 0);
  EXPECT_EQ(skipWhitespace(""), 0);
}

TEST(string_utils_test, expand_tilde) {
  EXPECT_EQ(expandToken("~"), std::getenv("HOME"));
  EXPECT_EQ(expandToken("~/bin"), std::string(std::getenv("HOME")) + "/bin");
  EXPECT_EQ(expandToken("~ /dev/"),
            std::string(std::getenv("HOME")) + " /dev/");
}

TEST(tokenizerTest, extracts_command_and_args_correctly) {
  std::string mockinput = " echo hellow world ";
  std::vector<std::string> mockhistory;
  Parser parser(mockinput);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments()[0], "hellow");
  EXPECT_EQ(parser.getArguments()[1], "world");
  EXPECT_FALSE(parser.isNoArguments());
}

TEST(tokenizerTest, tokenization) {
  std::vector<std::string> mockhistory;
  std::string empty = "  ";
  std::string command = " cd ";
  std::string full = " echo hello world ";
  std::string full2 = " pwd ";
  std::string full3 = " ls ./damn ";
  std::string full4 = " cd ~/bin ";

  Parser parser(empty);
  EXPECT_EQ(parser.getTokens().empty(), true);

  Parser parser2(command);
  EXPECT_EQ(parser2.getTokens().size(), 1);
  EXPECT_EQ(parser2.getTokens()[0], "cd");

  Parser parser3(full);
  EXPECT_EQ(parser3.getTokens().size(), 3);
  EXPECT_EQ(parser3.getTokens()[0], "echo");
  EXPECT_EQ(parser3.getTokens()[1], "hello");
  EXPECT_EQ(parser3.getTokens()[2], "world");

  Parser parser4(full2);
  EXPECT_EQ(parser4.getTokens().size(), 1);
  EXPECT_EQ(parser4.getTokens()[0], "pwd");

  Parser parser5(full3);
  EXPECT_EQ(parser5.getTokens().size(), 2);
  EXPECT_EQ(parser5.getTokens()[0], "ls");
  EXPECT_EQ(parser5.getTokens()[1], "./damn");

  Parser parser6(full4);
  EXPECT_EQ(parser6.getTokens().size(), 2);
  EXPECT_EQ(parser6.getTokens()[0], "cd");
}

TEST(tokenizerTest, expand_telde_correctly) {
  std::string mockinput = " echo ~/bin ";
  std::vector<std::string> mockhistory;
  Parser parser(mockinput);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
  EXPECT_EQ(parser.getTokens()[0], "echo");
  EXPECT_EQ(parser.getTokens()[1], "/home/panties/bin");

  std::string mockinput2 = " cd ~ ";
  Parser parser2(mockinput2);
  EXPECT_EQ(parser2.isMultiLineCommand(), false);
  EXPECT_EQ(parser2.getTokens()[0], "cd");
  EXPECT_EQ(parser2.getTokens()[1], "/home/panties");
}

TEST(tokenizerTest, parse_single_quotes_literally) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo '~ boobs are good' ");
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
  EXPECT_EQ(parser.getArguments()[0], "~ boobs are good");
  EXPECT_EQ(parser.isNoArguments(), false);
}

TEST(tokenizerTest, join_two_touching_single_quoted_tokens) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo 'boobs ''are'' good' ");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments()[0], "boobs are good");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(tokenizerTest, concat_single_quoted_strings_with_the_previous_token) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo hi' boobs' ");
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getArguments()[0], "hi boobs");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
  EXPECT_EQ(parser.isNoArguments(), false);
}
TEST(tokenizerTest, concat_single_quoted_strings_with_tailing_token) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo 'boobs 'hi ");
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getArguments()[0], "boobs hi");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
  EXPECT_EQ(parser.isNoArguments(), false);
}

TEST(tokenizerTest, ignore_empty_single_quotes) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo '' hi' hello' 'boobs' '' ");
  EXPECT_EQ(parser.getArguments().size(), 2);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments()[0], "hi hello");
  EXPECT_EQ(parser.getArguments()[1], "boobs");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}

TEST(tokenizerTest, parse_double_quotes_literally) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo \"~ boobs are good\" ");
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
  EXPECT_EQ(parser.getArguments()[0], "~ boobs are good");
  EXPECT_EQ(parser.isNoArguments(), false);
}

TEST(tokenizerTest, join_two_touching_double_quoted_tokens) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo \"boobs \"\"are\"\" good\" ");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments()[0], "boobs are good");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(tokenizerTest, concat_double_quoted_strings_with_the_previous_token) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo hi\" boobs\" ");
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getArguments()[0], "hi boobs");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
  EXPECT_EQ(parser.isNoArguments(), false);
}
TEST(tokenizerTest, concat_double_quoted_strings_with_tailing_token) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo \"boobs \"hi ");
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getArguments()[0], "boobs hi");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
  EXPECT_EQ(parser.isNoArguments(), false);
}
TEST(tokenizerTest, ignore_empty_double_quotes) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo \"\" hi\" hello\" \"boobs\" \"\" ");
  EXPECT_EQ(parser.getArguments().size(), 2);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments()[0], "hi hello");
  EXPECT_EQ(parser.getArguments()[1], "boobs");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}

TEST(tokenizerTest, proper_backslash_handling_in_raw_tokens) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo three\\ \\ \\ spaces ");
  EXPECT_EQ(parser.getArguments()[0], "three   spaces");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(tokenizerTest, proper_backslash_handling_in_raw_tokens__multi_arguments) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo before\\              after ");
  EXPECT_EQ(parser.getArguments()[0], "before ");
  EXPECT_EQ(parser.getArguments()[1], "after");
  EXPECT_EQ(parser.getArguments().size(), 2);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(tokenizerTest,
     proper_backslash_handling_in_raw_tokens__none_special_characters) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo test\\nexample ");
  EXPECT_EQ(parser.getArguments()[0], "testnexample");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(tokenizerTest,
     proper_backslash_handling_in_raw_tokens__escaping_backslash) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo hello\\\\world ");
  EXPECT_EQ(parser.getArguments()[0], "hello\\world");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(tokenizerTest,
     proper_backslash_handling_in_raw_tokens__escaping_single_quotes) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo \\'hello\\' ");
  EXPECT_EQ(parser.getArguments()[0], "'hello'");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(tokenizerTest,
     proper_backslash_handling_in_double_quoted_tokens__escaping_backslash) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo \"A \\ escapes itself\" ");
  EXPECT_EQ(parser.getArguments()[0], "A \\ escapes itself");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(
    tokenizerTest,
    proper_backslash_handling_in_double_quoted_tokens__escaping_double_quotes) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo \"A \\\" inside double quotes\" ");
  EXPECT_EQ(parser.getArguments()[0], "A \" inside double quotes");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.isNoArguments(), false);
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}

TEST(parserTest, handle_redirections) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo boobs >hi.txt ");
  EXPECT_EQ(parser.getRedirections().size(), 1);
  EXPECT_EQ(parser.getRedirections()[0].FileDescriptor, 1);
  EXPECT_EQ(parser.getRedirections()[0].redirectionType,
            RedirectionType::OverwriteOutputRedirection);
  EXPECT_EQ(parser.getRedirections()[0].target, "hi.txt");
  //
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments().size(), 1);
  EXPECT_EQ(parser.getArguments()[0], "boobs");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(parserTest, handle_redirections__multiple_command_arguments) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo boobs hi hey >hi.txt ");
  EXPECT_EQ(parser.getRedirections().size(), 1);
  EXPECT_EQ(parser.getRedirections()[0].FileDescriptor, 1);
  EXPECT_EQ(parser.getRedirections()[0].redirectionType,
            RedirectionType::OverwriteOutputRedirection);
  EXPECT_EQ(parser.getRedirections()[0].target, "hi.txt");
  //
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments().size(), 3);
  EXPECT_EQ(parser.getArguments()[0], "boobs");
  EXPECT_EQ(parser.getArguments()[1], "hi");
  EXPECT_EQ(parser.getArguments()[2], "hey");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(parserTest, handle_redirections__file_descriptor_specified) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo boobs hi hey 1>hi.txt ");
  EXPECT_EQ(parser.getRedirections().size(), 1);
  EXPECT_EQ(parser.getRedirections()[0].FileDescriptor, 1);
  EXPECT_EQ(parser.getRedirections()[0].redirectionType,
            RedirectionType::OverwriteOutputRedirection);
  EXPECT_EQ(parser.getRedirections()[0].target, "hi.txt");
  //
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments().size(), 3);
  EXPECT_EQ(parser.getArguments()[0], "boobs");
  EXPECT_EQ(parser.getArguments()[1], "hi");
  EXPECT_EQ(parser.getArguments()[2], "hey");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(parserTest, handle_redirections__file_descriptor_specified__foreign_fd) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo boobs hi hey 5>hi.txt ");
  EXPECT_EQ(parser.getRedirections().size(), 1);
  EXPECT_EQ(parser.getRedirections()[0].FileDescriptor, 5);
  EXPECT_EQ(parser.getRedirections()[0].redirectionType,
            RedirectionType::OverwriteOutputRedirection);
  EXPECT_EQ(parser.getRedirections()[0].target, "hi.txt");
  //
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments().size(), 3);
  EXPECT_EQ(parser.getArguments()[0], "boobs");
  EXPECT_EQ(parser.getArguments()[1], "hi");
  EXPECT_EQ(parser.getArguments()[2], "hey");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(parserTest, handle__redirections__weirdly_placed__before_arguments) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo 5>hi.txt boobs hi hey ");
  EXPECT_EQ(parser.getRedirections().size(), 1);
  EXPECT_EQ(parser.getRedirections()[0].FileDescriptor, 5);
  EXPECT_EQ(parser.getRedirections()[0].redirectionType,
            RedirectionType::OverwriteOutputRedirection);
  EXPECT_EQ(parser.getRedirections()[0].target, "hi.txt");
  //
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments().size(), 3);
  EXPECT_EQ(parser.getArguments()[0], "boobs");
  EXPECT_EQ(parser.getArguments()[1], "hi");
  EXPECT_EQ(parser.getArguments()[2], "hey");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
TEST(parserTest, handle__redirections__weirdly_placed__mid_arguments) {
  std::vector<std::string> mockhistory;
  Parser parser(" echo boobs 5>hi.txt hi hey ");
  EXPECT_EQ(parser.getRedirections().size(), 1);
  EXPECT_EQ(parser.getRedirections()[0].FileDescriptor, 5);
  EXPECT_EQ(parser.getRedirections()[0].redirectionType,
            RedirectionType::OverwriteOutputRedirection);
  EXPECT_EQ(parser.getRedirections()[0].target, "hi.txt");
  //
  EXPECT_EQ(parser.getCommand(), "echo");
  EXPECT_EQ(parser.getArguments().size(), 3);
  EXPECT_EQ(parser.getArguments()[0], "boobs");
  EXPECT_EQ(parser.getArguments()[1], "hi");
  EXPECT_EQ(parser.getArguments()[2], "hey");
  EXPECT_EQ(parser.isMultiLineCommand(), false);
}
// TEST(tokenizerTest, single_quote_support){
//   std::string mockinput = "hello world"
// }
