// WebAssembly entry point. Deliberately thin: it parses input, delegates to the
// same engine the CLI uses, and reuses formatJson so the browser and the
// terminal can never disagree about a number.
#include <emscripten/bind.h>

#include <chrono>
#include <format>
#include <string>

#include "even_flop/hand.hpp"
#include "even_flop/search.hpp"
#include "report.hpp"

namespace {

std::string errorJson(std::string_view message) {
  return std::format("{{\"error\": \"{}\"}}\n", even_flop::cli::escapeJsonString(message));
}

}  // namespace

// Returns the same JSON document the CLI's --json flag produces, or
// {"error": "..."} when the hands cannot be used.
std::string searchFlops(const std::string& heroText, const std::string& villainText,
                        int topCount) {
  using namespace even_flop;

  const auto hero = parseHand(heroText);
  if (!hero) return errorJson(hero.error());

  const auto villain = parseHand(villainText);
  if (!villain) return errorJson(villain.error());

  const auto started = std::chrono::steady_clock::now();
  // No pthreads in this build, so the engine runs its inline single-worker path.
  const auto report = findEvenFlops(*hero, *villain, {.topCount = topCount, .threadCount = 1});
  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - started);

  if (!report) return errorJson(report.error());

  return cli::formatJson(*report,
                         cli::ReportContext{.hero = *hero, .villain = *villain, .elapsed = elapsed});
}

EMSCRIPTEN_BINDINGS(even_flop_module) {
  emscripten::function("searchFlops", &searchFlops);
}
