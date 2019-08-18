// -*- C++ -*-
// logging.hpp
//

#pragma once

#include <fmt/chrono.h>
#include <fmt/color.h>


namespace lucid
{
    class Logger
    {
        static void
        timestamp(const fmt::text_style& ts,
                       const char i) noexcept
        {
            const std::time_t date = std::time(nullptr);
            fmt::print(ts, "[{:%T}][{}] ", *std::localtime(&date), i);
        }

        template <typename Output, typename ... Args>
        static void
        message(Output&& out,
                const fmt::text_style& ts,
                const char i,
                Args&& ... args) noexcept
        {
            timestamp(ts, i);
            fmt::print(std::forward<Output>(out), std::forward<Args>(args)...);
            fmt::print("\n");
        }

    public:
        enum Level
        {
         DEBUG = 10,
         INFO = 20,
         WARNING = 30,
         ERROR = 40,
         CRITICAL = 50
        };

        explicit
        Logger(const Level lvl_) : lvl(lvl_) {}

        template <typename ... Args>
        void
        debug(Args&& ... args) const noexcept
        {
            if (lvl <= Level::DEBUG)
                message(stdout,
                        fmt::fg(fmt::terminal_color::white),
                        'D',
                        std::forward<Args>(args)...);
        }

        template <typename ... Args>
        void
        info(Args&& ... args) const noexcept
        {
            if (lvl <= Level::INFO)
                message(stdout,
                        fmt::fg(fmt::terminal_color::green),
                        'I',
                        std::forward<Args>(args)...);
        }

        template <typename ... Args>
        void
        warning(Args&& ... args) const noexcept
        {
            if (lvl <= Level::WARNING)
                message(stderr,
                        fmt::fg(fmt::terminal_color::yellow),
                        'I',
                        std::forward<Args>(args)...);
        }

        template <typename ... Args>
        void
        error(Args&& ... args) const noexcept
        {
            if (lvl <= Level::ERROR)
                message(stderr,
                        fmt::fg(fmt::terminal_color::red),
                        std::forward<Args>(args)...);
        }

        template <typename ... Args>
        void
        critical(Args&& ... args) const noexcept
        {
            if (lvl <= Level::CRITICAL)
                message(stderr,
                        fmt::emphasis::bold | fmt::fg(fmt::terminal_color::red),
                        'C',
                        std::forward<Args>(args)...);
        }

    private:
        Level lvl = Level::INFO;
    };
}
