#include "options.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace even_flop::cli {
namespace {

// Mirrors how main() receives argv, including the program name at index 0.
std::expected<Options, std::string> parse(std::vector<const char*> arguments) {
  arguments.insert(arguments.begin(), "even-flop");
  return parseArguments(static_cast<int>(arguments.size()), arguments.data());
}

TEST(ParseArguments, ReadsTwoHands) {
  const auto options = parse({"AhKs", "QdQc"});

  ASSERT_TRUE(options.has_value()) << options.error();
  EXPECT_EQ(options->hero.toString(), "AhKs");
  EXPECT_EQ(options->villain.toString(), "QdQc");
}

TEST(ParseArguments, DefaultsToTenResultsAndAutomaticThreads) {
  const auto options = parse({"AhKs", "QdQc"});

  ASSERT_TRUE(options.has_value()) << options.error();
  EXPECT_EQ(options->search.topCount, 10);
  EXPECT_EQ(options->search.threadCount, 0);
  EXPECT_FALSE(options->asJson);
}

TEST(ParseArguments, AcceptsShortAndLongTopFlags) {
  const auto shortFlag = parse({"AhKs", "QdQc", "-n", "3"});
  const auto longFlag = parse({"AhKs", "QdQc", "--top", "3"});

  ASSERT_TRUE(shortFlag.has_value()) << shortFlag.error();
  ASSERT_TRUE(longFlag.has_value()) << longFlag.error();
  EXPECT_EQ(shortFlag->search.topCount, 3);
  EXPECT_EQ(longFlag->search.topCount, 3);
}

TEST(ParseArguments, AcceptsThreadFlag) {
  const auto options = parse({"AhKs", "QdQc", "--threads", "4"});

  ASSERT_TRUE(options.has_value()) << options.error();
  EXPECT_EQ(options->search.threadCount, 4);
}

TEST(ParseArguments, AcceptsJsonFlag) {
  const auto options = parse({"AhKs", "QdQc", "--json"});

  ASSERT_TRUE(options.has_value()) << options.error();
  EXPECT_TRUE(options->asJson);
}

TEST(ParseArguments, AcceptsFlagsBeforeHands) {
  const auto options = parse({"--top", "2", "AhKs", "QdQc"});

  ASSERT_TRUE(options.has_value()) << options.error();
  EXPECT_EQ(options->search.topCount, 2);
  EXPECT_EQ(options->hero.toString(), "AhKs");
}

TEST(ParseArguments, RejectsWrongNumberOfHands) {
  EXPECT_FALSE(parse({"AhKs"}).has_value());
  EXPECT_FALSE(parse({"AhKs", "QdQc", "5h5d"}).has_value());
}

TEST(ParseArguments, RejectsUnparseableHand) {
  const auto options = parse({"AhKs", "ZZZZ"});

  ASSERT_FALSE(options.has_value());
  EXPECT_FALSE(options.error().empty());
}

TEST(ParseArguments, RejectsUnknownOption) {
  const auto options = parse({"AhKs", "QdQc", "--nope"});

  ASSERT_FALSE(options.has_value());
  EXPECT_NE(options.error().find("--nope"), std::string::npos);
}

TEST(ParseArguments, RejectsFlagWithoutValue) {
  const auto options = parse({"AhKs", "QdQc", "--top"});

  ASSERT_FALSE(options.has_value());
  EXPECT_NE(options.error().find("--top"), std::string::npos);
}

TEST(ParseArguments, RejectsNonNumericCount) {
  const auto options = parse({"AhKs", "QdQc", "--top", "many"});

  ASSERT_FALSE(options.has_value());
  EXPECT_NE(options.error().find("many"), std::string::npos);
}

TEST(ParseArguments, RejectsTrailingGarbageOnACount) {
  // "5x" must not silently parse as 5.
  const auto options = parse({"AhKs", "QdQc", "--top", "5x"});

  EXPECT_FALSE(options.has_value());
}

TEST(ParseArguments, RejectsCountBelowOne) {
  EXPECT_FALSE(parse({"AhKs", "QdQc", "--top", "0"}).has_value());
  EXPECT_FALSE(parse({"AhKs", "QdQc", "--threads", "0"}).has_value());
}

TEST(WantsHelp, DetectsHelpFlags) {
  const char* shortFlag[] = {"even-flop", "-h"};
  const char* longFlag[] = {"even-flop", "--help"};
  const char* noArguments[] = {"even-flop"};
  const char* normal[] = {"even-flop", "AhKs", "QdQc"};

  EXPECT_TRUE(wantsHelp(2, shortFlag));
  EXPECT_TRUE(wantsHelp(2, longFlag));
  EXPECT_TRUE(wantsHelp(1, noArguments));
  EXPECT_FALSE(wantsHelp(3, normal));
}

TEST(Usage, MentionsEveryFlag) {
  const std::string text = usage();

  EXPECT_NE(text.find("--top"), std::string::npos);
  EXPECT_NE(text.find("--threads"), std::string::npos);
  EXPECT_NE(text.find("--json"), std::string::npos);
  EXPECT_NE(text.find("--help"), std::string::npos);
}

}  // namespace
}  // namespace even_flop::cli
