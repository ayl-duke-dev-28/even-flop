#pragma once

#include <compare>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace even_flop {

inline constexpr int RANK_COUNT = 13;
inline constexpr int SUIT_COUNT = 4;
inline constexpr int DECK_SIZE = RANK_COUNT * SUIT_COUNT;

// Rank 0..12 maps deuce..ace; suit 0..3 maps clubs, diamonds, hearts, spades.
// Card ids are rank * 4 + suit, which is exactly phevaluator's encoding, so a
// card id can be handed to the evaluator without translation.
class Card {
 public:
  explicit constexpr Card(std::uint8_t id) : id_(id) {}

  constexpr Card(int rank, int suit)
      : id_(static_cast<std::uint8_t>(rank * SUIT_COUNT + suit)) {}

  constexpr std::uint8_t id() const { return id_; }
  constexpr int rank() const { return id_ / SUIT_COUNT; }
  constexpr int suit() const { return id_ % SUIT_COUNT; }

  // Two characters, rank then suit, e.g. "Ah", "Td", "2c".
  std::string toString() const;

  friend constexpr bool operator==(Card, Card) = default;
  friend constexpr std::strong_ordering operator<=>(Card, Card) = default;

 private:
  std::uint8_t id_;
};

// Accepts a two-character card such as "Ah" or "th". Rank and suit letters are
// both case-insensitive. Returns a human-readable message on bad input.
std::expected<Card, std::string> parseCard(std::string_view text);

}  // namespace even_flop
