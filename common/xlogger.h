#pragma once
#include <boost\log\trivial.hpp>

#define XLOG_TRACE BOOST_LOG_TRIVIAL(trace)
#define XLOG_DEBUG BOOST_LOG_TRIVIAL(debug)
#define XLOG_INFO BOOST_LOG_TRIVIAL(info)
#define XLOG_WARNING BOOST_LOG_TRIVIAL(warning)
#define XLOG_ERROR BOOST_LOG_TRIVIAL(error)
#define XLOG_FATAL BOOST_LOG_TRIVIAL(fatal)


