#ifndef WERKZEUGKISTE_LOGGING_H
#define WERKZEUGKISTE_LOGGING_H

// NOLINTBEGIN(*macro-usage)

#if __has_include(<spdlog/spdlog.h>) && defined(werkzeugkiste_WITH_SPDLOG)
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
#define WKZLOG_TRACE SPDLOG_TRACE
#define WKZLOG_DEBUG SPDLOG_DEBUG
#define WKZLOG_INFO SPDLOG_INFO
#define WKZLOG_WARN SPDLOG_WARN
#define WKZLOG_ERROR SPDLOG_ERROR
#define WKZLOG_CRITICAL SPDLOG_CRITICAL
#else  // has<spdlog>
#define WKZLOG_TRACE(...) (void)0
#define WKZLOG_DEBUG(...) (void)0
#define WKZLOG_INFO(...) (void)0
#define WKZLOG_WARN(...) (void)0
#define WKZLOG_ERROR(...) (void)0
#define WKZLOG_CRITICAL(...) (void)0
#endif  // has<spdlog>

// NOLINTEND(*macro-usage)

#endif  // WERKZEUGKISTE_LOGGING_H
