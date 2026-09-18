#include "even_flop/equity.hpp"

#include <phevaluator/phevaluator.h>

#include <array>
#include <vector>

#include "even_flop/deck.hpp"

namespace even_flop {

std::string toString(const Flop& flop) {
  return flop[0].toString() + flop[1].toString() + flop[2].toString();
}

Equity flopEquity(const Hand& hero, const Hand& villain, const Flop& flop,
                  std::span<const Card> deck) {
  // Pulled out of the loop: these are the five cards that never change while we
  // enumerate turn/river, and phevaluator shares our rank*4+suit card encoding.
  const int hero1 = hero.first().id();
  const int hero2 = hero.second().id();
  const int villain1 = villain.first().id();
  const int villain2 = villain.second().id();
  const int board1 = flop[0].id();
  const int board2 = flop[1].id();
  const int board3 = flop[2].id();

  Equity equity;
  const std::size_t size = deck.size();
  for (std::size_t i = 0; i < size; ++i) {
    const int turn = deck[i].id();
    for (std::size_t j = i + 1; j < size; ++j) {
      const int river = deck[j].id();

      // phevaluator ranks are 1..7462 with 1 the strongest hand.
      const int heroRank =
          phevaluator::EvaluateCards(hero1, hero2, board1, board2, board3, turn, river).value();
      const int villainRank =
          phevaluator::EvaluateCards(villain1, villain2, board1, board2, board3, turn, river).value();

      if (heroRank < villainRank) {
        ++equity.wins;
      } else if (heroRank > villainRank) {
        ++equity.losses;
      } else {
        ++equity.ties;
      }
    }
  }
  return equity;
}

Equity flopEquity(const Hand& hero, const Hand& villain, const Flop& flop) {
  const std::array<Card, 2 * HOLE_CARDS + FLOP_CARDS> known{
      hero.first(), hero.second(), villain.first(), villain.second(),
      flop[0],      flop[1],       flop[2]};
  const std::vector<Card> deck = remainingCards(known);
  return flopEquity(hero, villain, flop, deck);
}

}  // namespace even_flop
