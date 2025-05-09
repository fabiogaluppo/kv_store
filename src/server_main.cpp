//Source code C++ MasterClass (KV Store project) by Fabio Galuppo
//C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
//Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
//May 2025

#include <zmq.h>

#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include "argparse/argparse.hpp"
#include "backend.hpp"
#include "cppcodec/base64_rfc4648.hpp"
#include "execute_command.hpp"
#include "format.hpp"
#include "logger.hpp"
#include "resp_command.hpp"
#include "resp_command_parser.hpp"
#include "zmq_monitor.hpp"

#include "eastl_stub_allocator.inl"

class monitor_zmq_socket final : private zmq_monitor_type 
{
    std::thread monitor_thread;
public:
    monitor_zmq_socket(void* context, void* socket, const char* address, int timeout = -1) 
        : zmq_monitor_type{context}
    {
        if (!context)
            throw std::invalid_argument("context is null");
        if (!socket)
            throw std::invalid_argument("socket is null");        
        monitor_thread = std::move(std::thread([&](){ 
            monitor(socket, address, timeout, ZMQ_EVENT_ALL);
        }));
        monitor_thread.detach();
    }

private:
    virtual void on_event_accepted(const zmq_event_type& ev, const char* addr) const 
    {
        LOG_TRACE_L1("Client accepted fd: {} addr: {}", ev.value, addr);
    }

    virtual void on_event_disconnected(const zmq_event_type& ev, const char* addr) const 
    {
        LOG_TRACE_L1("Client disconnected fd: {} addr: {}", ev.value, addr);
    }    
};

template<typename T, typename UnaryOperator>
std::optional<std::pair<T, int>> read(void* socket, UnaryOperator op)
{
    int more;
    std::size_t more_size = sizeof(more);
    T result{};
    zmq_msg_t msg;
    int rc = zmq_msg_init(&msg);
    if (rc != 0) return std::nullopt;
    rc = zmq_msg_recv(&msg, socket, 0);
    if (rc > 0)
    {
        char* ptr = static_cast<char*>(zmq_msg_data(&msg));
        std::size_t size = zmq_msg_size(&msg);
        if (size) result = op(ptr, size);
        zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
    }    
    zmq_msg_close(&msg);
    return std::make_pair(result, more);
}

std::optional<std::pair<std::string, std::string>> request_handler(void* socket)
{
    std::string id, payload;
    auto id_opt = read<std::string>(socket, [](char* ptr, std::size_t size){ return cppcodec::base64_rfc4648::encode(ptr, size); });
    int more = (*id_opt).second;
    if (more)
    {
        id = (*id_opt).first;
        auto payload_opt = read<std::string>(socket, [](char* ptr, std::size_t size){ return std::string(ptr, ptr + size); });
        payload = (*payload_opt).first;
    }
    return std::make_pair(id, payload);
}

inline std::optional<std::vector<std::string_view>> parse_resp_command(const std::string& payload)
{
    try
    {
        resp::command_parser parser { payload.c_str() };
        return parser.parse();
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Error: {}", e.what());
    }
    catch(...)
    {
        LOG_ERROR("Unknown error");
    }
    return std::nullopt;
}

struct args final
{
    int tcp_port;
};

args parse_args(int argc, char* argv[])
{
    argparse::ArgumentParser arg_parser("kv_store");
    arg_parser.add_argument("--port")
              .help("tcp port number (1024–49151)")
              .nargs(1)
              .scan<'i', int>()              
              .action([](const std::string& value) {
                        int port = std::stoi(value);
                        if (port < 1024 || port > 49151)
                            throw std::out_of_range("Port must be between 1024 and 49151.");
                        return value;
                     });
    int tcp_port;
    try
    {
        arg_parser.parse_args(argc, argv);
        tcp_port = 1234;
        if (arg_parser.is_used("--port"))
            tcp_port = arg_parser.get<int>("--port");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        std::cerr << arg_parser;
        std::exit(1);
    }
    return { tcp_port };
}

