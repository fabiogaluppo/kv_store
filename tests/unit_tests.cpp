//Source code C++ MasterClass (KV Store project) by Fabio Galuppo
//C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
//Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
//May 2025

#include <algorithm>
#include <string_view>
#include <vector>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "backend.hpp"
#include "execute_command.hpp"
#include "resp_command_parser.hpp"

#include "../src/eastl_stub_allocator.inl"

using namespace std::literals;

std::string execute_command(Context_t&& ctx, resp::command&& cmd)
{
    bool unk_cmd{};
    return execute_command<Context_t, Strategy_t>(ctx, cmd, unk_cmd);
}

struct unit_test_fixture
{
    std::string client_id = "TEST-CLIENT";

    unit_test_fixture()
    {
        Context_t::create_or_remove_client(client_id);
    }

    ~unit_test_fixture()
    {
        Context_t::create_or_remove_client(client_id);
        clear_all_databases();
    }
};

TEST_CASE_FIXTURE(unit_test_fixture, "SET") 
{
    auto cmd_reply = execute_command
    (
        Context_t{client_id}, 
        resp::command{"SET"sv, "KEY1"sv, "VAL1"sv}
    );
    CHECK(cmd_reply == resp::ok());
}

TEST_CASE_FIXTURE(unit_test_fixture, "GET") 
{
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id}, 
        resp::command{"GET"sv, "KEY1"sv}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id}, 
        resp::command{"SET"sv, "KEY1"sv, "VAL1"sv}
    );
    auto cmd_reply_3 = execute_command
    (
        Context_t{client_id}, 
        resp::command{"GET"sv, "KEY1"sv}
    );
    CHECK(cmd_reply_1 == resp::nil());
    CHECK(cmd_reply_2 == resp::ok());
    CHECK(cmd_reply_3 == resp::simple_string("VAL1"sv));
}

TEST_CASE_FIXTURE(unit_test_fixture, "EXISTS") 
{
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id}, 
        resp::command{"EXISTS"sv, "KEY1"sv}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id}, 
        resp::command{"SET"sv, "KEY1"sv, "VAL1"sv}
    );
    auto cmd_reply_3 = execute_command
    (
        Context_t{client_id},
        resp::command{"EXISTS"sv, "KEY1"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(0));
    CHECK(cmd_reply_2 == resp::ok());
    CHECK(cmd_reply_3 == resp::integer(1));
}

TEST_CASE_FIXTURE(unit_test_fixture, "KEYS") 
{
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id}, resp::command{ "KEYS"sv }
    );
    std::vector<std::string_view> keys{ "KEY1"sv, "KEY2"sv };
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id}, 
        resp::command{ "SET"sv, keys[0], "VAL1"sv }
    );
    auto cmd_reply_3 = execute_command
    (
        Context_t{client_id}, 
        resp::command{ "SET"sv, keys[1], "VAL2"sv }
    );
    auto cmd_reply_4 = execute_command
    (
        Context_t{client_id}, resp::command{ "KEYS"sv }
    );
    CHECK(cmd_reply_1 == resp::empty_array());
    CHECK(cmd_reply_2 == resp::ok());
    CHECK(cmd_reply_3 == resp::ok());
    resp::command_parser parser { cmd_reply_4.c_str() };
    auto reply_4 = *parser.parse();
    std::vector<std::string_view> vec(reply_4.begin(), reply_4.end());
    std::sort(vec.begin(), vec.end());
    CHECK(resp::array(vec.begin(), vec.end()) == resp::array(keys.begin(), keys.end()));
}

TEST_CASE_FIXTURE(unit_test_fixture, "DEL") 
{
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id}, 
        resp::command{ "SET"sv, "KEY1"sv, "VAL1"sv }
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id}, resp::command{ "DEL"sv, "KEY1"sv }
    );
    CHECK(cmd_reply_1 == resp::ok());
    CHECK(cmd_reply_2 == resp::integer(1));
}

