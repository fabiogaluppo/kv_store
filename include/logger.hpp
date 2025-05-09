//Source code C++ MasterClass (KV Store project) by Fabio Galuppo
//C++ MasterClass - https://www.linkedin.com/company/cppmasterclass - https://cppmasterclass.com.br/
//Fabio Galuppo - http://member.acm.org/~fabiogaluppo - fabiogaluppo@acm.org
//May 2025

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "quill/Backend.h"
#include "quill/Frontend.h"
#define QUILL_DISABLE_NON_PREFIXED_MACROS
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/ConsoleSink.h"
#include "quill/std/Array.h"
#include "quill/std/Chrono.h"
#include "quill/std/Deque.h"
#include "quill/std/FilesystemPath.h"
#include "quill/std/ForwardList.h"
#include "quill/std/List.h"
#include "quill/std/Map.h"
#include "quill/std/Optional.h"
#include "quill/std/Pair.h"
#include "quill/std/Set.h"
#include "quill/std/Tuple.h"
#include "quill/std/UnorderedMap.h"
#include "quill/std/UnorderedSet.h"
#include "quill/std/Vector.h"
#include "quill/std/WideString.h"

class Logger final
{
    quill::Logger* logger;
public:
    Logger()
    {
        quill::BackendOptions backend_options;
        quill::Backend::start(backend_options);
        auto console_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("kv_store_sink");
        logger = quill::Frontend::create_or_get_logger("root", std::move(console_sink));
        logger->set_log_level(quill::LogLevel::TraceL3);
    }
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    quill::Logger* get() const noexcept { return logger; }
};

static Logger g_logger;

#define LOG_TRACE_L3(fmt, ...) QUILL_LOG_TRACE_L3(g_logger.get(), fmt, ##__VA_ARGS__)
#define LOG_TRACE_L2(fmt, ...) QUILL_LOG_TRACE_L2(g_logger.get(), fmt, ##__VA_ARGS__)
#define LOG_TRACE_L1(fmt, ...) QUILL_LOG_TRACE_L1(g_logger.get(), fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) QUILL_LOG_DEBUG(g_logger.get(), fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) QUILL_LOG_INFO(g_logger.get(), fmt, ##__VA_ARGS__)
#define LOG_NOTICE(fmt, ...) QUILL_LOG_NOTICE(g_logger.get(), fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) QUILL_LOG_WARNING(g_logger.get(), fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) QUILL_LOG_ERROR(g_logger.get(), fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) QUILL_LOG_CRITICAL(g_logger.get(), fmt, ##__VA_ARGS__)

#endif /* LOGGER_HPP */