#include "even_flop/card.hpp"

#include <gtest/gtest.h>

namespace even_flop {
namespace {

TEST(ParseCard, ParsesRankAndSuit) {
  // Arrange / Act
  const auto card = parseCard("Ah");

  // Assert
  ASSERT_TRUE(card.has_value()) << card.error();
  EXPECT_EQ(card->rank(), 12);
  EXPECT_EQ(card->suit(), 2);
}

TEST(ParseCard, ParsesLowestCard) {
  const auto card = parseCard("2c");

  ASSERT_TRUE(card.has_value()) << card.error();
  EXPECT_EQ(card->rank(), 0);
  EXPECT_EQ(card->suit(), 0);
  EXPECT_EQ(card->id(), 0);
}

TEST(ParseCard, IsCaseInsensitive) {
  const auto upper = parseCard("TD");
  const auto lower = parseCard("td");

  ASSERT_TRUE(upper.has_value()) << upper.error();
  ASSERT_TRUE(lower.has_value()) << lower.error();
  EXPECT_EQ(*upper, *lower);
}

TEST(ParseCard, RejectsUnknownRank) {
  const auto card = parseCard("1h");

  ASSERT_FALSE(card.has_value());
  EXPECT_NE(card.error().find('1'), std::string::npos);
}

TEST(ParseCard, RejectsUnknownSuit) {
  const auto card = parseCard("Ax");

  ASSERT_FALSE(card.has_value());
  EXPECT_NE(card.error().find('x'), std::string::npos);
}

TEST(ParseCard, RejectsWrongLength) {
  EXPECT_FALSE(parseCard("A").has_value());
  EXPECT_FALSE(parseCard("Ahh").has_value());
  EXPECT_FALSE(parseCard("").has_value());
}

TEST(Card, RoundTripsThroughString) {
  // Every card in the deck must survive toString -> parseCard unchanged.
  for (int id = 0; id < DECK_SIZE; ++id) {
    const Card original{static_cast<std::uint8_t>(id)};
    const auto reparsed = parseCard(original.toString());

    ASSERT_TRUE(reparsed.has_value()) << original.toString();
    EXPECT_EQ(*reparsed, original) << original.toString();
  }
}

TEST(Card, IdIsRankTimesFourPlusSuit) {
  // This encoding is what lets us pass ids straight to phevaluator.
  const Card card{7, 3};

  EXPECT_EQ(card.id(), 31);
  EXPECT_EQ(card.rank(), 7);
  EXPECT_EQ(card.suit(), 3);
}

}  // namespace
}  // namespace even_flop
