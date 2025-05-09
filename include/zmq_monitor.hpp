//Source code C++ MasterClass (KV Store project) by Fabio Galuppo
//C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
//Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
//May 2025

#ifndef ZMQ_MONITOR_HPP
#define ZMQ_MONITOR_HPP

#include <zmq.h>

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

/*
    | Event name                             | Event code  | Value meaning                          | Address meaning                            |
    |:---------------------------------------|:------------|:---------------------------------------|:-------------------------------------------|
    | `ZMQ_EVENT_CONNECTED`                  | `0x0001`    | FD (file descriptor of connection)     | Address string (the endpoint connected to) |
    | `ZMQ_EVENT_CONNECT_DELAYED`            | `0x0002`    | errno value (why it was delayed)       | Address string (the endpoint)              |
    | `ZMQ_EVENT_CONNECT_RETRIED`            | `0x0004`    | Retry interval in milliseconds         | Address string (the endpoint)              |
    | `ZMQ_EVENT_LISTENING`                  | `0x0008`    | FD (listening socket)                  | Address string (the endpoint bound)        |
    | `ZMQ_EVENT_BIND_FAILED`                | `0x0010`    | errno value (reason for failure)       | Address string (the endpoint)              |
    | `ZMQ_EVENT_ACCEPTED`                   | `0x0020`    | FD (accepted connection)               | Address string (the peer address)          |
    | `ZMQ_EVENT_ACCEPT_FAILED`              | `0x0040`    | errno value (reason for failure)       | Address string (the endpoint)              |
    | `ZMQ_EVENT_CLOSED`                     | `0x0080`    | FD (closed connection)                 | Address string (the endpoint)              |
    | `ZMQ_EVENT_CLOSE_FAILED`               | `0x0100`    | errno value (reason for failure)       | Address string (the endpoint)              |
    | `ZMQ_EVENT_DISCONNECTED`               | `0x0200`    | FD (disconnected connection)           | Address string (the peer address)          |
    | `ZMQ_EVENT_MONITOR_STOPPED`            | `0x0400`    | 0 (no additional info)                 | empty string                               |
    | `ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL` | `0x0800`    | 0                                      | Address string (the peer address)          |
    | `ZMQ_EVENT_HANDSHAKE_SUCCEEDED`        | `0x1000`    | Mechanism enum value (eg CURVE, PLAIN) | Address string (the peer address)          |
    | `ZMQ_EVENT_HANDSHAKE_FAILED_PROTOCOL`  | `0x2000`    | 0                                      | Address string (the peer address)          |
    | `ZMQ_EVENT_HANDSHAKE_FAILED_AUTH`      | `0x4000`    | 0                                      | Address string (the peer address)          |
*/
struct zmq_event_type
{
    std::uint16_t event_code;
    std::int32_t  value;
};

class zmq_monitor_type
{
    void* ctx_;
    void* socket_;
    void *monitor_socket_;
    volatile bool running_;
public:
    explicit zmq_monitor_type(void* context) noexcept
        : ctx_{context}, socket_{nullptr}, monitor_socket_{nullptr}, running_{false}{}

    virtual ~zmq_monitor_type() noexcept
    {
        abort();
    }

    zmq_monitor_type(const zmq_monitor_type&) = delete;
    zmq_monitor_type& operator=(const zmq_monitor_type&) = delete;
    zmq_monitor_type& operator=(zmq_monitor_type&&) = delete;
    zmq_monitor_type(zmq_monitor_type&& that) noexcept :
        ctx_(that.ctx_),
        socket_(that.socket_),
        monitor_socket_(that.monitor_socket_),
        running_(that.running_)
    {
        that.ctx_ = nullptr;
        that.socket_ = nullptr;
        that.monitor_socket_ = nullptr;
        that.running_ = false;
    }

    void monitor(void* socket, const std::string& addr, int timeout, int events = ZMQ_EVENT_ALL)
    {
        monitor(socket, addr.c_str(), events);
    }

    void monitor(void* socket, const char* addr, int timeout, int events = ZMQ_EVENT_ALL)
    {
        init(socket, addr, events);
        while(running_) check_event(timeout);
    }

private:
    void init(void* socket, const char* addr, int events = ZMQ_EVENT_ALL)
    {
        int rc = zmq_socket_monitor(socket, addr, events);
        if (rc != 0)
            throw std::runtime_error(zmq_strerror(zmq_errno()));
        socket_ = socket;
        monitor_socket_ = zmq_socket(ctx_, ZMQ_PAIR);
        if (monitor_socket_ == nullptr)
            throw std::runtime_error(zmq_strerror(zmq_errno()));
        rc = zmq_connect(monitor_socket_, addr);
        if (rc != 0)
            throw std::runtime_error(zmq_strerror(zmq_errno()));
        running_ = true;
        on_monitor_started();
    }

