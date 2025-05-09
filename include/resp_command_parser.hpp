//Source code C++ MasterClass (KV Store project) by Fabio Galuppo
//C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
//Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
//May 2025

#ifndef RESP_COMMAND_PARSER_HPP
#define RESP_COMMAND_PARSER_HPP

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

//REDIS command
/*

redis_array   = "*" number CRLF redis_string+ ;
redis_string  = "$" number CRLF chars* CRLF ;
CRLF    = "\r\n" ;

*/

namespace resp
{
    struct command_parser final
    {
        const char* position;    
        std::optional<std::vector<std::string_view>> parse()
        {
            if (position[0] == '*')
            {
                const char* old_position = position;
                auto cmd_opt = read_array();
                position = old_position;
                return cmd_opt;
            }
            return std::nullopt;
        }

    private:
        std::optional<std::string_view> read_string()
        {
            const char* pos = position;        
            char* pos_beg = const_cast<char*>(pos + 1);
            char* pos_end = std::strchr(pos_beg,'\r');
            std::size_t size = std::strtoll(pos_beg, &pos_end, 10);
            if (pos_beg == pos_end)
                return std::nullopt;
            position = pos_end + 2; //\r\n
            std::string_view sv{ position, position + size };
            position += size;
            position += 2; //\r\n
            return sv;
        }

        std::optional<std::vector<std::string_view>> read_array()
        {
            using size_type = std::vector<std::string_view>::size_type;
            const char* pos = position;
            char* pos_beg = const_cast<char*>(pos + 1);
            char* pos_end = std::strchr(pos_beg,'\r');
            size_type size = std::strtoll(pos_beg, &pos_end, 10);
            if (pos_beg == pos_end)
                return std::nullopt;
            pos_end += 2; //\r\n
            position = pos_end;
            if (size > 0)
            {
                std::vector<std::string_view> temp(size); 
                for (size_type i = 0; i < size; ++i)
                {
                    auto opt_sv = read_string();
                    if (opt_sv)
                        temp[i] = *opt_sv;
                    else
                        return std::nullopt;
                }
                return temp;
            }
            return std::nullopt;
        }
    };
}

#endif /* RESP_COMMAND_PARSER_HPP */