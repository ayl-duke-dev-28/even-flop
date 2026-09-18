#pragma once

#include <chrono>
#include <string>

#include "even_flop/hand.hpp"
#include "even_flop/search.hpp"

namespace even_flop::cli {

struct ReportContext {
  Hand hero;
  Hand villain;
  std::chrono::milliseconds elapsed{0};
};

std::string formatTable(const SearchReport& report, const ReportContext& context);

std::string formatJson(const SearchReport& report, const ReportContext& context);

}  // namespace even_flop::cli
