#pragma once

#include <array>
#include <span>
#include <string>

#include "even_flop/card.hpp"
#include "even_flop/hand.hpp"

namespace even_flop {

inline constexpr int FLOP_CARDS = 3;
inline constexpr int BOARD_CARDS = 5;

using Flop = std::array<Card, FLOP_CARDS>;

std::string toString(const Flop& flop);

// Exact equity for one flop, enumerating every turn/river combination.
// A chopped runout counts a half win for each player, so `hero + villain == 1`.
struct Equity {
  int wins = 0;
  int losses = 0;
  int ties = 0;

  constexpr int runouts() const { return wins + losses + ties; }
  constexpr double hero() const {
    const int total = runouts();
    return total == 0 ? 0.0 : (wins + 0.5 * ties) / total;
  }
  constexpr double villain() const { return 1.0 - hero(); }

  // How far this flop lands from a perfect coinflip, in equity share (0..0.5).
  constexpr double distanceFromEven() const {
    const double d = hero() - 0.5;
    return d < 0 ? -d : d;
  }
};

// `deck` must be the live cards with the two hands AND the flop already removed
// (the 45 cards that can still come). Provided by the caller so the hot loop
// never rebuilds it.
Equity flopEquity(const Hand& hero, const Hand& villain, const Flop& flop,
                  std::span<const Card> deck);

// Convenience overload that derives the 45-card runout deck itself. Correct but
// slower; intended for tests and one-off queries rather than the full search.
Equity flopEquity(const Hand& hero, const Hand& villain, const Flop& flop);

}  // namespace even_flop
