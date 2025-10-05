#pragma once
#ifndef _H_INCLUDES
#define _H_INCLUDES

#include <string>
#include <sstream>
#include <random>
#include <vector>
#include <cmath>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <exception>
#include <algorithm>
#include <cfloat>
#include <map>
#include <queue>
#include <limits>
#include <tuple>

using namespace std;
using namespace std::chrono;

#define GET_CURRENT_TIMESTAMP (std::chrono::high_resolution_clock::now())
#define GET_EMPTY_TIMESTAMP std::chrono::_V2::system_clock::time_point()
#define TIMESTAMP std::chrono::_V2::system_clock::time_point
#define TIMESTAMP_ELAPSED_MS(start, end, ret)                            \
    {                                                                    \
        std::chrono::duration<double, std::milli> elapsed = end - start; \
        ret = elapsed.count();                                           \
    }
#define EXECUTIVE_OPERATION(op, time)                    \
    {                                                    \
        auto __beforeT = GET_CURRENT_TIMESTAMP;          \
        op();                                            \
        auto __afterT = GET_CURRENT_TIMESTAMP;           \
        TIMESTAMP_ELAPSED_MS(__beforeT, __afterT, time); \
    }
#define EXECUTIVE_OPERATION_RETURN(op, time, result)     \
    {                                                    \
        auto __beforeT = GET_CURRENT_TIMESTAMP;          \
        result = op();                                   \
        auto __afterT = GET_CURRENT_TIMESTAMP;           \
        TIMESTAMP_ELAPSED_MS(__beforeT, __afterT, time); \
    }

#define GET_ARRAY_SIZE(v) sizeof(v) / sizeof(v[0])

#define AVG_CAST(cast, a, b) (((cast)((a) + (b))) / 2.0)
#define AVG_ARRAY(type, v, size) average<type>(v, size)

#define CONTAINS_SET(set, element) (set.find(element) != set.end())
#define CONTAINS_MAP(map, element) (map.find(element) != map.end())

#endif