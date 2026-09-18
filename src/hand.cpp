#include "even_flop/hand.hpp"

#include <format>

namespace even_flop {
namespace {

constexpr int HAND_TEXT_LENGTH = HOLE_CARDS * 2;

bool isSeparator(char c) { return c == ' ' || c == ',' || c == '\t'; }

}  // namespace

std::expected<Hand, std::string> Hand::create(Card first, Card second) {
  if (first == second) {
    return std::unexpected(
        std::format("A hand cannot contain {} twice.", first.toString()));
  }
  return Hand{first, second};
}

std::string Hand::toString() const {
  return cards_[0].toString() + cards_[1].toString();
}

std::expected<Hand, std::string> parseHand(std::string_view text) {
  std::string compact;
  compact.reserve(text.size());
  for (const char c : text) {
    if (!isSeparator(c)) compact.push_back(c);
  }

  if (compact.size() != HAND_TEXT_LENGTH) {
    return std::unexpected(std::format(
        "Hand \"{}\" must be exactly two cards, such as \"AhKs\".", text));
  }

  const std::string_view view{compact};
  const auto first = parseCard(view.substr(0, 2));
  if (!first) return std::unexpected(first.error());

  const auto second = parseCard(view.substr(2, 2));
  if (!second) return std::unexpected(second.error());

  return Hand::create(*first, *second);
}

}  // namespace even_flop
