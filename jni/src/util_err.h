#pragma once

#include <osg/Notify>

#define UTIL_DEBUG OSG_DEBUG << "[" __FUNCTION__ "] "
#define UTIL_INFO OSG_NOTICE << "[" __FUNCTION__ "] "
#define UTIL_WARN OSG_WARN << "[" __FUNCTION__ "] Warning: "
#define UTIL_ERR OSG_WARN << "[" __FUNCTION__ "] Error: "
