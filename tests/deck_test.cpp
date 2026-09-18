#include "even_flop/deck.hpp"

#include <algorithm>
#include <gtest/gtest.h>

namespace even_flop {
namespace {

Hand hand(std::string_view text) { return parseHand(text).value(); }

TEST(RemainingCards, LeavesFortyEightCardsAfterTwoHands) {
  const auto live = remainingCards(hand("AhKs"), hand("QdQc"));

  EXPECT_EQ(live.size(), 48u);
}

TEST(RemainingCards, ExcludesEveryDealtCard) {
  const Hand hero = hand("AhKs");
  const Hand villain = hand("QdQc");

  const auto live = remainingCards(hero, villain);

  for (const Card dealt : {hero.first(), hero.second(), villain.first(), villain.second()}) {
    EXPECT_EQ(std::ranges::find(live, dealt), live.end()) << dealt.toString();
  }
}

TEST(RemainingCards, IsSortedAndFreeOfDuplicates) {
  const auto live = remainingCards(hand("AhKs"), hand("QdQc"));

  EXPECT_TRUE(std::ranges::is_sorted(live));
  EXPECT_EQ(std::ranges::adjacent_find(live), live.end());
}

TEST(RemainingCards, ReturnsWholeDeckWhenNothingIsKnown) {
  const auto live = remainingCards(std::span<const Card>{});

  EXPECT_EQ(live.size(), static_cast<std::size_t>(DECK_SIZE));
}

}  // namespace
}  // namespace even_flop
