// Copyright(c) 2025-present, ����.
// Distributed under the Apache License Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)

#include "thread_formatter.h"
#include <spdlog/details/os.h>
 
// ��ʼ�벻��spdlog��Դ���룬LTS�洢������������첽logger���棬��format�����̳߳�����̣߳�����log�̣߳����LTS�洢������ᵼ���첽logger�����޷���ȷ��¼�߳�����
// ���ղ������޸�log_msg�ṹ��ʹ��ԭʼ�߳��а�����߳�����¼����
void thread_name_formatter::format(const spdlog::details::log_msg& msg, const std::tm&, spdlog::memory_buf_t& dest)
{
    spdlog::details::scoped_padder p(msg.log_thread_name.size(), padinfo_, dest);
    spdlog::details::fmt_helper::append_string_view(msg.log_thread_name, dest);
}

