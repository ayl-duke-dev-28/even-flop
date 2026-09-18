#include "even_flop/equity.hpp"

#include <gtest/gtest.h>

namespace even_flop {
namespace {

Hand hand(std::string_view text) { return parseHand(text).value(); }

Flop flop(std::string_view a, std::string_view b, std::string_view c) {
  return Flop{parseCard(a).value(), parseCard(b).value(), parseCard(c).value()};
}

TEST(FlopEquity, EnumeratesEveryTurnAndRiver) {
  // 45 unseen cards choose 2 = 990 runouts.
  const auto equity = flopEquity(hand("AhKs"), hand("QdQc"), flop("2c", "7d", "9h"));

  EXPECT_EQ(equity.runouts(), 990);
}

TEST(FlopEquity, SharesSumToOne) {
  const auto equity = flopEquity(hand("AhKs"), hand("QdQc"), flop("2c", "7d", "9h"));

  EXPECT_DOUBLE_EQ(equity.hero() + equity.villain(), 1.0);
}

TEST(FlopEquity, IsSymmetricWhenHandsSwap) {
  const Hand hero = hand("AhKs");
  const Hand villain = hand("QdQc");
  const Flop board = flop("Qs", "Jh", "Tc");

  const auto forward = flopEquity(hero, villain, board);
  const auto reversed = flopEquity(villain, hero, board);

  EXPECT_DOUBLE_EQ(forward.hero(), reversed.villain());
  EXPECT_EQ(forward.wins, reversed.losses);
  EXPECT_EQ(forward.ties, reversed.ties);
}

TEST(FlopEquity, GivesLockedHandFullEquity) {
  // Hero flops four aces; kings cannot get there on any runout.
  const auto equity = flopEquity(hand("AhAs"), hand("KhKs"), flop("Ac", "Ad", "7c"));

  EXPECT_EQ(equity.wins, 990);
  EXPECT_EQ(equity.losses, 0);
  EXPECT_EQ(equity.ties, 0);
  EXPECT_DOUBLE_EQ(equity.hero(), 1.0);
}

TEST(FlopEquity, TopPairBeatsUnderpair) {
  // AK flops top pair top kicker against queens: a large but not total edge.
  const auto equity = flopEquity(hand("AhKs"), hand("QdQc"), flop("Ac", "7d", "2s"));

  EXPECT_GT(equity.hero(), 0.80);
  EXPECT_LT(equity.hero(), 1.0);
}

TEST(FlopEquity, UnderpairDominatesOnABlankBoard) {
  // Queens on a disconnected low board leave AK drawing to six outs.
  const auto equity = flopEquity(hand("AhKs"), hand("QdQc"), flop("2c", "7d", "9h"));

  EXPECT_LT(equity.hero(), 0.30);
  EXPECT_GT(equity.hero(), 0.0);
}

TEST(FlopEquity, StraightLeadsSetButIsNotLocked) {
  // Hero makes broadway on QJT; villain's set of queens can still boat up.
  const auto equity = flopEquity(hand("AhKs"), hand("QdQc"), flop("Qs", "Jh", "Tc"));

  EXPECT_GT(equity.hero(), 0.50);
  EXPECT_LT(equity.hero(), 0.85);
}

TEST(Equity, DistanceFromEvenIsAbsolute) {
  const Equity favoured{.wins = 990, .losses = 0, .ties = 0};
  const Equity crushed{.wins = 0, .losses = 990, .ties = 0};
  const Equity even{.wins = 495, .losses = 495, .ties = 0};

  EXPECT_DOUBLE_EQ(favoured.distanceFromEven(), 0.5);
  EXPECT_DOUBLE_EQ(crushed.distanceFromEven(), 0.5);
  EXPECT_DOUBLE_EQ(even.distanceFromEven(), 0.0);
}

}  // namespace
}  // namespace even_flop
