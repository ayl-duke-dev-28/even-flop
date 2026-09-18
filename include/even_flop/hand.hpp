#pragma once

#include <array>
#include <expected>
#include <string>
#include <string_view>

#include "even_flop/card.hpp"

namespace even_flop {

inline constexpr int HOLE_CARDS = 2;

// A two-card Texas Hold'em starting hand. Construction guarantees the two cards
// are distinct, so downstream code never has to re-check that.
class Hand {
 public:
  static std::expected<Hand, std::string> create(Card first, Card second);

  constexpr Card first() const { return cards_[0]; }
  constexpr Card second() const { return cards_[1]; }
  constexpr const std::array<Card, HOLE_CARDS>& cards() const { return cards_; }

  std::string toString() const;

 private:
  constexpr Hand(Card first, Card second) : cards_{first, second} {}

  std::array<Card, HOLE_CARDS> cards_;
};

// Parses a four-character hand such as "AhKs", optionally with a separating
// space or comma ("Ah Ks", "Ah,Ks").
std::expected<Hand, std::string> parseHand(std::string_view text);

}  // namespace even_flop
