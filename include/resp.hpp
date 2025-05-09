//Source code C++ MasterClass (KV Store project) by Fabio Galuppo
//C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
//Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
//May 2025

#ifndef RESP_HPP
#define RESP_HPP

#include <iterator>
#include <string>
#include <string_view>
#include <type_traits>

#include "format.hpp"

template <typename T>
constexpr bool is_random_access_iterator_v =
    std::is_base_of_v<std::random_access_iterator_tag,
                      typename std::iterator_traits<T>::iterator_category>;

namespace resp
{
    static inline std::string simple_string(const std::string_view& sv)
    {
        return format::resp_simple_string(sv);
    }

    template <class InputIt>
    static inline std::string array(InputIt first, InputIt last)
    {
        if constexpr (std::is_base_of_v<std::random_access_iterator_tag,
                      typename std::iterator_traits<InputIt>::iterator_category>)
        {
            std::string result = format::resp_array_size(std::distance(first, last));
            for (auto it = first; it != last; ++it)
                result.append(simple_string(*it));
            return result;
        }
        else
        {
            std::string result;
            std::size_t size{};
            for (auto it = first; it != last; ++it, ++size)
                result.append(simple_string(*it));
            result.insert(0, format::resp_array_size(size));
            return result;
        }
    }

    static inline std::string error_unknown_command(const std::string_view& sv)
    {
        return format::resp_error_unknown_command(sv);
    }

    static inline std::string error_unknown_subcommand(const std::string_view& sv)
    {
        return format::error_unknown_subcommand(sv);
    }

    constexpr const char* error_wrong_number_of_arguments_for_command()
    {
        return "-ERR wrong number of arguments for command\r\n";
    }

    constexpr const char* error_value_is_not_an_integer_or_out_of_range()
    {
        return "-ERR value is not an integer or out of range\r\n";
    }

    constexpr const char* error_min_or_max_is_not_a_float()
    {
        return "-ERR min or max is not a float\r\n";
    }

    constexpr const char* error_wrong_type()
    {
        return "-ERR WRONGTYPE Operation against a key holding the wrong kind of value\r\n";
    }

    constexpr const char* error_syntax_error()
    {
        return "-ERR syntax error\r\n";
    }

    static inline std::string integer(int num)
    {
        return format::resp_integer(num);
    }
    
    constexpr const char* ok() { return "+OK\r\n"; }
    constexpr const char* nil() { return "$-1\r\n"; }
    constexpr const char* pong() { return "+PONG\r\n"; }
    constexpr const char* empty_array() { return "*0\r\n"; }
}

#endif /* RESP_HPP */