std::string execute_command(Context_t&& ctx, resp::command&& cmd)
{
    using std::chrono::high_resolution_clock;
    using microseconds = std::chrono::duration<double, std::micro>;
    std::string reply;
    bool unk_cmd{};
    const auto t_s = high_resolution_clock::now();    
    try
    {
        reply = execute_command<Context_t, Strategy_t>(ctx, cmd, unk_cmd);
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("Error: {}", e.what());
    }
    catch(...)
    {
        LOG_ERROR("Unknown error");
    }
    const auto t_e = high_resolution_clock::now();
    const microseconds diff = t_e - t_s;
    if (unk_cmd)
        LOG_WARNING("Invalid command {} executed for client {} in {}", cmd.name(), ctx.Client().ClientNumber, diff);
    else
        LOG_INFO("Command {} executed for client {} in {}", cmd.name(), ctx.Client().ClientNumber, diff);
    return reply;
}

void tune_zmq_socket(void* s)
{
    int no_linger = 0;
    zmq_setsockopt(s, ZMQ_LINGER, &no_linger, sizeof(no_linger));
}

volatile bool running = true;
void sigint_handler(int) 
{
    std::cout << "\nThe server is shutting down gracefully. Closing active connections and releasing resources...\n";
    running = false;
}

int main(int argc, char* argv[])
{
    const char* banner = R"( __  __ ___ ___      _______ _______ _______ ______ _______ )""\n"
                         R"(|  |/  |   |   |    |     __|_     _|       |   __ \    ___|)""\n"
                         R"(|     <|   |   |    |__     | |   | |   -   |      <    ___|)""\n"
                         R"(|__|\__|\_____/     |_______| |___| |_______|___|__|_______|)""\n"
                         R"(C++ MasterClass  - cppmasterclass.com.br -  2025 version 0.1)""\n";
    std::cout << banner << '\n';

    int major, minor, patch;
    zmq_version(&major, &minor, &patch);
    std::cout << format::zmq_version_string(major, minor, patch, g_backend);
    
    signal(SIGINT, sigint_handler);

    auto args = parse_args(argc, argv);

    void* ctx = zmq_ctx_new();
    if (ctx)
    {
        void* stream_socket = zmq_socket(ctx, ZMQ_STREAM);
        if (stream_socket)
        {
            tune_zmq_socket(stream_socket);
            auto socket_address = format::zmq_tcp_address(args.tcp_port);
            int rc = zmq_bind(stream_socket, socket_address.c_str());
            LOG_TRACE_L1("Listening at {}", socket_address);
            if (rc == 0)
            {
                const int TIMEOUT_IN_MS = 3 * 1000;
                const char* monitor_address = "inproc://socket-monitor";
                monitor_zmq_socket monitor(ctx, stream_socket, monitor_address, TIMEOUT_IN_MS);
                while (running)
                {
                    zmq_pollitem_t events[]{ { stream_socket, 0, ZMQ_POLLIN, 0 } };
                    rc = zmq_poll(&events[0], 1, TIMEOUT_IN_MS);
                    if (!running) break;
                    if (rc == 0 || rc == -1) continue;                    
                    for (int i = 0; i < 1; ++i)
                    {
                        if (events[i].socket == stream_socket && (events[i].events & ZMQ_POLLIN))
                        {
                            auto req_opt = request_handler(stream_socket);
                            if (req_opt)
                            {
                                auto [client_id, payload] = *req_opt;
                                if (!payload.empty())
                                {
                                    auto cmd_opt = parse_resp_command(payload);
                                    if (cmd_opt)
                                    {
                                        auto cmd_reply = execute_command(Context_t{client_id}, resp::command(std::move(*cmd_opt)));
                                        using base64 = cppcodec::base64_rfc4648;
                                        auto id = base64::decode(client_id);
                                        zmq_send(stream_socket, id.data(), id.size(), ZMQ_SNDMORE);
                                        zmq_send(stream_socket, cmd_reply.c_str(), cmd_reply.size(), 0);
                                    }
                                    else
                                    {
                                        LOG_WARNING("Invalid command: {}", payload);                                        
                                    }
                                }
                                else
                                {
                                    auto client = Context_t::create_or_remove_client(client_id);
                                    LOG_INFO("Client {} {}", client.first, client.second ? "created" : "removed");
                                }
                            }
                        }
                    }
                }
            }
            zmq_close (stream_socket);
        }
        zmq_ctx_term(ctx);
    }
    return 0;
}