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

TEST(ResolveWorkerCount, ClampsHugeRequestsToTheCoreCount) {
  // Without this clamp a big --threads value shrinks each chunk to one flop and
  // starts ~17k threads, which is 250x slower than the correct answer.
  const int cores = resolveWorkerCount(0);

  EXPECT_GE(cores, 1);
  EXPECT_EQ(resolveWorkerCount(1'000'000), cores);
  EXPECT_EQ(resolveWorkerCount(TOTAL_FLOPS), cores);
}

TEST(ResolveWorkerCount, TreatsZeroAndNegativeAsOnePerCore) {
  const int cores = resolveWorkerCount(0);

  EXPECT_EQ(resolveWorkerCount(-5), cores);
  EXPECT_EQ(resolveWorkerCount(0), cores);
}

TEST(ResolveWorkerCount, HonoursRequestsBelowTheCoreCount) {
  EXPECT_EQ(resolveWorkerCount(1), 1);
}

TEST(FindEvenFlops, SurvivesAnAbsurdThreadRequest) {
  const auto baseline = findEvenFlops(hand("AhKs"), hand("QdQc"), {.topCount = 3, .threadCount = 1});
  const auto absurd =
      findEvenFlops(hand("AhKs"), hand("QdQc"), {.topCount = 3, .threadCount = 1'000'000});

  ASSERT_TRUE(absurd.has_value()) << absurd.error();
  ASSERT_TRUE(baseline.has_value()) << baseline.error();
  EXPECT_EQ(absurd->flopsEvaluated, TOTAL_FLOPS);
  ASSERT_EQ(absurd->top.size(), baseline->top.size());
  for (std::size_t i = 0; i < absurd->top.size(); ++i) {
    EXPECT_EQ(toString(absurd->top[i].flop), toString(baseline->top[i].flop)) << "at " << i;
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

TEST(Preflop, AggregatedCountsCoverEveryBoardTenTimes) {
  const auto report = findEvenFlops(hand("AhKs"), hand("QdQc"), {.topCount = 1});

  ASSERT_TRUE(report.has_value()) << report.error();
  EXPECT_EQ(report->preflop.runouts(), TOTAL_RUNOUTS);
  EXPECT_EQ(report->preflop.runouts(), TOTAL_FLOPS * 990);
}

TEST(Preflop, MatchesADirectEnumerationOfEveryFiveCardBoard) {
  // Independently enumerate all C(48,5) boards. If this disagrees with the
  // aggregated flop counts then the "each board appears 10 times" assumption
  // behind the free preflop number is wrong.
  const Hand hero = hand("AhKs");
  const Hand villain = hand("QdQc");
  const auto report = findEvenFlops(hero, villain, {.topCount = 1});
  ASSERT_TRUE(report.has_value()) << report.error();

  const std::vector<Card> live = remainingCards(hero, villain);
  Equity direct;
  const std::size_t n = live.size();
  for (std::size_t a = 0; a < n; ++a) {
    for (std::size_t b = a + 1; b < n; ++b) {
      for (std::size_t c = b + 1; c < n; ++c) {
        const Flop board{live[a], live[b], live[c]};
        for (std::size_t d = c + 1; d < n; ++d) {
          for (std::size_t e = d + 1; e < n; ++e) {
            const Equity one = flopEquity(hero, villain, board,
                                          std::array<Card, 2>{live[d], live[e]});
            direct.wins += one.wins;
            direct.losses += one.losses;
            direct.ties += one.ties;
          }
        }
      }
    }
  }

  ASSERT_EQ(direct.runouts(), TOTAL_BOARDS);
  EXPECT_NEAR(direct.hero(), report->preflop.hero(), 1e-12);
}

TEST(Preflop, IsUnaffectedByTheNumberOfResultsRequested) {
  const auto few = findEvenFlops(hand("AhKs"), hand("QdQc"), {.topCount = 1});
  const auto many = findEvenFlops(hand("AhKs"), hand("QdQc"), {.topCount = 50});

  ASSERT_TRUE(few.has_value()) << few.error();
  ASSERT_TRUE(many.has_value()) << many.error();
  EXPECT_EQ(few->preflop.wins, many->preflop.wins);
  EXPECT_EQ(few->preflop.ties, many->preflop.ties);
}

TEST(Preflop, PutsBigPairAheadOfTwoOvercards) {
  // AKo vs QQ is a well-known roughly 43/57 race.
  const auto report = findEvenFlops(hand("AhKs"), hand("QdQc"), {.topCount = 1});

  ASSERT_TRUE(report.has_value()) << report.error();
  EXPECT_GT(report->preflop.hero(), 0.42);
  EXPECT_LT(report->preflop.hero(), 0.44);
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
