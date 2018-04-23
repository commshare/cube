#ifndef LOG_H_
#define LOG_H_

#include "logger.h"
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/attributes/named_scope.hpp>


//stlog define
#define BOOST_SLOG(stlog_lvl) \
    BOOST_LOG_FUNCTION();\
    BOOST_LOG_SEV(light::Logger::Instance().GetMt(), stlog_lvl)

#define STLOG_TRACE     BOOST_SLOG(trace)
#define STLOG_DEBUG     BOOST_SLOG(debug)
#define STLOG_INFO      BOOST_SLOG(info)
#define STLOG_WARN      BOOST_SLOG(warn)
#define STLOG_ERROR     BOOST_SLOG(error)
#define STLOG_FATAL     BOOST_SLOG(fatal)

#endif