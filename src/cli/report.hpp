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

// Escapes a string for embedding in a JSON document. Error messages quote the
// user's own input back at them, so they can contain quotes and backslashes.
std::string escapeJsonString(std::string_view text);

}  // namespace even_flop::cli
