//Source code C++ MasterClass (KV Store project) by Fabio Galuppo
//C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
//Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
//May 2025

#ifndef EXECUTE_COMMAND_HPP
#define EXECUTE_COMMAND_HPP

#include <string>

#include "resp.hpp"
#include "resp_command.hpp"

template<typename Context, typename CommandStrategy>
static inline std::string execute_command(Context& ctx, const resp::command& cmd, bool& unk_cmd)
{
    const auto& cmd_name = cmd.name();
    
    if (cmd_name == "SET") //SET key value
        return CommandStrategy::set(ctx, cmd);

    if (cmd_name == "GET") //GET key
        return CommandStrategy::get(ctx, cmd);

    if (cmd_name == "EXISTS") //EXISTS key [key ...]
        return CommandStrategy::exists(ctx, cmd);

    if (cmd_name == "KEYS") //KEYS [pattern]
        return CommandStrategy::keys(ctx, cmd);
    
    if (cmd_name == "DEL") //DEL key [key ...]
        return CommandStrategy::del(ctx, cmd);

    if (cmd_name == "SADD") //SADD key member [member ...]
        return CommandStrategy::sadd(ctx, cmd);

    if (cmd_name == "SREM") //SREM key member [member ...]
        return CommandStrategy::srem(ctx, cmd);
    
    if (cmd_name == "SCARD") //SCARD key
        return CommandStrategy::scard(ctx, cmd);
    
    if (cmd_name == "SMEMBERS") //SMEMBERS key
        return CommandStrategy::smembers(ctx, cmd);
        
    if (cmd_name == "SISMEMBER") //SISMEMBER key member
        return CommandStrategy::sismember(ctx, cmd);

    if (cmd_name == "SINTER") //SINTER key [key ...]
        return CommandStrategy::sinter(ctx, cmd);
    
    if (cmd_name == "SUNION") //SUNION key [key ...]
        return CommandStrategy::sunion(ctx, cmd);

    if (cmd_name == "ZADD") //ZADD key score member [score member ...]
        return CommandStrategy::zadd(ctx, cmd);

    if (cmd_name == "ZSCORE") //ZSCORE key member
        return CommandStrategy::zscore(ctx, cmd);

    if (cmd_name == "ZCARD") //ZCARD key
        return CommandStrategy::zcard(ctx, cmd);

    if (cmd_name == "ZRANGE") //ZRANGE key start stop [BYSCORE] [WITHSCORES]
        return CommandStrategy::zrange(ctx, cmd);
    
    if (cmd_name == "ZREMRANGEBYSCORE") //ZREMRANGEBYSCORE key min max
        return CommandStrategy::zremrangebyscore(ctx, cmd);

    if (cmd_name == "ZREM") //ZREM key member [member ...]
        return CommandStrategy::zrem(ctx, cmd);

    if (cmd_name == "TYPE") //TYPE key
        return CommandStrategy::type(ctx, cmd);
    
    if (cmd_name == "CLIENT") //CLIENT
        return CommandStrategy::client(ctx, cmd);

    if (cmd_name == "SELECT") //SELECT index
        return CommandStrategy::select(ctx, cmd);

    if (cmd_name == "FLUSHDB") //FLUSHDB
        return CommandStrategy::flushdb(ctx, cmd);

    if (cmd_name == "DBSIZE") //DBSIZE
        return CommandStrategy::dbsize(ctx, cmd);

    if (cmd_name == "PING") //PING
        return CommandStrategy::ping(ctx, cmd);

    //ignore command
    unk_cmd = true;
    return resp::error_unknown_command(cmd_name);
}

#endif /* EXECUTE_COMMAND_HPP */