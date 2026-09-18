#include "report.hpp"

#include <algorithm>

#include <gtest/gtest.h>

namespace even_flop::cli {
namespace {

Hand hand(std::string_view text) { return parseHand(text).value(); }

Flop flop(std::string_view a, std::string_view b, std::string_view c) {
  return Flop{parseCard(a).value(), parseCard(b).value(), parseCard(c).value()};
}

SearchReport sampleReport() {
  return SearchReport{
      .flopsEvaluated = TOTAL_FLOPS,
      .top = {FlopResult{.flop = flop("8h", "9h", "Jh"),
                         .equity = Equity{.wins = 496, .losses = 492, .ties = 2}}},
  };
}

ReportContext sampleContext() {
  return ReportContext{
      .hero = hand("AhKs"), .villain = hand("QdQc"), .elapsed = std::chrono::milliseconds{34}};
}

TEST(FormatTable, ShowsBothHandsAndTheFlopCount) {
  const std::string text = formatTable(sampleReport(), sampleContext());

  EXPECT_NE(text.find("AhKs"), std::string::npos);
  EXPECT_NE(text.find("QdQc"), std::string::npos);
  EXPECT_NE(text.find("17296"), std::string::npos);
  EXPECT_NE(text.find("34 ms"), std::string::npos);
}

TEST(FormatTable, PrintsFlopCardsSpacedAndEquitiesAsPercentages) {
  const std::string text = formatTable(sampleReport(), sampleContext());

  EXPECT_NE(text.find("8h 9h Jh"), std::string::npos);
  EXPECT_NE(text.find("50.20%"), std::string::npos);
  EXPECT_NE(text.find("49.80%"), std::string::npos);
}

TEST(FormatTable, HandlesAnEmptyResultList) {
  const SearchReport empty{.flopsEvaluated = TOTAL_FLOPS, .top = {}};

  const std::string text = formatTable(empty, sampleContext());

  EXPECT_NE(text.find("17296"), std::string::npos);
}

TEST(EscapeJsonString, LeavesOrdinaryTextAlone) {
  EXPECT_EQ(escapeJsonString("AhKs"), "AhKs");
}

TEST(EscapeJsonString, EscapesQuotesAndBackslashes) {
  EXPECT_EQ(escapeJsonString("Card \"Xs\" is bad"), "Card \\\"Xs\\\" is bad");
  EXPECT_EQ(escapeJsonString("a\\b"), "a\\\\b");
}

TEST(EscapeJsonString, EscapesControlCharacters) {
  EXPECT_EQ(escapeJsonString("a\nb"), "a\\nb");
  EXPECT_EQ(escapeJsonString("a\tb"), "a\\tb");
  EXPECT_EQ(escapeJsonString(std::string(1, '\x01')), "\\u0001");
}

TEST(FormatJson, IncludesRawCountsAlongsideEquities) {
  const std::string text = formatJson(sampleReport(), sampleContext());

  EXPECT_NE(text.find("\"flop\": \"8h9hJh\""), std::string::npos);
  EXPECT_NE(text.find("\"wins\": 496"), std::string::npos);
  EXPECT_NE(text.find("\"losses\": 492"), std::string::npos);
  EXPECT_NE(text.find("\"ties\": 2"), std::string::npos);
  EXPECT_NE(text.find("\"runouts\": 990"), std::string::npos);
}

TEST(FormatJson, SeparatesResultsWithCommasButNotTrailing) {
  SearchReport report = sampleReport();
  report.top.push_back(FlopResult{.flop = flop("8s", "9s", "Js"),
                                  .equity = Equity{.wins = 497, .losses = 492, .ties = 1}});

  const std::string text = formatJson(report, sampleContext());

  // A trailing comma before the closing bracket would be invalid JSON.
  EXPECT_EQ(text.find(",\n  ]"), std::string::npos);
  EXPECT_NE(text.find("},\n"), std::string::npos);
}

TEST(FormatJson, IsBalancedForAnEmptyResultList) {
  const SearchReport empty{.flopsEvaluated = TOTAL_FLOPS, .top = {}};

  const std::string text = formatJson(empty, sampleContext());

  EXPECT_NE(text.find("\"results\": [\n  ]"), std::string::npos);
  EXPECT_EQ(std::ranges::count(text, '{'), std::ranges::count(text, '}'));
}

}  // namespace
}  // namespace even_flop::cli
