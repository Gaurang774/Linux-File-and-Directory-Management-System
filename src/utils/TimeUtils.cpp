#include "TimeUtils.hpp"

#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace filemgr {
namespace TimeUtils {

std::string formatTime(time_t t) {
    std::tm tm{};
    localtime_r(&t, &tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string formatTimeLs(time_t t) {
    std::tm fileTm{};
    localtime_r(&t, &fileTm);

    // If within the last 6 months, show time; otherwise show year
    time_t now = std::time(nullptr);
    constexpr time_t sixMonths = 180 * 24 * 60 * 60;

    std::ostringstream oss;
    if (now - t < sixMonths && t <= now) {
        oss << std::put_time(&fileTm, "%b %e %H:%M");
    } else {
        oss << std::put_time(&fileTm, "%b %e  %Y");
    }
    return oss.str();
}

time_t parseDate(std::string_view dateStr) {
    std::tm tm{};   
    std::memset(&tm, 0, sizeof(tm));

    // Expected format: "YYYY-MM-DD"
    std::istringstream iss{std::string(dateStr)};
    iss >> std::get_time(&tm, "%Y-%m-%d");

    if (iss.fail()) return static_cast<time_t>(-1);

    tm.tm_hour = 0;
    tm.tm_min  = 0;
    tm.tm_sec  = 0;
    tm.tm_isdst = -1;  // let mktime determine DST

    return mktime(&tm);
}

std::string timeAgo(time_t t) {
    time_t now = std::time(nullptr);
    auto diff = static_cast<uint64_t>(now - t);

    if (diff < 60)        return std::to_string(diff) + " seconds ago";
    if (diff < 3600)      return std::to_string(diff / 60) + " minutes ago";
    if (diff < 86400)     return std::to_string(diff / 3600) + " hours ago";
    if (diff < 2592000)   return std::to_string(diff / 86400) + " days ago";
    if (diff < 31536000)  return std::to_string(diff / 2592000) + " months ago";
    return std::to_string(diff / 31536000) + " years ago";
}

} // namespace TimeUtils
} // namespace filemgr
