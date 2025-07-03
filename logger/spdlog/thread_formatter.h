// Copyright(c) 2025-present, ÐìÏè.
// Distributed under the Apache License Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/pattern_formatter.h>
#include <shared_mutex>

class thread_name_formatter : public spdlog::custom_flag_formatter
{
public:
    virtual void format(const spdlog::details::log_msg& msg, const std::tm&, spdlog::memory_buf_t& dest) override;
    virtual std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<thread_name_formatter>();
    }
};

