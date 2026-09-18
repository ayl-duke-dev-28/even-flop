#include "even_flop/card.hpp"

#include <cctype>
#include <format>

namespace even_flop {
namespace {

constexpr std::string_view RANK_CHARS = "23456789TJQKA";
constexpr std::string_view SUIT_CHARS = "cdhs";

int indexOf(std::string_view chars, char needle) {
  const auto position = chars.find(needle);
  return position == std::string_view::npos ? -1 : static_cast<int>(position);
}

char upper(char c) { return static_cast<char>(std::toupper(static_cast<unsigned char>(c))); }
char lower(char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }

}  // namespace

std::string Card::toString() const {
  return std::string{RANK_CHARS[static_cast<std::size_t>(rank())],
                     SUIT_CHARS[static_cast<std::size_t>(suit())]};
}

std::expected<Card, std::string> parseCard(std::string_view text) {
  if (text.size() != 2) {
    return std::unexpected(std::format(
        "Card \"{}\" must be exactly two characters, such as \"Ah\".", text));
  }

  const int rank = indexOf(RANK_CHARS, upper(text[0]));
  if (rank < 0) {
    return std::unexpected(std::format(
        "Unknown rank '{}' in \"{}\". Expected one of {}.", text[0], text, RANK_CHARS));
  }

  const int suit = indexOf(SUIT_CHARS, lower(text[1]));
  if (suit < 0) {
    return std::unexpected(std::format(
        "Unknown suit '{}' in \"{}\". Expected one of {}.", text[1], text, SUIT_CHARS));
  }

  return Card{rank, suit};
}

}  // namespace even_flop
