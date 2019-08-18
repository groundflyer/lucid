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
        // static const constexpr char DEFAULT[] = "\033[39m";
        // static const constexpr char RED[] = "\033[31m";
        // static const constexpr char GREEN[] = "\033[32m";
        // static const constexpr char YELLOW[] = "\033[33m";
        // static const constexpr char BOLD_RED[] = "\033[1m\033[31m";
        // static const constexpr char RESET[] = "\033[0m";

        void timestamp(const fmt::text_style& ts) const noexcept
        {
            const std::time_t date = std::time(nullptr);
            fmt::print(ts, "[{:%T}] ", *std::localtime(&date));
        }

        template <std::size_t N, typename ... Args>
        void
        std_out(const fmt::text_style& ts,
                const char (&fmt_str)[N],
                Args&& ... args) const noexcept
        {
            timestamp(ts);
            fmt::print(fmt_str, std::forward<Args>(args)...);
            fmt::print("\n");
        }

        template <std::size_t N, typename ... Args>
        void
        std_error(const fmt::text_style& ts,
                  const char (&fmt_str)[N],
                  Args&& ... args) const noexcept
        {
            timestamp(ts);
            fmt::print(stderr, fmt_str, std::forward<Args>(args)...);
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
                std_out(fmt::fg(fmt::terminal_color::white),
                        std::forward<Args>(args)...);
        }

        template <typename ... Args>
        void
        info(Args&& ... args) const noexcept
        {
            if (lvl <= Level::INFO)
                std_out(fmt::fg(fmt::terminal_color::green),
                        std::forward<Args>(args)...);
        }

        template <typename ... Args>
        void
        warning(Args&& ... args) const noexcept
        {
            if (lvl <= Level::WARNING)
                std_error(fmt::fg(fmt::terminal_color::yellow),
                          std::forward<Args>(args)...);
        }

        template <typename ... Args>
        void
        error(Args&& ... args) const noexcept
        {
            if (lvl <= Level::ERROR)
                std_error(fmt::fg(fmt::terminal_color::red),
                          std::forward<Args>(args)...);
        }

        template <typename ... Args>
        void
        critical(Args&& ... args) const noexcept
        {
            if (lvl <= Level::CRITICAL)
                std_error(fmt::emphasis::bold | fmt::fg(fmt::terminal_color::red),
                          std::forward<Args>(args)...);
        }

    private:
        Level lvl = Level::INFO;
    };
}
