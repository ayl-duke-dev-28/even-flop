#include "report.hpp"

#include <format>

#include "even_flop/equity.hpp"

namespace even_flop::cli {
namespace {

// Flop cards are printed space-separated ("2c 5d 9h") so boards stay readable.
std::string spacedFlop(const Flop& flop) {
  return std::format("{} {} {}", flop[0].toString(), flop[1].toString(), flop[2].toString());
}

std::string asPercent(double share) { return std::format("{:.2f}%", share * 100.0); }

}  // namespace

std::string formatTable(const SearchReport& report, const ReportContext& context) {
  const std::string heroName = context.hero.toString();
  const std::string villainName = context.villain.toString();

  std::string out = std::format(
      "{} vs {}\n{} flops evaluated in {} ms\n\n",
      heroName, villainName, report.flopsEvaluated, context.elapsed.count());

  out += std::format("Preflop          {:>8}  {:>8}   ({} off even)\n",
                     asPercent(report.preflop.hero()),
                     asPercent(report.preflop.villain()),
                     asPercent(report.preflop.distanceFromEven()));

  if (!report.top.empty()) {
    const FlopResult& best = report.top.front();
    out += std::format("Most even flop   {:>8}  {:>8}   ({} off even, on {})\n",
                       asPercent(best.equity.hero()),
                       asPercent(best.equity.villain()),
                       asPercent(best.equity.distanceFromEven()),
                       spacedFlop(best.flop));
  }
  out += "\n";

  out += std::format("{:>3}  {:<10}  {:>8}  {:>8}  {:>7}\n",
                     "#", "Flop", heroName, villainName, "Off");
  out += std::format("{:->3}  {:-<10}  {:->8}  {:->8}  {:->7}\n", "", "", "", "", "");

  int position = 1;
  for (const FlopResult& result : report.top) {
    out += std::format("{:>3}  {:<10}  {:>8}  {:>8}  {:>7}\n",
                       position++,
                       spacedFlop(result.flop),
                       asPercent(result.equity.hero()),
                       asPercent(result.equity.villain()),
                       asPercent(result.equity.distanceFromEven()));
  }
  return out;
}

std::string formatJson(const SearchReport& report, const ReportContext& context) {
  std::string out = std::format(
      "{{\n  \"hero\": \"{}\",\n  \"villain\": \"{}\",\n"
      "  \"flopsEvaluated\": {},\n  \"elapsedMs\": {},\n"
      "  \"preflop\": {{\"heroEquity\": {:.6f}, \"villainEquity\": {:.6f}, "
      "\"wins\": {}, \"losses\": {}, \"ties\": {}, \"runouts\": {}}},\n"
      "  \"results\": [\n",
      context.hero.toString(), context.villain.toString(),
      report.flopsEvaluated, context.elapsed.count(),
      report.preflop.hero(), report.preflop.villain(), report.preflop.wins,
      report.preflop.losses, report.preflop.ties, report.preflop.runouts());

  for (std::size_t i = 0; i < report.top.size(); ++i) {
    const FlopResult& result = report.top[i];
    out += std::format(
        "    {{\"flop\": \"{}\", \"heroEquity\": {:.6f}, \"villainEquity\": {:.6f}, "
        "\"wins\": {}, \"losses\": {}, \"ties\": {}, \"runouts\": {}}}{}\n",
        toString(result.flop), result.equity.hero(), result.equity.villain(),
        result.equity.wins, result.equity.losses, result.equity.ties,
        result.equity.runouts(), i + 1 < report.top.size() ? "," : "");
  }

  out += "  ]\n}\n";
  return out;
}

}  // namespace even_flop::cli
