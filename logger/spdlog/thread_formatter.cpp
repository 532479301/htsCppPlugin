// Copyright(c) 2025-present, 徐翔.
// Distributed under the Apache License Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)

#include "thread_formatter.h"
#include <spdlog/details/os.h>
 
// 开始想不改spdlog的源代码，LTS存储放这里，但是在异步logger里面，调format是在线程池里的线程，不是log线程，因此LTS存储放这里会导致异步logger根本无法正确记录线程名称
// 最终不得已修改log_msg结构，使在原始线程中把这个线程名记录下来
void thread_name_formatter::format(const spdlog::details::log_msg& msg, const std::tm&, spdlog::memory_buf_t& dest)
{
    spdlog::details::scoped_padder p(msg.log_thread_name.size(), padinfo_, dest);
    spdlog::details::fmt_helper::append_string_view(msg.log_thread_name, dest);
}

