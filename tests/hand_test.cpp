#include "even_flop/hand.hpp"

#include <gtest/gtest.h>

namespace even_flop {
namespace {

Card card(std::string_view text) { return parseCard(text).value(); }

TEST(ParseHand, ParsesFourCharacterForm) {
  const auto hand = parseHand("AhKs");

  ASSERT_TRUE(hand.has_value()) << hand.error();
  EXPECT_EQ(hand->first(), card("Ah"));
  EXPECT_EQ(hand->second(), card("Ks"));
}

TEST(ParseHand, AcceptsSpaceAndCommaSeparators) {
  const auto spaced = parseHand("Ah Ks");
  const auto comma = parseHand("Ah,Ks");

  ASSERT_TRUE(spaced.has_value()) << spaced.error();
  ASSERT_TRUE(comma.has_value()) << comma.error();
  EXPECT_EQ(spaced->toString(), "AhKs");
  EXPECT_EQ(comma->toString(), "AhKs");
}

TEST(ParseHand, RejectsDuplicateCards) {
  const auto hand = parseHand("AhAh");

  ASSERT_FALSE(hand.has_value());
  EXPECT_NE(hand.error().find("Ah"), std::string::npos);
}

TEST(ParseHand, RejectsMalformedInput) {
  EXPECT_FALSE(parseHand("AhK").has_value());
  EXPECT_FALSE(parseHand("AhKsQd").has_value());
  EXPECT_FALSE(parseHand("").has_value());
  EXPECT_FALSE(parseHand("AhXs").has_value());
}

TEST(Hand, CreateRejectsDuplicateCards) {
  const auto hand = Hand::create(card("Qd"), card("Qd"));

  EXPECT_FALSE(hand.has_value());
}

TEST(Hand, RoundTripsThroughString) {
  const auto hand = parseHand("Td9c");

  ASSERT_TRUE(hand.has_value()) << hand.error();
  EXPECT_EQ(hand->toString(), "Td9c");
}

}  // namespace
}  // namespace even_flop
