//Source code C++ MasterClass (KV Store project) by Fabio Galuppo
//C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
//Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
//May 2025

#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include <cctype>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>

#include "format.hpp"

static inline std::string double_to_string(double val)
{
    return format::double_to_string(val);
}

static inline std::optional<double> string_to_double(const std::string& s)
{
    std::optional<double> result;
    try { result = std::stod(s); }
    catch (std::logic_error& err) {}
    return result;
}

static inline std::optional<int> string_to_int(const std::string& s)
{
    std::optional<int> result;
    try { result = std::stoi(s); }
    catch (std::logic_error& err) {}
    return result;
}

static inline std::string int_to_string(int val)
{
    return std::to_string(val);
}

static inline std::string to_upper(std::string_view sv)
{
    std::string temp;
    std::transform(sv.begin(), sv.end(), std::back_inserter(temp), 
        [](std::string::value_type c) -> std::string::value_type { return std::toupper(c); });
    return temp;
}

#endif /* UTILS_HPP */