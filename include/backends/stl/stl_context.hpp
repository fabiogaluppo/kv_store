//Source code C++ MasterClass (KV Store project) by Fabio Galuppo
//C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
//Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
//May 2025

#ifndef STL_CONTEXT_HPP
#define STL_CONTEXT_HPP

#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#include "stl_databases.hpp"

struct Client_t final
{
    std::string Id;
    int ClientNumber;
    int CurrentDbNumber = 0;
    std::string LibName, LibVersion, ConnectionName;
    
    static inline int ClientCounter = 0;
    static Client_t create(const std::string& id) 
    { 
        return Client_t{id, ++ClientCounter}; 
    }

    STL_Database_t& CurrentDb() const
    {
        return g_databases[CurrentDbNumber];
    }
    
    bool CurrentDb(int db_num)
    {
        if (db_num < g_databases.size())
        {
            CurrentDbNumber = db_num;
            return true;
        }
        return false;
    }

    std::string to_string() const
    {
        std::stringstream ss;
        ss << "db=" << CurrentDbNumber 
           << " id=" << ClientNumber           
           << " lib-name=" << LibName
           << " lib-ver=" << LibVersion;
        //unhandled/not supported
        ss << " age=0"
           << " argv-mem=0"
           << " idle=0"
           << " multi=0"
           << " obl=0"
           << " oll=0"
           << " omem=0"
           << " sub=0"
           << " psub=0"
           << " qbuf=0"
           << " qbuf-free=0"
           << " tot-mem=0";
        return std::move(ss.str());
    }
};

std::unordered_map<std::string, Client_t> g_clients;

//don't reuse context
class Context_t final
{
    Client_t& client;
public:
    Context_t(const std::string& client_id)
        : client{g_clients[client_id]}{}
    Context_t() = delete;
    ~Context_t() = default;
    Context_t(const Context_t&) = delete;
    Context_t& operator=(const Context_t&) = delete;
    Context_t(Context_t&&) = delete;
    Context_t& operator=(Context_t&&) = delete;

    Client_t& Client() const { return client; }

    static std::pair<int, bool> create_or_remove_client(const std::string& client_id)
    {
        int client_number;
        bool created;
        auto it = g_clients.find(client_id);
        if (it != g_clients.end())
        {
            client_number = it->second.ClientNumber;
            g_clients.erase(it);
            created = false;
        }
        else
        {
            auto client = Client_t::create(client_id);
            g_clients.emplace(client_id, client);
            client_number = client.ClientNumber;
            created = true;
        }
        return std::make_pair(client_number, created);
    }
};

#endif /* STL_CONTEXT_HPP */