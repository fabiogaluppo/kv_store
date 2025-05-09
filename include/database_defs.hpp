//Source code C++ MasterClass (KV Store project) by Fabio Galuppo
//C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
//Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
//May 2025

#ifndef DATABASE_DEFS_HPP
#define DATABASE_DEFS_HPP

#include <string>

enum class DbValueTypeEnum
{
    NONE, STRING, SET, SORTEDSET
};

static inline std::string to_string(DbValueTypeEnum value)
{
    using enum DbValueTypeEnum;
    switch (value)
    {
        case STRING: return "string";
        case SET: return "set";
        case SORTEDSET: return "zset";
        default: return "none";
    }
}

#endif /* DATABASE_DEFS_HPP */