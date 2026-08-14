#pragma once

///////////////////////////////////////////////////////////////////////////////
// TimeUtils.hpp — Time formatting and parsing utilities
///////////////////////////////////////////////////////////////////////////////

#include <ctime>
#include <string>
#include <string_view>

namespace filemgr {
namespace TimeUtils {

/// Format time_t as "YYYY-MM-DD HH:MM:SS"
std::string formatTime(time_t t);

/// Format time_t as "Mar 15 14:30" (ls-style, current year) or "Mar 15  2023"
std::string formatTimeLs(time_t t);

/// Parse date string "YYYY-MM-DD" to time_t (midnight).
/// Returns -1 on parse failure.
time_t parseDate(std::string_view dateStr);

/// Human-readable relative time: "3 hours ago", "2 days ago"
std::string timeAgo(time_t t);

} // namespace TimeUtils
} // namespace filemgr
