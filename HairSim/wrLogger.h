#pragma once
#include "macros.h"

#define USE_BOOST_LOGGER

#ifdef USE_BOOST_LOGGER

#include <boost\log\trivial.hpp>

#define WR_LOG_TRACE BOOST_LOG_TRIVIAL(trace)
#define WR_LOG_DEBUG BOOST_LOG_TRIVIAL(debug)
#define WR_LOG_INFO BOOST_LOG_TRIVIAL(info)
#define WR_LOG_WARNING BOOST_LOG_TRIVIAL(warning)
#define WR_LOG_ERROR BOOST_LOG_TRIVIAL(error)
#define WR_LOG_FATAL BOOST_LOG_TRIVIAL(fatal)

#else

#include <sstream>

class wrLogger
{
    template <class T>
    friend wrLogger& operator << (wrLogger& thiz, const T& val);

public:
    wrLogger& __CLR_OR_THIS_CALL operator<<(std::wostream& (__cdecl *_Pfn)(std::wostream&))
    {
        s << _Pfn;
        OutputDebugString(s.str().c_str());
        s.str(L"");
        s.clear();
        return *this;
    }

    wrLogger& operator << (std::wios& (__cdecl *_Pfn)(std::wios&))
    {
        s << _Pfn;
        OutputDebugString(s.str().c_str());
        s.str(L"");
        s.clear();
        return *this;
    }

    wrLogger& operator << (std::ios_base& (__cdecl *_Pfn)(std::ios_base&))
    {
        s << _Pfn;
        OutputDebugString(s.str().c_str());
        s.str(L"");
        s.clear();
        return *this;
    }

private:
    std::wstringstream s;
};


template <class T>
inline wrLogger& operator << (wrLogger& thiz, const T& val)
{
    thiz.s << val;
    OutputDebugString(thiz.s.str().c_str());
    thiz.s.str(L"");
    return thiz;
}

extern wrLogger _g_DebugLogger;

const char LOGGER_PREFIX_trace[] = "trace: ";
const char LOGGER_PREFIX_debug[] = "debug: ";
const char LOGGER_PREFIX_info[] = "info: ";
const char LOGGER_PREFIX_warning[] = "warning: ";
const char LOGGER_PREFIX_error[] = "error: ";
const char LOGGER_PREFIX_fatal[] = "fatal: ";

#define WR_LOG_DEBUG_HELPER(level) _g_DebugLogger << "\n[log] " << LOGGER_PREFIX_##trace

#define WR_LOG_TRACE WR_LOG_DEBUG_HELPER(trace)
#define WR_LOG_DEBUG WR_LOG_DEBUG_HELPER(debug)
#define WR_LOG_INFO WR_LOG_DEBUG_HELPER(info)
#define WR_LOG_WARNING WR_LOG_DEBUG_HELPER(warning)
#define WR_LOG_ERROR WR_LOG_DEBUG_HELPER(error)
#define WR_LOG_FATAL WR_LOG_DEBUG_HELPER(fatal)

#endif

#define UNIMPLEMENTED_DECLARATION WR_LOG_ERROR << UNIMPLEMENTED_METHOD << __FUNCTION__