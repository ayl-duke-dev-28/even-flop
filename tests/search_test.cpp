#include "even_flop/search.hpp"

#include <algorithm>
#include <gtest/gtest.h>

#include "even_flop/deck.hpp"

namespace even_flop {
namespace {

Hand hand(std::string_view text) { return parseHand(text).value(); }

TEST(FindEvenFlops, EvaluatesEveryPossibleFlop) {
  const auto report = findEvenFlops(hand("AhKs"), hand("QdQc"), {.topCount = 5});

  ASSERT_TRUE(report.has_value()) << report.error();
  EXPECT_EQ(report->flopsEvaluated, TOTAL_FLOPS);
}

TEST(FindEvenFlops, ReturnsRequestedNumberOfResults) {
  const auto report = findEvenFlops(hand("AhKs"), hand("QdQc"), {.topCount = 7});

  ASSERT_TRUE(report.has_value()) << report.error();
  EXPECT_EQ(report->top.size(), 7u);
}

TEST(FindEvenFlops, OrdersResultsByClosenessToEven) {
  const auto report = findEvenFlops(hand("AhKs"), hand("QdQc"), {.topCount = 20});

  ASSERT_TRUE(report.has_value()) << report.error();
  EXPECT_TRUE(std::ranges::is_sorted(report->top, {}, [](const FlopResult& r) {
    return r.equity.distanceFromEven();
  }));
}

TEST(FindEvenFlops, FindsAFlopVeryCloseToAExactCoinflip) {
  // With 17,296 candidates there is always something within a hair of 50/50.
  const auto report = findEvenFlops(hand("AhKs"), hand("QdQc"), {.topCount = 1});

  ASSERT_TRUE(report.has_value()) << report.error();
  EXPECT_LT(report->top.front().equity.distanceFromEven(), 0.005);
}

TEST(FindEvenFlops, NeverDealsACardHeldByEitherPlayer) {
  const Hand hero = hand("AhKs");
  const Hand villain = hand("QdQc");

  const auto report = findEvenFlops(hero, villain, {.topCount = 50});

  ASSERT_TRUE(report.has_value()) << report.error();
  for (const FlopResult& result : report->top) {
    for (const Card board : result.flop) {
      EXPECT_NE(board, hero.first());
      EXPECT_NE(board, hero.second());
      EXPECT_NE(board, villain.first());
      EXPECT_NE(board, villain.second());
    }
  }
}

TEST(FindEvenFlops, ProducesIdenticalResultsRegardlessOfThreadCount) {
  // Parallelism must not change the answer or the tie-breaking order.
  const auto single = findEvenFlops(hand("AhKs"), hand("QdQc"), {.topCount = 25, .threadCount = 1});
  const auto many = findEvenFlops(hand("AhKs"), hand("QdQc"), {.topCount = 25, .threadCount = 8});

  ASSERT_TRUE(single.has_value()) << single.error();
  ASSERT_TRUE(many.has_value()) << many.error();
  ASSERT_EQ(single->top.size(), many->top.size());

  for (std::size_t i = 0; i < single->top.size(); ++i) {
    EXPECT_EQ(toString(single->top[i].flop), toString(many->top[i].flop)) << "at " << i;
    EXPECT_EQ(single->top[i].equity.wins, many->top[i].equity.wins) << "at " << i;
    EXPECT_EQ(single->top[i].equity.ties, many->top[i].equity.ties) << "at " << i;
  }
}

TEST(FindEvenFlops, AgreesWithTheStandaloneEquityCalculation) {
  // The threaded search and the simple one-shot path must not drift apart.
  const Hand hero = hand("AhKs");
  const Hand villain = hand("QdQc");

  const auto report = findEvenFlops(hero, villain, {.topCount = 3});

  ASSERT_TRUE(report.has_value()) << report.error();
  for (const FlopResult& result : report->top) {
    const Equity direct = flopEquity(hero, villain, result.flop);
    EXPECT_EQ(direct.wins, result.equity.wins) << toString(result.flop);
    EXPECT_EQ(direct.losses, result.equity.losses) << toString(result.flop);
    EXPECT_EQ(direct.ties, result.equity.ties) << toString(result.flop);
  }
}

TEST(FindEvenFlops, RejectsHandsThatShareACard) {
  const auto report = findEvenFlops(hand("AhKs"), hand("AhQc"), {.topCount = 5});

  ASSERT_FALSE(report.has_value());
  EXPECT_NE(report.error().find("Ah"), std::string::npos);
}

TEST(FindEvenFlops, RejectsNonPositiveTopCount) {
  EXPECT_FALSE(findEvenFlops(hand("AhKs"), hand("QdQc"), {.topCount = 0}).has_value());
}

}  // namespace
}  // namespace even_flop
