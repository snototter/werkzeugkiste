#ifndef WERKZEUGKISTE_LOGGING_H
#define WERKZEUGKISTE_LOGGING_H

// NOLINTBEGIN(*macro-usage)

#if __has_include(<spdlog/spdlog.h>) && defined(werkzeugkiste_WITH_SPDLOG)
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
#define WZKLOG_TRACE SPDLOG_TRACE
#define WZKLOG_DEBUG SPDLOG_DEBUG
#define WZKLOG_INFO SPDLOG_INFO
#define WZKLOG_WARN SPDLOG_WARN
#define WZKLOG_ERROR SPDLOG_ERROR
#define WZKLOG_CRITICAL SPDLOG_CRITICAL
#else  // has<spdlog>
#define WZKLOG_TRACE(...) (void)0
#define WZKLOG_DEBUG(...) (void)0
#define WZKLOG_INFO(...) (void)0
#define WZKLOG_WARN(...) (void)0
#define WZKLOG_ERROR(...) (void)0
#define WZKLOG_CRITICAL(...) (void)0
#endif  // has<spdlog>

// NOLINTEND(*macro-usage)

#endif  // WERKZEUGKISTE_LOGGING_H
