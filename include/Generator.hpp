//Source code C++ MasterClass (Algorithms with C++ MasterClass) by Fabio Galuppo
//C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
//Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
//This file is licensed to you under the MIT license
//March 2025

//For C++ 20, in C++ 23 use std::generator instead: https://en.cppreference.com/w/cpp/coroutine/generator

#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <cstdlib>
#include <coroutine>
#include <optional>

template<typename T>
struct Generator final
{
    struct promise_type final
    {
        std::optional<T> current_;
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::exit(1); }
        template<std::convertible_to<T> From>
        std::suspend_always yield_value(From&& from)
        {
            current_ = std::forward<From>(from);
            return {};
        }
        void return_void() { current_ = std::nullopt; }
        Generator get_return_object() { return Generator(handle_type::from_promise(*this)); }
    };
    using handle_type = std::coroutine_handle<promise_type>;
    handle_type handler_;
    Generator(handle_type handler) noexcept : handler_(handler) {}
    ~Generator() noexcept { handler_.destroy(); }
    std::optional<T> next() noexcept
    {
        if (handler_.done())
            return std::nullopt;
        handler_.resume();
        return handler_.promise().current_;
    }
};

#endif /* GENERATOR_HPP */