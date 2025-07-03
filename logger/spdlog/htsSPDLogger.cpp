// Copyright(c) 2025-present, 徐翔.
// Distributed under the Apache License Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)

#define _CRT_SECURE_NO_WARNINGS
#include "htsSPDLogger.h"
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#ifdef HAS_SYSLOG
#include <spdlog/sinks/syslog_sink.h>
#endif
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include <htsPathHelper.h>
#include "thread_formatter.h"

static bool g_SPDLoggerRegisted = IhtsPluginClassFactory::RegistClass<hts::ILogger, SPDLogger>();

SPDLogger::~SPDLogger()
{
    // 调用flush会导致程序退出时，spdlog的全局静态变量（主要是thread_pool)先析构时抛异常
    // 考虑到程序运行时调用可能会损害性能，干脆去掉这个调用。
    //if (m_logger)
    //{
    //    m_logger->flush();
    //}
}

void SPDLogger::SetInstanceName(const char* name)
{
	IhtsPluginClass::SetInstanceName(name);
    m_logger = spdlog::get(name);
}

const constexpr static char* SINK_TYPE_STDOUT_SINK_ST = "stdout_sink_st";
const constexpr static char* SINK_TYPE_STDOUT_SINK_MT = "stdout_sink_mt";
const constexpr static char* SINK_TYPE_STDERR_SINK_ST = "stderr_sink_st";
const constexpr static char* SINK_TYPE_STDERR_SINK_MT = "stderr_sink_mt";
const constexpr static char* SINK_TYPE_STDOUT_COLOR_SINK_ST = "stdout_color_sink_st";
const constexpr static char* SINK_TYPE_STDOUT_COLOR_SINK_MT = "stdout_color_sink_mt";
const constexpr static char* SINK_TYPE_STDERR_COLOR_SINK_ST = "stderr_color_sink_st";
const constexpr static char* SINK_TYPE_STDERR_COLOR_SINK_MT = "stderr_color_sink_mt";
#ifdef HAS_SYSLOG
const constexpr static char* SINK_TYPE_SYSLOG_SINK_ST = "syslog_sink_st";
const constexpr static char* SINK_TYPE_SYSLOG_SINK_MT = "syslog_sink_mt";
#endif
const constexpr static char* SINK_TYPE_BASIC_FILE_SINK_ST = "basic_file_sink_st";
const constexpr static char* SINK_TYPE_BASIC_FILE_SINK_MT = "basic_file_sink_mt";
const constexpr static char* SINK_TYPE_DAILY_FILE_SINK_ST = "daily_file_sink_st";
const constexpr static char* SINK_TYPE_DAILY_FILE_SINK_MT = "daily_file_sink_mt";
const constexpr static char* SINK_TYPE_ROTATING_FILE_SINK_ST = "rotating_file_sink_st";
const constexpr static char* SINK_TYPE_ROTATING_FILE_SINK_MT = "rotating_file_sink_mt";
const constexpr static char* SINK_TYPE_MSVC_SINK_ST = "msvc_sink_st";
const constexpr static char* SINK_TYPE_MSVC_SINK_MT = "msvc_sink_mt";

