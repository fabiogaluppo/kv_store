#ifndef FORMAT_HPP
#define FORMAT_HPP

#include <cstddef>
#include <string>
#include <string_view>

#if __has_include(<format>)
#include <format>
namespace format
{
    static inline std::string double_to_string(double val)
    {
        return std::format("{:.15g}", val);
    }

    static inline std::string resp_simple_string(const std::string_view& sv)
    {
        return std::format("${}\r\n{}\r\n", sv.size(), sv);
    }

    static inline std::string resp_array_size(std::size_t size)
    {
        return std::format("*{}\r\n", size);
    }

    static inline std::string resp_error_unknown_command(const std::string_view& sv)
    {
        return std::format("-ERR unknown command '{}'\r\n", sv);
    }

    static inline std::string error_unknown_subcommand(const std::string_view& sv)
    {
        return std::format("-ERR unknown subcommand '{}'\r\n", sv);
    }

    static inline std::string resp_integer(int num)
    {
        if (num) return std::format(":{:+}\r\n", num);
        return std::format(":0\r\n", num);
    }

    static inline std::string zmq_version_string(int major, int minor, int patch, const std::string& backend)
    {
        return std::format("ZMQ version {}.{}.{} (Backend: {})\n\n", major, minor, patch, backend);
    }

    static inline std::string zmq_tcp_address(int port)
    {
        return std::format("tcp://*:{}", port);
    }
}
#else
#include <sstream>
#include <utility>
namespace format
{
    static inline std::string double_to_string(double val)
    {
        std::ostringstream oss;
        oss << std::setprecision(15) << std::noshowpoint << val;
        return std::move(oss.str());
    }

    static inline std::string resp_simple_string(const std::string_view& sv)
    {
        std::ostringstream oss;
        oss << '$' << sv.size() << "\r\n" << sv << "\r\n";
        return std::move(oss.str());
    }

    static inline std::string resp_array_size(std::size_t size)
    {
        std::ostringstream oss;
        oss << '*' << size << "\r\n";
        return std::move(oss.str());
    }

    static inline std::string resp_error_unknown_command(const std::string_view& sv)
    {
        std::ostringstream oss;
        oss << "-ERR unknown command '" << sv << "'\r\n";
        return std::move(oss.str());
    }

    static inline std::string error_unknown_subcommand(const std::string_view& sv)
    {
        std::ostringstream oss;
        oss << "-ERR unknown subcommand '" << sv << "'\r\n";
        return std::move(oss.str());
    }

    static inline std::string resp_integer(int num)
    {
        std::ostringstream oss;
        if (num != 0)
            oss << ':' << (num > 0 ? "+" : "") << num << "\r\n";
        else
            oss << ":0\r\n";
        return std::move(oss.str());
    }

    static inline std::string zmq_version_string(int major, int minor, int patch, const std::string& backend)
    {
        std::ostringstream oss;
        oss << "ZMQ version " << major << '.' << minor << '.' << patch
            << " (Backend: " << backend << ")\n\n";
        return std::move(oss.str());
    }

    static inline std::string zmq_tcp_address(int port)
    {
        std::ostringstream oss;
        oss << "tcp://*:" << port;
        return std::move(oss.str());
    }
}
#endif 

#endif /* FORMAT_HPP */