TEST_CASE_FIXTURE(unit_test_fixture, "SADD") 
{
    auto cmd_reply = execute_command
    (
        Context_t{client_id},
        resp::command{"SADD"sv, "SET1"sv, "KEY1"sv}
    );
    CHECK(cmd_reply == resp::integer(1));
}

TEST_CASE_FIXTURE(unit_test_fixture, "SREM") 
{
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"SADD"sv, "SET1"sv, "KEY1"sv}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"SREM"sv, "SET1"sv, "KEY1"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(1));
    CHECK(cmd_reply_2 == resp::integer(1));
}

TEST_CASE_FIXTURE(unit_test_fixture, "SCARD") 
{
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"SADD"sv, "SET1"sv, "KEY1"sv, "KEY2"sv, "KEY3"sv, "KEY4"sv}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"SADD"sv, "SET1"sv, "KEY1"sv, "KEY2"sv, "KEY5"sv, "KEY6"sv}
    );
    auto cmd_reply_3 = execute_command
    (
        Context_t{client_id},
        resp::command{"SCARD"sv, "SET1"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(4));
    CHECK(cmd_reply_2 == resp::integer(2));
    CHECK(cmd_reply_3 == resp::integer(6));
}

TEST_CASE_FIXTURE(unit_test_fixture, "SMEMBERS") 
{
    std::vector<std::string_view> keys{ "KEY1"sv, "KEY2"sv, "KEY3"sv, "KEY4"sv };
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"SADD"sv, "SET1"sv, keys[0], keys[1], keys[2], keys[3]}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"SMEMBERS"sv, "SET1"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(4));
    CHECK(cmd_reply_2 == resp::array(keys.begin(), keys.end()));    
}

TEST_CASE_FIXTURE(unit_test_fixture, "SISMEMBER") 
{
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"SADD"sv, "SET1"sv, "KEY1"sv, "KEY2"sv, "KEY3"sv, "KEY4"sv}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"SISMEMBER"sv, "SET1"sv, "KEY1"sv}
    );
    auto cmd_reply_3 = execute_command
    (
        Context_t{client_id},
        resp::command{"SISMEMBER"sv, "SET1"sv, "KEY5"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(4));
    CHECK(cmd_reply_2 == resp::integer(1));
    CHECK(cmd_reply_3 == resp::integer(0));
}

TEST_CASE_FIXTURE(unit_test_fixture, "SINTER") 
{
    std::vector<std::string_view> keys{ "KEY1"sv, "KEY2"sv };
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"SADD"sv, "SET1"sv, keys[0], keys[1], "KEY3"sv, "KEY4"sv}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"SADD"sv, "SET2"sv, keys[0], keys[1], "KEY5"sv, "KEY6"sv}
    );
    auto cmd_reply_3 = execute_command
    (
        Context_t{client_id},
        resp::command{"SINTER"sv, "SET1"sv, "SET2"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(4));
    CHECK(cmd_reply_2 == resp::integer(4));
    CHECK(cmd_reply_3 == resp::array(keys.begin(), keys.end()));    
}

TEST_CASE_FIXTURE(unit_test_fixture, "SUNION") 
{
    std::vector<std::string_view> keys{ "KEY1"sv, "KEY2"sv, "KEY3"sv, "KEY4"sv, "KEY5"sv };
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"SADD"sv, "SET1"sv, keys[0], keys[1], keys[2]}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"SADD"sv, "SET2"sv, keys[2], keys[3], keys[4]}
    );
    auto cmd_reply_3 = execute_command
    (
        Context_t{client_id},
        resp::command{"SUNION"sv, "SET1"sv, "SET2"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(3));
    CHECK(cmd_reply_2 == resp::integer(3));
    CHECK(cmd_reply_3 == resp::array(keys.begin(), keys.end()));    
}