std::shared_ptr<spdlog::sinks::sink> CreateSink(SPD_SINK_CONFIG& config)
{
    std::shared_ptr<spdlog::sinks::sink> ret;
    std::string sink_type(config.name);

    if (sink_type == SINK_TYPE_STDOUT_SINK_ST) {
        ret = std::make_shared<spdlog::sinks::stdout_sink_st>();
    }
    else if (sink_type == SINK_TYPE_STDOUT_SINK_MT) {
        ret = std::make_shared<spdlog::sinks::stdout_sink_mt>();
    }
    else if (sink_type == SINK_TYPE_STDERR_SINK_ST) {
        ret = std::make_shared<spdlog::sinks::stderr_sink_st>();
    }
    else if (sink_type == SINK_TYPE_STDERR_SINK_MT) {
        ret = std::make_shared<spdlog::sinks::stderr_sink_mt>();
    }
    else if (sink_type == SINK_TYPE_STDOUT_COLOR_SINK_ST) {
        ret = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    }
    else if (sink_type == SINK_TYPE_STDOUT_COLOR_SINK_MT) {
        ret = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    }
    else if (sink_type == SINK_TYPE_STDERR_COLOR_SINK_ST) {
        ret = std::make_shared<spdlog::sinks::stderr_color_sink_st>();
    }
    else if (sink_type == SINK_TYPE_STDERR_COLOR_SINK_MT) {
        ret = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    }
    else if (sink_type == SINK_TYPE_MSVC_SINK_ST) {
        ret = std::make_shared<spdlog::sinks::msvc_sink_st>();
    }
    else if (sink_type == SINK_TYPE_MSVC_SINK_MT) {
        ret = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    }
#ifdef HAS_SYSLOG
    else if (sink_type == SINK_TYPE_SYSLOG_SINK_ST) {
        ret = std::make_shared<spdlog::sinks::syslog_sink_st>(config.params.indent);
    }
    else if (sink_type == SINK_TYPE_SYSLOG_SINK_MT) {
        ret = std::make_shared<spdlog::sinks::syslog_sink_mt>(config.params.indent);
    }
#endif
    else if (sink_type == SINK_TYPE_BASIC_FILE_SINK_ST) {
        if (!hts::PathHelper::CreateFullDirectory(config.params.file_name))
            printf("%s: Create directory '%s' failure\n", __FUNCTION__, config.params.file_name.c_str());
        else
            ret = std::make_shared<spdlog::sinks::basic_file_sink_st>(config.params.file_name, config.params.truncate);
    }
    else if (sink_type == SINK_TYPE_BASIC_FILE_SINK_MT) {
        if (!hts::PathHelper::CreateFullDirectory(config.params.file_name))
            printf("%s: Create directory '%s' failure\n", __FUNCTION__, config.params.file_name.c_str());
        else
            ret = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config.params.file_name, config.params.truncate);
    }
    else if (sink_type == SINK_TYPE_DAILY_FILE_SINK_ST) {
        if (!hts::PathHelper::CreateFullDirectory(config.params.file_name))
            printf("%s: Create directory '%s' failure\n", __FUNCTION__, config.params.file_name.c_str());
        else
            ret = std::make_shared<spdlog::sinks::daily_file_sink_st>(config.params.file_name, config.params.rotation_hour, config.params.rotation_minute, config.params.truncate);
    }
    else if (sink_type == SINK_TYPE_DAILY_FILE_SINK_MT) {
        if (!hts::PathHelper::CreateFullDirectory(config.params.file_name))
            printf("%s: Create directory '%s' failure\n", __FUNCTION__, config.params.file_name.c_str());
        else
            ret = std::make_shared<spdlog::sinks::daily_file_sink_mt>(config.params.file_name, config.params.rotation_hour, config.params.rotation_minute, config.params.truncate);
    }
    else if (sink_type == SINK_TYPE_ROTATING_FILE_SINK_ST) {
        if (!hts::PathHelper::CreateFullDirectory(config.params.file_name))
            printf("%s: Create directory '%s' failure\n", __FUNCTION__, config.params.file_name.c_str());
        else
            ret = std::make_shared<spdlog::sinks::rotating_file_sink_st>(config.params.file_name, config.params.max_size, config.params.max_files);
    }
    else if (sink_type == SINK_TYPE_ROTATING_FILE_SINK_MT) {
        if (!hts::PathHelper::CreateFullDirectory(config.params.file_name))
            printf("%s: Create directory '%s' failure\n", __FUNCTION__, config.params.file_name.c_str());
        else
            ret = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(config.params.file_name, config.params.max_size, config.params.max_files);
    }

    if (ret)
    {
        // set level if any
        if (!config.level.empty())
            ret->set_level(spdlog::level::from_str(config.level));

        // set pattern if any
        if (!config.pattern.empty())
            ret->set_pattern(config.pattern);
    }

    return ret;
}

