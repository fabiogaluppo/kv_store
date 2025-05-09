//Source code C++ MasterClass (KV Store project) by Fabio Galuppo
//C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
//Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
//May 2025

#ifndef EASTL_STRATEGY_HPP
#define EASTL_STRATEGY_HPP

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <EASTL/algorithm.h>
#include <EASTL/iterator.h>
#include <EASTL/optional.h>
#include <EASTL/set.h>

#include "database_defs.hpp"
#include "resp.hpp"
#include "resp_command.hpp"
#include "eastl_context.hpp"

struct Strategy_t final
{
    static inline std::string set(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() != 3)
            return resp::error_wrong_number_of_arguments_for_command();

        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();
        const auto& key = cmd[1];
        switch (CurrentDb.lookup_type_of(key))
        {
            case NONE:
            case STRING:
            {
                auto& s = CurrentDb.Strings(key);
                const auto& val = cmd[2];
                s = val;
                return resp::ok();
            }
            default:
                return resp::error_wrong_type();
        }
    }

    static inline std::string get(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() != 2)
            return resp::error_wrong_number_of_arguments_for_command();

        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();
        const auto& key = cmd[1];
        switch (CurrentDb.lookup_type_of(key))
        {
            case STRING:            
                return resp::simple_string(CurrentDb.Strings(key));
            case NONE:
                return resp::nil();
            default:
                return resp::error_wrong_type();
        }
    }

    static inline std::string exists(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() < 2)
            return resp::error_wrong_number_of_arguments_for_command();

        const auto& CurrentDb = ctx.Client().CurrentDb();
        int exists{};
        for (std::size_t i = 1; i < cmd.size(); ++i)
        {
            const auto& key = cmd[i];
            exists += CurrentDb.exists(key) ? 1 : 0;
        }
        return resp::integer(exists);
    }

    static inline std::string keys(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() > 2)
            return resp::error_wrong_number_of_arguments_for_command();
        
        const auto& CurrentDb = ctx.Client().CurrentDb();
        if (cmd.size() == 1 || (cmd[1].size() == 1 && cmd[1][0] == '*'))
        {
            std::vector<std::string> keys = keys_if(CurrentDb, 
                [](const std::string&){ return true; });
            return resp::array(keys.begin(), keys.end());
        }
        else
        {
            const auto& pattern = cmd[1];
            if (!pattern.empty())
            {
                if (pattern[0] == '*') //ends with
                {
                    std::string_view pattern_sv{ pattern.begin() + 1, pattern.end() };
                    std::vector<std::string> keys = keys_if(CurrentDb, 
                        [&pattern_sv](const std::string& s){ return s.ends_with(pattern_sv); });
                    return resp::array(keys.begin(), keys.end());
                }
                if (pattern[pattern.size() - 1] == '*') //starts with
                {
                    std::string_view pattern_sv{ pattern.begin(), pattern.end() - 1 };
                    std::vector<std::string> keys = keys_if(CurrentDb, 
                        [&pattern_sv](const std::string& s){ return s.starts_with(pattern_sv); });
                    return resp::array(keys.begin(), keys.end());
                }                
            }            
            std::vector<std::string> keys = keys_if(CurrentDb, 
                [&pattern](const std::string& s){ return s == pattern; });
            return resp::array(keys.begin(), keys.end());
        }
    }

    static inline std::string del(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() < 2)
            return resp::error_wrong_number_of_arguments_for_command();

        auto& CurrentDb = ctx.Client().CurrentDb();
        int deletes{};
        for (std::size_t i = 1; i < cmd.size(); ++i)
        {
            const auto& key = cmd[i];
            deletes += CurrentDb.del(key) ? 1 : 0;
        }
        return resp::integer(deletes);
    }

    static inline std::string sadd(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() < 3)
            return resp::error_wrong_number_of_arguments_for_command();
        
        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();
        const auto& key = cmd[1];
        switch (CurrentDb.lookup_type_of(key))
        {
            case NONE:
            case SET:
            {
                auto& set = CurrentDb.Sets(key);                
                int inserted{};
                for (std::size_t i = 2; i < cmd.size(); ++i)
                {
                    const auto& member = cmd[i];
                    auto result = set.emplace(member.c_str());
                    inserted += result.second ? 1 : 0;
                }
                return resp::integer(inserted);
            }   
            default:
                return resp::error_wrong_type();
        }
    }

    static inline std::string srem(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() < 3)
            return resp::error_wrong_number_of_arguments_for_command();
        
        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();
        const auto& key = cmd[1];
        switch (CurrentDb.lookup_type_of(key))
        {
            case NONE:
            case SET:
            {
                auto& set = CurrentDb.Sets(key);
                int erased{};
                for (std::size_t i = 2; i < cmd.size(); ++i)
                {
                    const auto& member = cmd[i];
                    auto it = set.find(member.c_str());
                    if (it != set.end())
                    {
                        set.erase(it);
                        ++erased;
                    }
                }
                if (set.size() == 0) CurrentDb.del(key);
                return resp::integer(erased);
            }   
            default:
                return resp::error_wrong_type();
        }
    }

    static inline std::string scard(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() != 2)
            return resp::error_wrong_number_of_arguments_for_command();

        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();
        const auto& key = cmd[1];
        switch (CurrentDb.lookup_type_of(key))
        {
            case SET:
            {
                const auto& set = CurrentDb.Sets(key);
                return resp::integer(set.size());
            }
            case NONE:
                return resp::integer(0);
            default:
                return resp::error_wrong_type();
        }
    }

    static inline std::string smembers(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() != 2)
            return resp::error_wrong_number_of_arguments_for_command();

        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();
        const auto& key = cmd[1];
        switch (CurrentDb.lookup_type_of(key))
        {
            case SET:
            {
                const auto& set = CurrentDb.Sets(key);
                return resp::array(set.begin(), set.end());
            }
            case NONE:
                return resp::empty_array();
            default:
                return resp::error_wrong_type();
        }
    }

    static inline std::string sismember(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() != 3)
            return resp::error_wrong_number_of_arguments_for_command();

        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();
        const auto& key = cmd[1];
        switch (CurrentDb.lookup_type_of(key))
        {
            case SET:
            {
                const auto& set = CurrentDb.Sets(key);
                const auto& member = cmd[2];
                return resp::integer(set.find(member.c_str()) != set.end() ? 1 : 0);
            }
            case NONE:
                return resp::integer(0);
            default:
                return resp::error_wrong_type();
        }
    }

    static inline std::string sinter(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() < 2)
            return resp::error_wrong_number_of_arguments_for_command();
        
        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();        
        std::vector<eastl::set<std::string>*> sets;        
        for (std::size_t i = 1; i < cmd.size(); ++i)
        {
            const auto& key = cmd[i];
            switch (CurrentDb.lookup_type_of(key))
            {
            case NONE:
                return resp::empty_array();
            case SET:
                sets.push_back(&CurrentDb.Sets(key));
                break;
            default:
                return resp::error_wrong_type();
            }
        }
        switch (sets.size())
        {
            case 0:
                return resp::empty_array();
            case 1:
                return resp::array(sets[0]->begin(), sets[0]->end());
            default:
            {
                eastl::set<std::string> result = *sets[0];
                for (std::size_t i = 1; i < sets.size(); ++i)
                {
                    eastl::set<std::string> temp;
                    eastl::set_intersection(result.begin(), result.end(), sets[i]->begin(), sets[i]->end(),
                        eastl::inserter(temp, temp.end()));
                    result = temp;
                }
                return resp::array(result.begin(), result.end());
            }
        }
    }

    static inline std::string sunion(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() < 2)
            return resp::error_wrong_number_of_arguments_for_command();
        
        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();        
        std::vector<eastl::set<std::string>*> sets;
        for (std::size_t i = 1; i < cmd.size(); ++i)
        {
            const auto& key = cmd[i];
            switch (CurrentDb.lookup_type_of(key))
            {
            case NONE:
                break;
            case SET:
                sets.push_back(&CurrentDb.Sets(key));
                break;
            default:
                return resp::error_wrong_type();
            }
        }
        switch (sets.size())
        {
            case 0:
                return resp::empty_array();
            case 1:
                return resp::array(sets[0]->begin(), sets[0]->end());
            default:
            {
                eastl::set<std::string> result = *sets[0];
                for (std::size_t i = 1; i < sets.size(); ++i)
                {
                    eastl::set_union(result.begin(), result.end(), sets[i]->begin(), sets[i]->end(),
                        eastl::inserter(result, result.end()));
                }
                return resp::array(result.begin(), result.end());
            }
        }
    }

    static inline std::string zadd(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() < 4)
            return resp::error_wrong_number_of_arguments_for_command();
        
        if ((cmd.size() - 2) & 0x1)
            return resp::error_syntax_error();

        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();
        const auto& key = cmd[1];
        switch (CurrentDb.lookup_type_of(key))
        {
            case NONE:
            case SORTEDSET:
            {
                auto& sorted_set = CurrentDb.SortedSets(key);                
                int inserted{};
                for (std::size_t i = 2; i < cmd.size(); i += 2)
                {
                    std::optional<double> score_opt = string_to_double(cmd[i]);        
                    if (score_opt)
                    {
                        const auto& member = cmd[i+1];
                        auto [iter, emplaced] = sorted_set.Members.emplace(member.c_str(), *score_opt);
                        double& score = iter->second;
                        if (emplaced)
                        {  
                            sorted_set.Scores.emplace(score, iter);
                            ++inserted;
                        }
                        else
                        {
                            auto range = sorted_set.Scores.equal_range(score);
                            sorted_set.Scores.erase(eastl::find_if(range.first, range.second, [&iter](auto x) {
                                return x.second->first == iter->first;
                            }));
                            score = *score_opt;
                            sorted_set.Scores.emplace(score, iter);
                        }
                    }
                }
                return resp::integer(inserted);
            }
            default:
                return resp::error_wrong_type();
        }
    }

    static inline std::string zscore(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() != 3)
            return resp::error_wrong_number_of_arguments_for_command();
        
        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();
        const auto& key = cmd[1];
        switch (CurrentDb.lookup_type_of(key))
        {
            case SORTEDSET:
            {
                const auto& sorted_set = CurrentDb.SortedSets(key);
                const auto& member = cmd[2];
                auto it = sorted_set.Members.find(member.c_str());
                if (it != sorted_set.Members.end())
                    return resp::simple_string(double_to_string(it->second));
                return resp::nil();
            }
            case NONE:
                return resp::nil();
            default:
                return resp::error_wrong_type();
        }
    }

    static inline std::string zcard(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() != 2)
            return resp::error_wrong_number_of_arguments_for_command();

        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();
        const auto& key = cmd[1];
        switch (CurrentDb.lookup_type_of(key))
        {
            case SORTEDSET:
                return resp::integer(CurrentDb.SortedSets(key).Members.size());
            case NONE:
                return resp::integer(0);
            default:
                return resp::error_wrong_type();
        }
    }

    static inline std::string zrange(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() < 4)
            return resp::error_wrong_number_of_arguments_for_command();

        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();
        const auto& key = cmd[1];
        switch (CurrentDb.lookup_type_of(key))
        {
            case SORTEDSET:
            {
                bool byscore = false, withscores = false;
                if (cmd.size() > 5)
                {
                    auto arg = to_upper(cmd[5]);
                    if (arg == "WITHSCORES") withscores = true;
                    else return resp::error_syntax_error();
                }
                if (cmd.size() > 4)
                {
                    auto arg = to_upper(cmd[4]);
                    if (arg == "BYSCORE") byscore = true;
                    else if (!withscores && arg == "WITHSCORES") withscores = true;
                    else return resp::error_syntax_error();
                }
                const auto& sorted_set = CurrentDb.SortedSets(key);   
                if (byscore)
                {
                    const auto& start = cmd[2];
                    const auto& stop = cmd[3];
                    std::optional<double> start_score_opt = string_to_double(start);
                    std::optional<double> stop_score_opt = string_to_double(stop);
                    if (start_score_opt && stop_score_opt)
                    {
                        if (*stop_score_opt < *start_score_opt)
                            return resp::empty_array();

                        //BYSCORE
                        auto& scores = sorted_set.Scores;
                        auto start_it = scores.lower_bound(*start_score_opt); //[
                        auto stop_it = scores.upper_bound(*stop_score_opt); //]                        
                        std::vector<std::string> result;
                        for (auto it = start_it; it != stop_it; ++it)
                        {
                            result.emplace_back(it->second->first);
                            if (withscores) result.emplace_back(double_to_string(it->second->second).c_str());
                        }
                        return resp::array(result.begin(), result.end());
                    }
                    return resp::error_min_or_max_is_not_a_float();
                }
                else
                {
                    const auto& start = cmd[2];
                    const auto& stop = cmd[3];
                    std::optional<int> start_index_opt = string_to_int(start);
                    std::optional<int> stop_index_opt = string_to_int(stop);
                    if (start_index_opt && stop_index_opt)
                    {
                        if (*stop_index_opt < *start_index_opt)
                            return resp::empty_array();
                        if (*start_index_opt < 0 || *stop_index_opt < 0) //negative index not supported 
                            return resp::error_syntax_error();
                        
                        //BYINDEX
                        auto& members = sorted_set.Members;
                        auto start_it = members.begin();
                        if (*start_index_opt < members.size())
                            eastl::advance(start_it, *start_index_opt);
                        else
                            start_it = members.end();
                        auto stop_it = members.begin();
                        if ((*stop_index_opt + 1) < members.size())
                            eastl::advance(stop_it, *stop_index_opt + 1);
                        else
                            stop_it = members.end();
                        std::vector<std::string> result;
                        for (auto it = start_it; it != stop_it; ++it)
                        {
                            result.emplace_back(it->first);
                            if (withscores) result.emplace_back(double_to_string(it->second).c_str());
                        }
                        return resp::array(result.begin(), result.end());
                    }
                    return resp::error_value_is_not_an_integer_or_out_of_range();
                }
            }
            case NONE:
                return resp::empty_array();
            default:
                return resp::error_wrong_type();
        }
    }

    static inline std::string zremrangebyscore(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() != 4)
            return resp::error_wrong_number_of_arguments_for_command();

        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();
        const auto& key = cmd[1];
        switch (CurrentDb.lookup_type_of(key))
        {
            case SORTEDSET:
            {
                const auto& min = cmd[2];
                const auto& max = cmd[3];
                std::optional<double> min_score_opt = string_to_double(min);
                std::optional<double> max_score_opt = string_to_double(max);
                if (min_score_opt && max_score_opt)
                {
                    auto& sorted_set = CurrentDb.SortedSets(key);
                    if (*max_score_opt < *min_score_opt)
                        return resp::empty_array();
                    auto& members = sorted_set.Members;
                    auto& scores = sorted_set.Scores;
                    auto start_it = scores.lower_bound(*min_score_opt); //[
                    auto stop_it = scores.upper_bound(*max_score_opt); //]
                    int erased{};                
                    for (auto it = start_it; it != stop_it; ++it, ++erased)
                        members.erase(it->second->first);
                    if (erased)
                    {
                        scores.erase(start_it, stop_it);
                        if (members.size() == 0) CurrentDb.del(key);
                    }
                    return resp::integer(erased);
                }
                return resp::error_min_or_max_is_not_a_float();    
            }
            case NONE:
                return resp::integer(0);
            default:
                return resp::error_wrong_type();
        }
    }

    static inline std::string zrem(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() < 3)
            return resp::error_wrong_number_of_arguments_for_command();

        using enum DbValueTypeEnum;
        auto& CurrentDb = ctx.Client().CurrentDb();
        const auto& key = cmd[1];
        switch (CurrentDb.lookup_type_of(key))
        {
            case SORTEDSET:
            {
                auto& sorted_set = CurrentDb.SortedSets(key);
                auto& members = sorted_set.Members;
                auto& scores = sorted_set.Scores;
                int erased{};
                for (std::size_t i = 2; i < cmd.size(); ++i)
                {
                    const auto& member = cmd[i];
                    auto it = members.find(member.c_str());
                    if (it != members.end())
                    {
                        auto range = scores.equal_range(it->second);
                        scores.erase(eastl::find_if(range.first, range.second, [&it](auto x) {
                            return x.second->first == it->first;
                        }));
                        members.erase(it);
                        ++erased;
                    }
                }
                if (members.size() == 0) CurrentDb.del(key);
                return resp::integer(erased);
            }
            case NONE:
                return resp::integer(0);
            default:
                return resp::error_wrong_type();
        }
    }

    static inline std::string type(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() != 2)
            return resp::error_wrong_number_of_arguments_for_command();

        auto& CurrentDb = ctx.Client().CurrentDb();
        const auto& key = cmd[1];
        return resp::simple_string(to_string(CurrentDb.lookup_type_of(key)));
    }

    static inline std::string client(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() < 2)
            return resp::error_wrong_number_of_arguments_for_command();

        const auto& subcmd = cmd[1];
        if (subcmd == "SETNAME") //CLIENT SETNAME connection-name
        {
            if (cmd.size() < 3)
                return resp::error_wrong_number_of_arguments_for_command();
            
            ctx.Client().ConnectionName = cmd[2];
            return resp::ok();
        }
        if (subcmd == "GETNAME") //CLIENT SETNAME connection-name
        {
            if (cmd.size() != 2)
                return resp::error_wrong_number_of_arguments_for_command();
            if (ctx.Client().ConnectionName.empty())
                return resp::nil();            
            return resp::simple_string(ctx.Client().ConnectionName);
        }
        if (subcmd == "SETINFO") //CLIENT SETINFO <LIB-NAME libname | LIB-VER libver>
        {
            if (cmd.size() < 4)
                return resp::error_wrong_number_of_arguments_for_command();
            
            const auto& subcmdarg = cmd[2];
            if (subcmdarg == "LIB-NAME")
            {
                ctx.Client().LibName = cmd[3];
                return resp::ok();
            }
            else if (subcmdarg == "LIB-VER")
            {
                ctx.Client().LibVersion = cmd[3];
                return resp::ok();
            }
            
            return resp::error_unknown_subcommand(cmd[2]);
        }
        if (subcmd == "INFO") //CLIENT INFO
        {
            if (cmd.size() != 2)
                return resp::error_wrong_number_of_arguments_for_command();
            
            return resp::simple_string(ctx.Client().to_string());
        }        
        return resp::error_unknown_subcommand(subcmd);
    }

    static inline std::string select(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() != 2)
            return resp::error_wrong_number_of_arguments_for_command();
        
        const auto& db = cmd[1];
        std::optional<int> index_opt = string_to_int(db);
        if (index_opt && ctx.Client().CurrentDb(*index_opt))
            return resp::ok();
        return resp::error_value_is_not_an_integer_or_out_of_range();
    }

    static inline std::string flushdb(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() != 1)
            return resp::error_wrong_number_of_arguments_for_command();

        auto& CurrentDb = ctx.Client().CurrentDb();
        CurrentDb.clear();
        return resp::ok();
    }

    static inline std::string dbsize(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() != 1)
            return resp::error_wrong_number_of_arguments_for_command();

        return resp::integer(ctx.Client().CurrentDb().size());
    }

    static inline std::string ping(Context_t& ctx, const resp::command& cmd)
    {
        if (cmd.size() != 1)
            return resp::error_wrong_number_of_arguments_for_command();

        return resp::pong();
    }
};

#endif /* EASTL_STRATEGY_HPP */