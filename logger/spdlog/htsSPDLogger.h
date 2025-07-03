// Copyright(c) 2025-present, ÐìÏè.
// Distributed under the Apache License Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)

#pragma once

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <iguana/json_reader.hpp>
#include "htsLogInterface.h"
#include <spdlog/spdlog.h>
#include <spdlog/async_logger.h>


struct SPD_SINK_PARAM
{
    // paremter for syslog_sink
    std::string indent;
    // paremter for file_sink
    std::string file_name;
    bool truncate = false;
    // additional paremter for daily_file_sink
    int32_t rotation_hour = 2;
    int32_t rotation_minute = 0;
    // additional paremter for rotating_file_sink
    size_t max_files = 10;
    size_t max_size = 1024 * 1024 * 10;
};
REFLECTION(SPD_SINK_PARAM, indent, file_name, truncate, rotation_hour, rotation_minute, max_files, max_size)

struct SPD_SINK_CONFIG
{
    std::string name;
    std::string level;
    std::string pattern;
    SPD_SINK_PARAM params;
};
REFLECTION(SPD_SINK_CONFIG, name, params)

struct SPD_LOGGER_CONFIG
{
    std::string name;
    bool async = true;
    bool overrun = false;
    std::string level;
    std::vector<SPD_SINK_CONFIG> sink;
};
REFLECTION(SPD_LOGGER_CONFIG, name, async, level, sink)

struct SPD_CONFIG
{
    uint32_t thread_count = 1;
    uint32_t queue_size = 8192;
    std::string pattern;
    std::vector<SPD_LOGGER_CONFIG> logger;
};
REFLECTION(SPD_CONFIG, pattern, logger)

class SPDLogger : public hts::ILogger
{
public:
	virtual ~SPDLogger();
	virtual void SetInstanceName(const char* name) override;

public:
	virtual bool Config(const char* configString) override;
    virtual bool ConfigFromFile(const char* configFilePath) override;
	virtual bool AllowLog(int logLevel);
	virtual void Log(int logLevel, const char* fileName, int line, const char* funcName, const char* msg) override;
	virtual void EnterThread(const char* pThreadName) override;
	virtual void LeaveThread() override;

private:
	std::shared_ptr<spdlog::async_logger> CreateAsync(SPD_LOGGER_CONFIG& config, uint32_t thread_count, uint32_t queue_size);

private:
	std::shared_ptr<spdlog::logger> m_logger;
	std::shared_ptr<spdlog::details::thread_pool> m_tp;
};