TEST_CASE_FIXTURE(unit_test_fixture, "ZADD") 
{
    auto cmd_reply = execute_command
    (
        Context_t{client_id},
        resp::command{"ZADD"sv, "ZSET1"sv, "4.5"sv, "KEY1"sv, "4.0"sv, "KEY2"sv, 
                                           "3.5"sv, "KEY3"sv, "3.0"sv, "KEY4"sv,
                                           "2.0"sv, "KEY5"sv, "1.0"sv, "KEY6"sv}
    );
    CHECK(cmd_reply == resp::integer(6));
}

TEST_CASE_FIXTURE(unit_test_fixture, "ZSCORE") 
{
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZADD"sv, "ZSET1"sv, "4.5"sv, "KEY1"sv, "4.0"sv, "KEY2"sv, 
                                           "3.5"sv, "KEY3"sv, "3.0"sv, "KEY4"sv,
                                           "2.0"sv, "KEY5"sv, "1.0"sv, "KEY6"sv}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZSCORE"sv, "ZSET1"sv, "KEY3"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(6));
    CHECK(cmd_reply_2 == resp::simple_string("3.5"));
}

TEST_CASE_FIXTURE(unit_test_fixture, "ZCARD") 
{
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZADD"sv, "ZSET1"sv, "4.5"sv, "KEY1"sv, "4.0"sv, "KEY2"sv, 
                                           "3.5"sv, "KEY3"sv, "3.0"sv, "KEY4"sv,
                                           "2.0"sv, "KEY5"sv, "1.0"sv, "KEY6"sv}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZCARD"sv, "ZSET1"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(6));
    CHECK(cmd_reply_2 == resp::integer(6));
}

TEST_CASE_FIXTURE(unit_test_fixture, "ZRANGE") 
{
    std::vector<std::string_view> keys{ "KEY1"sv, "KEY2"sv, "KEY3"sv, "KEY4"sv, "KEY5"sv, "KEY6"sv };
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZADD"sv, "ZSET1"sv, "4.5"sv, keys[0], "4.0"sv, keys[1], 
                                           "3.5"sv, keys[2], "3.0"sv, keys[3],
                                           "2.0"sv, keys[4], "1.0"sv, keys[5]}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZRANGE"sv, "ZSET1"sv, "1"sv, "2"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(6));
    CHECK(cmd_reply_2 == resp::array(keys.begin() + 1, keys.begin() + 3));
}

TEST_CASE_FIXTURE(unit_test_fixture, "ZRANGE WITHSCORES") 
{
    std::vector<std::string_view> key_score_pairs{ "KEY1"sv, "4.5"sv, "KEY2"sv, "4"sv, 
        "KEY3"sv, "3.5"sv, "KEY4"sv, "3"sv, "KEY5"sv, "2"sv, "KEY6"sv, "1"sv };
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZADD"sv, "ZSET1"sv, 
            key_score_pairs[1], key_score_pairs[0], 
            key_score_pairs[3], key_score_pairs[2], 
            key_score_pairs[5], key_score_pairs[4],
            key_score_pairs[7], key_score_pairs[6], 
            key_score_pairs[9], key_score_pairs[8],
            key_score_pairs[11], key_score_pairs[10]}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZRANGE"sv, "ZSET1"sv, "3"sv, "4"sv, "WITHSCORES"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(6));
    CHECK(cmd_reply_2 == resp::array(key_score_pairs.begin() + 6, key_score_pairs.begin() + 10));
}

TEST_CASE_FIXTURE(unit_test_fixture, "ZRANGE BYSCORE") 
{
    std::vector<std::string_view> keys{ "KEY1"sv, "KEY2"sv, "KEY3"sv, "KEY4"sv, "KEY5"sv, "KEY6"sv };
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZADD"sv, "ZSET1"sv, "4.5"sv, keys[0], "4.0"sv, keys[1], 
                                           "3.5"sv, keys[2], "3.0"sv, keys[3],
                                           "2.0"sv, keys[4], "1.0"sv, keys[5]}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZRANGE"sv, "ZSET1"sv, "2"sv, "4"sv, "BYSCORE"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(6));
    CHECK(cmd_reply_2 == resp::array(keys.rbegin() + 1, keys.rbegin() + 5));
}

