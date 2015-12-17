#include "DXUT.h"
#include "wrLogger.h"
#include <boost/log/trivial.hpp>


wrLogger::wrLogger()
{
}


wrLogger::~wrLogger()
{
}


void wrLogger::init()
{
}

void wrLogger::trace(const char* str)
{
    BOOST_LOG_TRIVIAL(trace) << str;
}


void wrLogger::debug(const char* str)
{
    BOOST_LOG_TRIVIAL(debug) << str;
}


void wrLogger::info(const char* str)
{
    BOOST_LOG_TRIVIAL(info) << str;
}


void wrLogger::warning(const char* str)
{
    BOOST_LOG_TRIVIAL(warning) << str;
}


void wrLogger::error(const char* str)
{
    BOOST_LOG_TRIVIAL(error) << str;
}


void wrLogger::fatal(const char* str)
{
    BOOST_LOG_TRIVIAL(fatal) << str;
}
