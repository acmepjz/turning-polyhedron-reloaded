#pragma once

#include <osg/Notify>

#define UTIL_INFO OSG_NOTICE << "[" __FUNCTION__ "] "
#define UTIL_WARN OSG_NOTICE << "[" __FUNCTION__ "] Warning: "
#define UTIL_ERR OSG_NOTICE << "[" __FUNCTION__ "] Error: "
