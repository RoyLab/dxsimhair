#pragma once
#include <boost/log/trivial.hpp>

#define USE_BOOST_LOGGER

class wrLogger
{
public:
    wrLogger();
    ~wrLogger();

    void init();
    void trace(const char* str);
    void debug(const char* str);
    void info(const char* str);
    void warning(const char* str);
    void error(const char* str);
    void fatal(const char* str);
};

#ifdef USE_BOOST_LOGGER

#define WR_LOG_TRACE BOOST_LOG_TRIVIAL(trace)
#define WR_LOG_DEBUG BOOST_LOG_TRIVIAL(debug)
#define WR_LOG_INFO BOOST_LOG_TRIVIAL(info)
#define WR_LOG_WARNING BOOST_LOG_TRIVIAL(warning)
#define WR_LOG_ERROR BOOST_LOG_TRIVIAL(error)
#define WR_LOG_FATAL BOOST_LOG_TRIVIAL(fatal)

#else

#define WR_LOG_TRACE BOOST_LOG_TRIVIAL(trace)
#define WR_LOG_DEBUG BOOST_LOG_TRIVIAL(debug)
#define WR_LOG_INFO BOOST_LOG_TRIVIAL(info)
#define WR_LOG_WARNING BOOST_LOG_TRIVIAL(warning)
#define WR_LOG_ERROR BOOST_LOG_TRIVIAL(error)
#define WR_LOG_FATAL BOOST_LOG_TRIVIAL(fatal)

#endif