TEST_CASE_FIXTURE(unit_test_fixture, "ZRANGE BYSCORES WITHSCORES") 
{
    std::vector<std::string_view> score_key_pairs{ "4.5"sv, "KEY1"sv, "4"sv, "KEY2"sv,
        "3.5"sv, "KEY3"sv, "3"sv, "KEY4"sv, "2"sv, "KEY5"sv, "1"sv, "KEY6"sv };
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZADD"sv, "ZSET1"sv, 
            score_key_pairs[0], score_key_pairs[1], 
            score_key_pairs[2], score_key_pairs[3], 
            score_key_pairs[4], score_key_pairs[5],
            score_key_pairs[6], score_key_pairs[7], 
            score_key_pairs[8], score_key_pairs[9],
            score_key_pairs[10], score_key_pairs[11]}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZRANGE"sv, "ZSET1"sv, "2"sv, "4"sv, "BYSCORE"sv, "WITHSCORES"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(6));
    CHECK(cmd_reply_2 == resp::array(score_key_pairs.rbegin() + 2, score_key_pairs.rbegin() + 10));
}

TEST_CASE_FIXTURE(unit_test_fixture, "ZREMRANGEBYSCORE") 
{
    std::vector<std::string_view> score_key_pairs{ "4.5"sv, "KEY1"sv, "4"sv, "KEY2"sv,
        "3.5"sv, "KEY3"sv, "3"sv, "KEY4"sv, "2"sv, "KEY5"sv, "1"sv, "KEY6"sv };
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZADD"sv, "ZSET1"sv, 
            score_key_pairs[0], score_key_pairs[1], 
            score_key_pairs[2], score_key_pairs[3], 
            score_key_pairs[4], score_key_pairs[5],
            score_key_pairs[6], score_key_pairs[7], 
            score_key_pairs[8], score_key_pairs[9],
            score_key_pairs[10], score_key_pairs[11]}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZREMRANGEBYSCORE"sv, "ZSET1"sv, "2"sv, "4"sv}
    );
    auto cmd_reply_3 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZCARD"sv, "ZSET1"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(6));
    CHECK(cmd_reply_2 == resp::integer(4));
    CHECK(cmd_reply_3 == resp::integer(2));
}

TEST_CASE_FIXTURE(unit_test_fixture, "ZREM") 
{
    std::vector<std::string_view> score_key_pairs{ "4.5"sv, "KEY1"sv, "4"sv, "KEY2"sv,
        "3.5"sv, "KEY3"sv, "3"sv, "KEY4"sv, "2"sv, "KEY5"sv, "1"sv, "KEY6"sv };
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZADD"sv, "ZSET1"sv, 
            score_key_pairs[0], score_key_pairs[1], 
            score_key_pairs[2], score_key_pairs[3], 
            score_key_pairs[4], score_key_pairs[5],
            score_key_pairs[6], score_key_pairs[7], 
            score_key_pairs[8], score_key_pairs[9],
            score_key_pairs[10], score_key_pairs[11]}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZREM"sv, "ZSET1"sv, "KEY2"sv, "KEY4"sv}
    );
    auto cmd_reply_3 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZCARD"sv, "ZSET1"sv}
    );
    auto cmd_reply_4 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZRANGE"sv, "ZSET1"sv, "3"sv, "4"sv, "BYSCORE"sv}
    );
    CHECK(cmd_reply_1 == resp::integer(6));
    CHECK(cmd_reply_2 == resp::integer(2));
    CHECK(cmd_reply_3 == resp::integer(4));
    CHECK(cmd_reply_4 == resp::array(score_key_pairs.begin() + 5, score_key_pairs.begin() + 6));
}

