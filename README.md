# (C++ MasterClass's) KV Store
### _A Lightweight In-Memory Key-Value Store for Learning and Exploration_
- 🟢 **Easy to learn** — Designed for students and curious minds  
- ⚙️ **Real-world architecture** — Implements RESP and ZeroMQ 
- 🎓 **Didactic focus** — Ideal for teaching databases, systems, and performance fundamentals

---

### 📚 Overview

This project is a **didactic in-memory NoSQL database** built using **modern C++ (C++20)**. Developed as part of the **C++ MasterClass initiative**, it teaches how to implement a real-world key-value store from scratch, covering important systems programming concepts and protocols.

It is fully **RESP-compatible**, meaning you can use **Redis CLI** and clients to connect, issue commands like `SET`, `GET`, and `PING`, and receive appropriate responses.

---

### 🧩 Core Features

- **RESP protocol parsing and serialization** — inspired by Redis, implemented in `resp_command_parser.hpp`
- **TCP socket communication** via **ZeroMQ STREAM** sockets — implemented in `server_main.cpp`
- **Pluggable backends** (default: STL) for flexible storage — see `backend.hpp`, `stl_backend.hpp`
- **Extensive command support**:
  - Strings: `SET`, `GET`, `DEL`, `EXISTS`
  - Sets: `SADD`, `SREM`, `SCARD`, `SMEMBERS`, `SINTER`, `SUNION`
  - Sorted Sets: `ZADD`, `ZSCORE`, `ZCARD`, `ZRANGE`, `ZREM`, `ZREMRANGEBYSCORE`
  - Database ops: `FLUSHDB`, `SELECT`, `DBSIZE`, `TYPE`
  - Meta: `PING`, `CLIENT`, etc.
- **Extensible command execution engine** — `execute_command.hpp`
- **ZeroMQ connection monitoring** for logging client connections — `zmq_monitor.hpp`
- **Integration-tested** using real Redis clients over TCP

---

### 🗂️ Source Code Structure

| File                     | Description                                                        |
|--------------------------|--------------------------------------------------------------------|
| `server_main.cpp`        | Entry point; sets up ZeroMQ socket, handles RESP traffic           |
| `resp_command_parser.hpp`| Parses RESP messages from Redis clients                            |
| `resp_command.hpp`       | Holds RESP command abstraction and parsing logic                   |
| `execute_command.hpp`    | Routes parsed commands to backend strategies                       |
| `stl_strategy.hpp`       | Implements Redis-like commands using STL containers                |
| `stl_backend.hpp`        | Chooses STL as backend and connects context/database/strategy      |
| `backend.hpp`            | Backend selector for STL or EASTL                                  |
| `zmq_monitor.hpp`        | Observes client connect/disconnect using ZeroMQ monitor API        |

---

### 🚀 Future Enhancements

This project serves as a foundation for deeper exploration into key-value store design and distributed systems. Future enhancements may include:

- ➕ **Expanding Command Set** — Add support for additional Redis-compatible commands to broaden functionality  
- 🧩 **Plugin Architecture** — Introduce a modular plugin system to extend behavior without modifying the core  
- 🔐 **Authentication Support** — Implement `AUTH` command for client access control and security layers  
- ⏱️ **Key Expiration and TTL** — Add time-based key eviction via `TTL` and `EXPIRE` support  
- 🚦 **Protective Limits** — Enforce memory limits, key quotas, and input validation safeguards  
- 🧱 **Improved Resilience** — Enhance stability under high load or malformed requests  
- ⚡ **Performance Optimizations** — Optimize storage and protocol handling for faster throughput  
- 🧵 **Asynchronous Execution** — Support internal async command execution for concurrency and scalability  

---
## License

MIT

**Free Software, Hell Yeah!**