std::shared_ptr<spdlog::async_logger> SPDLogger::CreateAsync(SPD_LOGGER_CONFIG& config, uint32_t thread_count, uint32_t queue_size)
{
    std::vector<std::shared_ptr<spdlog::sinks::sink>> sink_list;
    for (auto& sink : config.sink)
    {
        auto tmp = CreateSink(sink);
        if (tmp)
            sink_list.push_back(tmp);
    }

    // 之所以要使用自家的m_tp来保证async_logger用的线程池，是因为async_logger用的weak_ptr，
    // 而registry会在程序退出时，先于htsPluginFactory析构，从而释放线程池，导致htsPluginFactory析构函数中无法正常的记录log
    if (!m_tp)
    {
       // create global thread pool if not already exists..
       auto& registry_inst = spdlog::details::registry::instance();
       std::lock_guard<std::recursive_mutex> tp_lock(registry_inst.tp_mutex());
       m_tp = registry_inst.get_tp();
       if (!m_tp)
       {
           m_tp = std::make_shared<spdlog::details::thread_pool>(queue_size, thread_count);
           spdlog::details::registry::instance().set_tp(m_tp);
       }
    }

    if (config.overrun)
        return std::make_shared<spdlog::async_logger>(config.name, begin(sink_list), end(sink_list), m_tp, spdlog::async_overflow_policy::overrun_oldest);
    else
        return std::make_shared<spdlog::async_logger>(config.name, begin(sink_list), end(sink_list), m_tp, spdlog::async_overflow_policy::block);
}

std::shared_ptr<spdlog::logger> CreateSync(SPD_LOGGER_CONFIG& config) 
{
    std::vector<std::shared_ptr<spdlog::sinks::sink>> sink_list;
    for (auto& sink : config.sink)
    {
        auto tmp = CreateSink(sink);
        if (tmp)
            sink_list.push_back(tmp);
    }
    
    return std::make_shared<spdlog::logger>(config.name, begin(sink_list), end(sink_list));
}

bool SPDLogger::Config(const char* configString)
{
    SPD_CONFIG config;
    try
    {
        iguana::from_json(config, configString, strlen(configString));
    }
    catch (std::runtime_error& e)
    {
        printf(e.what());
        return false;
    }

    // do config work
    bool ret = true;
    auto& registry_inst = spdlog::details::registry::instance();

    for (auto& logger : config.logger)
    {
        std::shared_ptr<spdlog::logger> loggerPtr;
        if (logger.async)
            loggerPtr = CreateAsync(logger, config.thread_count, config.queue_size);
        else
            loggerPtr = CreateSync(logger);

        if (loggerPtr)
        {
            if (!logger.level.empty())
                loggerPtr->set_level(spdlog::level::from_str(logger.level));

            //register_logger在同名的logger已经注册的情况下抛异常，因此先删除同名logger
            registry_inst.drop(logger.name);
            registry_inst.register_logger(loggerPtr);
            if (logger.name.empty())
                registry_inst.set_default_logger(loggerPtr);
            //registry_inst.register_and_init(loggerPtr);
        }
        else
            ret = false;
    }

    if (!config.pattern.empty())
    {
        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        //std::unique_ptr<spdlog::formatter>(new pattern_formatter(std::move(pattern), time_type)
        formatter->add_flag<thread_name_formatter>('N');
        formatter->set_pattern(config.pattern);
        spdlog::details::registry::instance().set_formatter(std::move(formatter));
    }

    m_logger = spdlog::get(instanceName());
    return ret;
}

inline size_t file_size(const char* filepath) {
    std::error_code ec;
    size_t size = std::filesystem::file_size(filepath, ec);
    if (ec)
        return 0;
    return size;
}

std::string ReadFile(const char* file_name) {
    size_t size = file_size(file_name);
    FILE* file = fopen(file_name, "rb");
    if (file != nullptr) {
        std::string buf(size, '\0');
        if (fread(buf.data(), size, 1, file) != 1) {
            buf.resize(0, '\0');
        }
        fclose(file);
        return buf;
    }
    else
        return std::string();
}

bool SPDLogger::ConfigFromFile(const char* configFilePath)
{
    auto config = ReadFile(configFilePath);
    if (!config.empty())
        return Config(config.c_str());
    else
        return false;
}

bool SPDLogger::AllowLog(int logLevel)
{
    return m_logger ? m_logger->should_log((spdlog::level::level_enum)logLevel) : false;
}

void SPDLogger::Log(int logLevel, const char* fileName, int line, const char* funcName, const char* msg)
{
    if (m_logger)
        m_logger->log(spdlog::source_loc(fileName, line, funcName), (spdlog::level::level_enum)logLevel, spdlog::string_view_t(msg));
}

void SPDLogger::EnterThread(const char* pThreadName)
{
    assert(pThreadName != nullptr);
    sprintf_s(spdlog::details::log_msg::thread_name, pThreadName);
}

void SPDLogger::LeaveThread()
{
    spdlog::details::log_msg::thread_name[0] = 0;
}