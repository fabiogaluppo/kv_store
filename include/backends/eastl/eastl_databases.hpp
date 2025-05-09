//Source code C++ MasterClass (KV Store project) by Fabio Galuppo
//C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
//Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
//May 2025

#ifndef EASTL_DATABASES_HPP
#define EASTL_DATABASES_HPP

#include <string>
#include <vector>

#include <EASTL/array.h>
#include <EASTL/map.h>
#include <EASTL/set.h>
#include <EASTL/string.h>
#include <EASTL/unordered_map.h>
#include <EASTL/variant.h>

#include "database_defs.hpp"
#include "Generator.hpp"

struct SortedSet_t final
{
    eastl::map<std::string, double> Members;
    eastl::multimap<double, eastl::map<std::string, double>::iterator> Scores;
};

struct EASTL_Database_t final
{
    using string_type = std::string;
    using set_type = eastl::set<std::string>;
    using sortedset_type = SortedSet_t;

    using mapped_type = eastl::variant<string_type, set_type, sortedset_type>;
    eastl::unordered_map<eastl::string, mapped_type> Dict;

    template<typename T>
    T& Get(const std::string& key)
    {
        auto it = Dict.find(key.c_str());
        if (it != Dict.end())
            return eastl::get<T>(it->second);
        return eastl::get<T>(Dict.emplace(key.c_str(), T{}).first->second);
    }

    string_type& Strings(const std::string& key)
    {
        return Get<string_type>(key);
    }

    set_type& Sets(const std::string& key)
    {
        return Get<set_type>(key);
    }

    sortedset_type& SortedSets(const std::string& key)
    {
        return Get<sortedset_type>(key);
    }

    DbValueTypeEnum lookup_type_of(const std::string& key) const
    { 
        using enum DbValueTypeEnum;
        auto it = Dict.find(key.c_str());
        if (it != Dict.end())
        {
            const auto& value = it->second;
            if (eastl::holds_alternative<string_type>(value))
                return STRING;
            if (eastl::holds_alternative<set_type>(value))
                return SET;
            if (eastl::holds_alternative<sortedset_type>(value))
                return SORTEDSET;
        }
        return NONE;
    }

    bool exists(const std::string& key) const
    {
        return lookup_type_of(key) != DbValueTypeEnum::NONE;
    }

    bool del(const std::string& key)
    {
        if (auto it = Dict.find(key.c_str()); it != Dict.cend())
        {
            Dict.erase(it);
            return true;
        }
        return false;  
    }

    int size() const
    {
        return Dict.size();
    }

    Generator<std::string> keys() const
    {
        for (auto& kv : Dict)
            co_yield kv.first.c_str();
    }

    void clear()
    {
        Dict.clear();
    }
};

template<typename Predicate>
static std::vector<std::string> keys_if(const EASTL_Database_t& db, Predicate predicate)
{
    std::vector<std::string> keys;
    auto gen = db.keys();
    while (auto key_opt = gen.next())
        if (predicate(*key_opt))
            keys.emplace_back(*key_opt);
    return keys;
}

static eastl::array<EASTL_Database_t, 8> g_databases;

static void clear_all_databases()
{
    for(auto& db : g_databases) db.clear();
}

#endif /* EASTL_DATABASES_HPP */