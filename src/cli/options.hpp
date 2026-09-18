#pragma once

#include <expected>
#include <string>
#include <string_view>

#include "even_flop/hand.hpp"
#include "even_flop/search.hpp"

namespace even_flop::cli {

struct Options {
  Hand hero;
  Hand villain;
  SearchOptions search;
  bool asJson = false;
};

// True when the arguments ask for usage text rather than a search.
bool wantsHelp(int argc, const char* const* argv);

std::string usage();

std::expected<Options, std::string> parseArguments(int argc, const char* const* argv);

}  // namespace even_flop::cli
