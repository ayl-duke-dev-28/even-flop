#include "even_flop/search.hpp"

#include <algorithm>
#include <format>
#include <span>
#include <thread>
#include <vector>

#include "even_flop/deck.hpp"

namespace even_flop {
namespace {

std::expected<void, std::string> requireDistinctHands(const Hand& hero, const Hand& villain) {
  for (const Card heroCard : hero.cards()) {
    for (const Card villainCard : villain.cards()) {
      if (heroCard == villainCard) {
        return std::unexpected(std::format(
            "Both hands contain {}. Each card can only be dealt once.", heroCard.toString()));
      }
    }
  }
  return {};
}

std::vector<Flop> buildFlops(std::span<const Card> live) {
  std::vector<Flop> flops;
  flops.reserve(TOTAL_FLOPS);
  const std::size_t size = live.size();
  for (std::size_t i = 0; i < size; ++i) {
    for (std::size_t j = i + 1; j < size; ++j) {
      for (std::size_t k = j + 1; k < size; ++k) {
        flops.push_back(Flop{live[i], live[j], live[k]});
      }
    }
  }
  return flops;
}



// Closest to even first. Flops that tie on equity fall back to card order, which
// makes the ranking total and therefore identical on every run and thread count.
bool isCloserToEven(const FlopResult& lhs, const FlopResult& rhs) {
  const double left = lhs.equity.distanceFromEven();
  const double right = rhs.equity.distanceFromEven();
  if (left != right) return left < right;
  return lhs.flop < rhs.flop;
}

}  // namespace

int resolveWorkerCount(int requested) {
  const unsigned int detected = std::thread::hardware_concurrency();
  const int available = detected == 0 ? 1 : static_cast<int>(detected);
  if (requested <= 0) return available;
  return std::min(requested, available);
}

std::expected<SearchReport, std::string> findEvenFlops(
    const Hand& hero, const Hand& villain, const SearchOptions& options) {
  if (options.topCount <= 0) {
    return std::unexpected(std::format(
        "Asked for {} results; the count must be at least 1.", options.topCount));
  }
  if (const auto distinct = requireDistinctHands(hero, villain); !distinct) {
    return std::unexpected(distinct.error());
  }

  const std::vector<Card> live = remainingCards(hero, villain);
  const std::vector<Flop> flops = buildFlops(live);

  // Each worker owns a disjoint index range, so the shared vector needs no
  // locking and the results land in a fixed, reproducible order.
  std::vector<Equity> equities(flops.size());

  const auto evaluateRange = [&](std::size_t begin, std::size_t end) {
    std::vector<Card> runoutDeck;
    runoutDeck.reserve(live.size());
    for (std::size_t index = begin; index < end; ++index) {
      const Flop& flop = flops[index];
      runoutDeck.clear();
      for (const Card card : live) {
        if (card != flop[0] && card != flop[1] && card != flop[2]) {
          runoutDeck.push_back(card);
        }
      }
      equities[index] = flopEquity(hero, villain, flop, runoutDeck);
    }
  };

  const auto requested = static_cast<std::size_t>(resolveWorkerCount(options.threadCount));
  const std::size_t threadCount = std::max<std::size_t>(1, std::min(requested, flops.size()));

  if (threadCount == 1) {
    // Run inline rather than spawning a single worker. Besides saving the
    // thread, this is the path a WebAssembly build without pthreads takes,
    // where std::jthread is unavailable.
    evaluateRange(0, flops.size());
  } else {
    const std::size_t chunkSize = (flops.size() + threadCount - 1) / threadCount;
    std::vector<std::jthread> workers;
    workers.reserve(threadCount);
    for (std::size_t begin = 0; begin < flops.size(); begin += chunkSize) {
      const std::size_t end = std::min(begin + chunkSize, flops.size());
      workers.emplace_back(evaluateRange, begin, end);
    }
  }  // jthreads join here.

  std::vector<FlopResult> results;
  results.reserve(flops.size());
  for (std::size_t index = 0; index < flops.size(); ++index) {
    results.push_back(FlopResult{.flop = flops[index], .equity = equities[index]});
  }

  const auto topCount = std::min(static_cast<std::size_t>(options.topCount), results.size());
  std::partial_sort(results.begin(), results.begin() + static_cast<std::ptrdiff_t>(topCount),
                    results.end(), isCloserToEven);

  // Every five-card board is reachable through C(5,3)=10 distinct (flop,
  // turn/river) splits, and equally so, which makes these totals an exact
  // preflop equity rather than an approximation of one.
  Equity preflop;
  for (const Equity& equity : equities) {
    preflop.wins += equity.wins;
    preflop.losses += equity.losses;
    preflop.ties += equity.ties;
  }

  SearchReport report;
  report.preflop = preflop;
  report.flopsEvaluated = static_cast<int>(results.size());
  report.top.assign(results.begin(), results.begin() + static_cast<std::ptrdiff_t>(topCount));
  return report;
}

}  // namespace even_flop
