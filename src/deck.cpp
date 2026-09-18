#include "even_flop/deck.hpp"

#include <array>

namespace even_flop {

std::vector<Card> remainingCards(std::span<const Card> known) {
  std::array<bool, DECK_SIZE> isDealt{};
  for (const Card card : known) {
    isDealt[card.id()] = true;
  }

  std::vector<Card> live;
  live.reserve(DECK_SIZE - known.size());
  for (int id = 0; id < DECK_SIZE; ++id) {
    if (!isDealt[static_cast<std::size_t>(id)]) {
      live.emplace_back(static_cast<std::uint8_t>(id));
    }
  }
  return live;
}

std::vector<Card> remainingCards(const Hand& hero, const Hand& villain) {
  const std::array<Card, 2 * HOLE_CARDS> known{
      hero.first(), hero.second(), villain.first(), villain.second()};
  return remainingCards(known);
}

}  // namespace even_flop
