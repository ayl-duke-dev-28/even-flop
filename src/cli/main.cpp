#include <chrono>
#include <cstdio>
#include <print>

#include "even_flop/search.hpp"
#include "options.hpp"
#include "report.hpp"

int main(int argc, char** argv) {
  using namespace even_flop;

  if (cli::wantsHelp(argc, argv)) {
    std::print("{}", cli::usage());
    return 0;
  }

  const auto options = cli::parseArguments(argc, argv);
  if (!options) {
    std::println(stderr, "even-flop: {}", options.error());
    std::println(stderr, "Try 'even-flop --help' for usage.");
    return 2;
  }

  const auto started = std::chrono::steady_clock::now();
  const auto report = findEvenFlops(options->hero, options->villain, options->search);
  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - started);

  if (!report) {
    std::println(stderr, "even-flop: {}", report.error());
    return 1;
  }

  const cli::ReportContext context{
      .hero = options->hero, .villain = options->villain, .elapsed = elapsed};

  std::print("{}", options->asJson ? cli::formatJson(*report, context)
                                   : cli::formatTable(*report, context));
  return 0;
}
