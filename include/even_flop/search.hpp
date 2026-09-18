#pragma once

#include <cstddef>
#include <expected>
#include <string>
#include <vector>

#include "even_flop/equity.hpp"
#include "even_flop/hand.hpp"

namespace even_flop {

// C(48,3): every flop that can fall once two hands are dealt.
inline constexpr int TOTAL_FLOPS = 17296;

struct FlopResult {
  Flop flop;
  Equity equity;
};

struct SearchOptions {
  int topCount = 10;
  // 0 means "use the hardware concurrency reported by the platform".
  int threadCount = 0;
};

// C(48,5): every five-card board, each of which shows up C(5,3)=10 times across
// the (flop, turn/river) pairs we enumerate.
inline constexpr int TOTAL_BOARDS = 1712304;
inline constexpr int BOARDS_PER_FLOP_SPLIT = 10;
inline constexpr int TOTAL_RUNOUTS = TOTAL_BOARDS * BOARDS_PER_FLOP_SPLIT;

struct SearchReport {
  int flopsEvaluated = 0;
  // Equity with no board out, i.e. the matchup these hands start from. Summing
  // the per-flop counts gives this exactly and for free: every five-card board
  // appears the same number of times across the flops we already evaluated.
  Equity preflop;
  // Closest to 50/50 first. Ties broken by flop card order so runs reproduce.
  std::vector<FlopResult> top;
};

// How many workers a requested thread count actually becomes. Zero or negative
// means "one per core". Requests above the core count are clamped down: extra
// threads cannot help a CPU-bound enumeration, and a very large request would
// otherwise shrink each chunk to a single flop and start a thread for each one.
int resolveWorkerCount(int requested);

std::expected<SearchReport, std::string> findEvenFlops(
    const Hand& hero, const Hand& villain, const SearchOptions& options);

}  // namespace even_flop