TEST_CASE_FIXTURE(unit_test_fixture, "TYPE") 
{
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"SET"sv, "KEY1"sv, "VAL1"sv}            
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"SADD"sv, "SET1"sv, "KEY1"sv}            
    );
    auto cmd_reply_3 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZADD"sv, "ZSET1"sv, "99.9"sv, "KEY1"sv}
    );
    auto cmd_reply_4 = execute_command
    (
        Context_t{client_id},
        resp::command{"TYPE"sv, "KEY1"sv}
    );
    auto cmd_reply_5 = execute_command
    (
        Context_t{client_id},
        resp::command{"TYPE"sv, "SET1"sv}
    );
    auto cmd_reply_6 = execute_command
    (
        Context_t{client_id},
        resp::command{"TYPE"sv, "ZSET1"sv}
    );
    CHECK(cmd_reply_1 == resp::ok());
    CHECK(cmd_reply_2 == resp::integer(1));
    CHECK(cmd_reply_3 == resp::integer(1));
    CHECK(cmd_reply_4 == resp::simple_string("string"));
    CHECK(cmd_reply_5 == resp::simple_string("set"));
    CHECK(cmd_reply_6 == resp::simple_string("zset"));
}

TEST_CASE_FIXTURE(unit_test_fixture, "SELECT") 
{
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"SET"sv, "KEY1"sv, "VAL1"sv}            
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"SELECT"sv, "1"sv}
    );
    auto cmd_reply_3 = execute_command
    (
        Context_t{client_id},
        resp::command{"SET"sv, "KEY1"sv, "VAL1"sv}           
    );
    CHECK(cmd_reply_1 == resp::ok());
    CHECK(cmd_reply_2 == resp::ok());
    CHECK(cmd_reply_3 == resp::ok());
}

TEST_CASE_FIXTURE(unit_test_fixture, "FLUSHDB") 
{
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"SET"sv, "KEY1"sv, "VAL1"sv}
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"DBSIZE"sv}
    );
    auto cmd_reply_3 = execute_command
    (
        Context_t{client_id},
        resp::command{"FLUSHDB"sv}
    );
    auto cmd_reply_4 = execute_command
    (
        Context_t{client_id},
        resp::command{"DBSIZE"sv}
    );
    CHECK(cmd_reply_1 == resp::ok());
    CHECK(cmd_reply_2 == resp::integer(1));
    CHECK(cmd_reply_3 == resp::ok());
    CHECK(cmd_reply_4 == resp::integer(0));
}

TEST_CASE_FIXTURE(unit_test_fixture, "DBSIZE") 
{
    auto cmd_reply_1 = execute_command
    (
        Context_t{client_id},
        resp::command{"SET"sv, "KEY1"sv, "VAL1"sv}            
    );
    auto cmd_reply_2 = execute_command
    (
        Context_t{client_id},
        resp::command{"SADD"sv, "SET1"sv, "KEY1"sv}            
    );
    auto cmd_reply_3 = execute_command
    (
        Context_t{client_id},
        resp::command{"ZADD"sv, "ZSET1"sv, "99.9"sv, "KEY1"sv}
    );
    auto cmd_reply_4 = execute_command
    (
        Context_t{client_id},
        resp::command{"DBSIZE"sv}
    );
    CHECK(cmd_reply_1 == resp::ok());
    CHECK(cmd_reply_2 == resp::integer(1));
    CHECK(cmd_reply_3 == resp::integer(1));
    CHECK(cmd_reply_4 == resp::integer(3));
}

TEST_CASE_FIXTURE(unit_test_fixture, "PING") 
{
    auto cmd_reply = execute_command
    (
        Context_t{client_id},
        resp::command{"PING"sv}
    );
    CHECK(cmd_reply == resp::pong());
}