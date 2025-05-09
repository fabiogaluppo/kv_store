//Source code C++ MasterClass (KV Store project) by Fabio Galuppo
//C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
//Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
//May 2025

#ifndef RESP_COMMAND_HPP
#define RESP_COMMAND_HPP

#include <string>
#include <string_view>
#include <vector>

#include "utils.hpp"

namespace resp
{
    //don't reuse command
    class command final
    {
        std::vector<std::string_view> args;
        std::string cmd_name;
    public:
        template<typename... Args>
        explicit constexpr command(Args&&... args) noexcept 
            : command(std::vector<std::string_view>{std::move(args)...}) {}
        explicit command(std::vector<std::string_view>&& args) noexcept 
            : args{args}, cmd_name{to_upper(args[0])}{}
        command() = delete;
        ~command() = default;
        command(const command&) = delete;
        command& operator=(const command&) = delete;
        command(command&&) = delete;
        command& operator=(command&&) = delete;
    
        const std::string& name() const { return cmd_name; }

        const std::size_t size() const { return args.size(); }

        const std::string operator[](std::size_t index) const
        {
            if (index == 0)
                return cmd_name;
            return std::string{args[index]};
        }
    };
}

#endif /* RESP_COMMAND_HPP */