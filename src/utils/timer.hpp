// -*- C++ -*-
// timer.hpp
// A library to calculate elapsed times and easily write chrono::duration
// into an iostream
//
//
// MIT License
//
// Copyright (c) 2017 Roman Saldygashev
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#pragma once

#include <chrono>
#include <ratio>
#include <string>
#include <utility>


namespace lucid
{
    template <typename Clock = std::chrono::steady_clock>
    class ElapsedTimer
    {
        using ClockDuration = std::chrono::duration<typename Clock::rep, typename Clock::period>;
        using TimePoint = typename Clock::time_point;

        TimePoint startpoint = Clock::now();
    public:
        ElapsedTimer() {}

        ElapsedTimer(const ElapsedTimer & rhs) : startpoint(rhs.startpoint) {}

        explicit
        ElapsedTimer(const TimePoint & rhs) : startpoint(rhs) {}

        bool
        operator==(const ElapsedTimer & rhs) const noexcept
        { return startpoint == rhs.startpoint; }
        bool
        operator!=(const ElapsedTimer & rhs) const noexcept
        { return startpoint != rhs.startpoint; }

        // is younger
        bool
        operator<(const ElapsedTimer & rhs) const noexcept
        { return startpoint > rhs.startpoint; }
        bool
        operator<=(const ElapsedTimer & rhs) const noexcept
        { return startpoint >= rhs.startpoint; }

        // is older
        bool
        operator>(const ElapsedTimer & rhs) const noexcept
        { return startpoint < rhs.startpoint; }
        bool
        operator>=(const ElapsedTimer & rhs) const noexcept
        { return startpoint <= rhs.startpoint; }

        ClockDuration
        operator-(const ElapsedTimer & rhs) const noexcept
        { return startpoint - rhs.startpoint; }

        ClockDuration
        elapsed() const noexcept
        { return Clock::now() - startpoint; }

        ClockDuration
        restart() noexcept
        {
            const ClockDuration ret = elapsed();
            startpoint = Clock::now();
            return ret;
        }

        template <typename Rep, typename Period>
        bool
        has_expired(const std::chrono::duration<Rep, Period> & timeout) const noexcept
        { return elapsed() > timeout; }
    };

    template <typename F, typename Clock = std::chrono::steady_clock, typename ... Args>
    std::chrono::duration<typename Clock::rep, typename Clock::period>
    measure(F && callable, Args && ...args)
    {
        ElapsedTimer<Clock> timer;

        callable(std::forward<Args>(args)...);

        return timer.elapsed();
    }
}


// namespace std
// {
//     namespace chrono
//     {
//         typedef duration<int64_t, ratio<86400>> days;
//     }

//     template <typename Rep, intmax_t Num, intmax_t Denom>
//     string
//     to_string(chrono::duration<Rep, ratio<Num, Denom>> duration,
//               const tinytimer::FF flag = tinytimer::DAY,
//               const streamsize precision = 3)
//     {
//         stringstream ss;
//     	ss << setfill('0');

//         switch (flag) {
//         case tinytimer::DAY:
//             {
//                 auto days = chrono::duration_cast<chrono::days>(duration);
//                 ss << days.count() << ':';
//                 duration -= days;
//             }
//         case tinytimer::HOUR:
//             {
//                 auto hours = chrono::duration_cast<chrono::hours>(duration);
//                 ss << setw(2) << hours.count() << ':';
//                 duration -= hours;
//             }
//         case tinytimer::MINUTE:
//             {
//                 auto minutes = chrono::duration_cast<chrono::minutes>(duration);
//                 ss << setw(2) << minutes.count() << ':';
//                 duration -= minutes;
//             }
//         case tinytimer::SECOND:
//             break;
//         }

//         auto dseconds = chrono::duration_cast<chrono::seconds>(duration);
//         duration -= dseconds;
//         double fseconds = static_cast<double>(dseconds.count()) + static_cast<double>(duration.count()) / Denom;
//         ss << setprecision(precision) << setw(3 + precision) << fixed << fseconds;

//         return ss.str();
//     }
// }
