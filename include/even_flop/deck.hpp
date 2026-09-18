#pragma once

#include <span>
#include <vector>

#include "even_flop/card.hpp"
#include "even_flop/hand.hpp"

namespace even_flop {

// Cards of the full deck that are not among `known`, ascending by id.
std::vector<Card> remainingCards(std::span<const Card> known);

// Convenience overload for the two-hand case: returns the 48 live cards.
std::vector<Card> remainingCards(const Hand& hero, const Hand& villain);

}  // namespace even_flop
