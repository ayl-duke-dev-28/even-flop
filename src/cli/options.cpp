#include "options.hpp"

#include <charconv>
#include <format>
#include <vector>

namespace even_flop::cli {
namespace {

constexpr int DEFAULT_TOP_COUNT = 10;

std::expected<int, std::string> parseCount(std::string_view text, std::string_view flag) {
  int value = 0;
  const char* end = text.data() + text.size();
  const auto [stop, error] = std::from_chars(text.data(), end, value);
  if (error != std::errc{} || stop != end) {
    return std::unexpected(std::format("{} expects a whole number, got \"{}\".", flag, text));
  }
  if (value < 1) {
    return std::unexpected(std::format("{} must be at least 1, got {}.", flag, value));
  }
  return value;
}

}  // namespace

bool wantsHelp(int argc, const char* const* argv) {
  for (int i = 1; i < argc; ++i) {
    const std::string_view argument{argv[i]};
    if (argument == "-h" || argument == "--help") return true;
  }
  return argc <= 1;
}

std::string usage() {
  return
      "even-flop - find the flop that splits equity closest to 50/50\n"
      "\n"
      "Usage:\n"
      "  even-flop <hand> <hand> [options]\n"
      "\n"
      "Hands are two cards each, for example AhKs or \"Ah Ks\".\n"
      "Ranks are 23456789TJQKA and suits are cdhs, both case-insensitive.\n"
      "\n"
      "Options:\n"
      "  -n, --top <count>      How many flops to list (default 10)\n"
      "  -j, --threads <count>  Worker threads (default: one per core)\n"
      "      --json             Emit JSON instead of a table\n"
      "  -h, --help             Show this message\n"
      "\n"
      "Example:\n"
      "  even-flop AhKs QdQc --top 5\n";
}

std::expected<Options, std::string> parseArguments(int argc, const char* const* argv) {
  std::vector<std::string_view> positional;
  int topCount = DEFAULT_TOP_COUNT;
  int threadCount = 0;
  bool asJson = false;

  for (int i = 1; i < argc; ++i) {
    const std::string_view argument{argv[i]};

    if (argument == "--json") {
      asJson = true;
      continue;
    }

    if (argument == "-n" || argument == "--top" || argument == "-j" || argument == "--threads") {
      if (i + 1 >= argc) {
        return std::unexpected(std::format("{} needs a value.", argument));
      }
      const auto value = parseCount(argv[++i], argument);
      if (!value) return std::unexpected(value.error());

      if (argument == "-n" || argument == "--top") {
        topCount = *value;
      } else {
        threadCount = *value;
      }
      continue;
    }

    if (argument.starts_with('-') && argument.size() > 1) {
      return std::unexpected(std::format("Unknown option {}. Try --help.", argument));
    }

    positional.push_back(argument);
  }

  if (positional.size() != 2) {
    return std::unexpected(std::format(
        "Expected exactly two hands, got {}. Try: even-flop AhKs QdQc", positional.size()));
  }

  const auto hero = parseHand(positional[0]);
  if (!hero) return std::unexpected(hero.error());

  const auto villain = parseHand(positional[1]);
  if (!villain) return std::unexpected(villain.error());

  return Options{
      .hero = *hero,
      .villain = *villain,
      .search = SearchOptions{.topCount = topCount, .threadCount = threadCount},
      .asJson = asJson,
  };
}

}  // namespace even_flop::cli