    bool check_event(int timeout = 0) const
    {
        if (!monitor_socket_)
            return false;

        zmq_msg_t event_msg;
        zmq_msg_init(&event_msg);
        zmq_pollitem_t items[]{ { monitor_socket_, 0, ZMQ_POLLIN, 0 } };
        zmq_poll(&items[0], 1, timeout);
        if (items[0].revents & ZMQ_POLLIN)
        {
            int rc = zmq_msg_recv(&event_msg, monitor_socket_, 0);
            if (rc == -1 && zmq_errno() == ETERM)
            {
                zmq_msg_close(&event_msg);    
                return false;
            }
        }
        else
        {
            zmq_msg_close(&event_msg);
            return false;
        }

        const char* data = static_cast<const char*>(zmq_msg_data(&event_msg));
        zmq_event_type msg_event;
        std::memcpy(&msg_event.event_code, data, sizeof(uint16_t)); data += sizeof(uint16_t);
        std::memcpy(&msg_event.value, data, sizeof(int32_t));
        zmq_event_type* event = &msg_event;

        zmq_msg_t addr_msg;
        zmq_msg_init (&addr_msg);
        int rc = zmq_msg_recv(&addr_msg, monitor_socket_, 0);
        if (rc == -1 && zmq_errno() == ETERM)
        {
            zmq_msg_close(&event_msg);
            return false;
        }

        const char* str = static_cast<const char*>(zmq_msg_data(&addr_msg));
        std::string address(str, str + zmq_msg_size(&addr_msg));
        zmq_msg_close(&addr_msg);

        if (event->event_code == ZMQ_EVENT_MONITOR_STOPPED)
        {
            zmq_msg_close(&event_msg);
            return true;
        }

        switch (event->event_code) {
        case ZMQ_EVENT_CONNECTED:
            on_event_connected(*event, address.c_str());
            break;
        case ZMQ_EVENT_CONNECT_DELAYED:
            on_event_connect_delayed(*event, address.c_str());
            break;
        case ZMQ_EVENT_CONNECT_RETRIED:
            on_event_connect_retried(*event, address.c_str());
            break;
        case ZMQ_EVENT_LISTENING:
            on_event_listening(*event, address.c_str());
            break;
        case ZMQ_EVENT_BIND_FAILED:
            on_event_bind_failed(*event, address.c_str());
            break;
        case ZMQ_EVENT_ACCEPTED:
            on_event_accepted(*event, address.c_str());
            break;
        case ZMQ_EVENT_ACCEPT_FAILED:
            on_event_accept_failed(*event, address.c_str());
            break;
        case ZMQ_EVENT_CLOSED:
            on_event_closed(*event, address.c_str());
            break;
        case ZMQ_EVENT_CLOSE_FAILED:
            on_event_close_failed(*event, address.c_str());
            break;
        case ZMQ_EVENT_DISCONNECTED:
            on_event_disconnected(*event, address.c_str());
            break;
        case ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL:
            on_event_handshake_failed_no_detail(*event, address.c_str());
            break;
        case ZMQ_EVENT_HANDSHAKE_SUCCEEDED:
            on_event_handshake_succeed(*event, address.c_str());
            break;
        case ZMQ_EVENT_HANDSHAKE_FAILED_PROTOCOL:
            on_event_handshake_failed_protocol(*event, address.c_str());
            break;
        case ZMQ_EVENT_HANDSHAKE_FAILED_AUTH:
            on_event_handshake_failed_auth(*event, address.c_str());
            break;
        default:
            on_event_unknown(*event, address.c_str());
            break;
        }
        zmq_msg_close(&event_msg);
        return true ;
    }

    void abort()
    {
        if (socket_)
            zmq_socket_monitor(socket_, nullptr, 0);
        if (monitor_socket_)
            zmq_close(monitor_socket_);        
        ctx_ = nullptr;
        socket_ = nullptr;
        monitor_socket_ = nullptr;
        running_ = false;
    }

public:
    virtual void on_monitor_started() const {}
    virtual void on_event_connected(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
    virtual void on_event_connect_delayed(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
    virtual void on_event_connect_retried(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
    virtual void on_event_listening(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
    virtual void on_event_bind_failed(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
    virtual void on_event_accepted(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
    virtual void on_event_accept_failed(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
    virtual void on_event_closed(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
    virtual void on_event_close_failed(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
    virtual void on_event_disconnected(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
    virtual void on_event_handshake_failed_no_detail(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
    virtual void on_event_handshake_succeed(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
    virtual void on_event_handshake_failed_protocol(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
    virtual void on_event_handshake_failed_auth(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
    virtual void on_event_unknown(const zmq_event_type& ev, const char* addr) const { (void)ev; (void)addr; }
};

#endif /* ZMQ_MONITOR_HPP */