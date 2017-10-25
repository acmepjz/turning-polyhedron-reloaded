#pragma once

#include <osg/Notify>

#define UTIL_DEBUG OSG_DEBUG << "[" __FUNCTION__ "] "
#define UTIL_INFO OSG_INFO << "[" __FUNCTION__ "] "
#define UTIL_NOTICE OSG_NOTICE << "[" __FUNCTION__ "] "
#define UTIL_WARN OSG_WARN << "[" __FUNCTION__ "] Warning: "
#define UTIL_ERR OSG_FATAL << "[" __FUNCTION__ "] Error: "
