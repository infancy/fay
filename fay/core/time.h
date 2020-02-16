#pragma once

#include <chrono>

#include "fay/core/fay.h"

namespace fay
{

class date
{
public:
    constexpr date(size literal) : literal_{ literal } {}

private:
    size literal_;
    // year, month, day
};

class time
{
public:
    constexpr time(size literal) : literal_{ literal } {}

private:
    size literal_;
    // hour, minutes, seconds
};

class datetime
{
public:
    constexpr datetime(size d, size t) : date_{ d }, time_{ t } {}
    constexpr datetime(date d, time t) : date_{ d }, time_{ t } {}

private:
    date date_;
    time time_;
};



constexpr date operator ""date(size sz) { return date(sz); }
constexpr time operator ""time(size sz) { return time(sz); }

// datetime dt{ 19700101, 000000 };
// datetime dt{ 19700101date, 000000time };

// unix stamp
inline auto now() { return std::chrono::high_resolution_clock::now().time_since_epoch().count(); }

} // namespace fay