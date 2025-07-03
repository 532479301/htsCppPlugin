// htsplugin_log_sample.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <thread>
#include <htsPluginAPI.h>
#include <htsLogInterface.h>

void Thread1()
{
	USE_LOGGER("");
    m_logger->EnterThread("Thread1");
    LogI("Enter Thread1");

	for (int i = 0; i < 20; i++)
	{
		LogD("Loop %d", i);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
    LogI("Leave Thread1");
    m_logger->LeaveThread();
}

void UseLog(const char* pluginName, const char* configName, const char* loggerName)
{
    USE_PLUGIN_LOGGER(pluginName, loggerName);
    if (configName != nullptr)
        m_logger->ConfigFromFile(configName);
    m_logger->EnterThread("main");
    LogI("Start UseLog");

    std::thread t = std::thread(Thread1);
    for (int i = 0; i < 10; i++)
    {
        LogD("Loop %d", i);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    t.join();
    m_logger->LeaveThread();
}

// 使用如下的命令来运行：
// htsplugin_log_sample htsplugin_SPDLog\SPDLogger spd_log.config htsPlugin_spdlog_sample  // 使用spdlog
// htsplugin_log_sample htsplugin_log4cplus\Log4cplusLogger log4cplus.config sample // 使用Log4cplus
int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: \n\thtsplugin_log_sample log_plugin_name [log_config] [logger_name]\n");
        printf("Where:\n\tlog_plugin_name shall be a log plugin's full name. It should be htsplugin_log4cplus\\ or htsplugin_SPDLog\\SPDLogger or what you implemented.\n");
        printf("\tlog_config shall be the config file name for you selected log plugin\n");
        printf("\tlogger_name shall be the logger'a name, it will be root logger if not given.\n");
    }
    else
    {
        const char* pluginName = argv[1];
        const char* configName = nullptr;
        const char* loggerName = "";
        if (argc >= 3)
            configName = argv[2];
        if (argc >= 4)
            loggerName = argv[3];
        UseLog(pluginName, configName, loggerName);
    }
    return 0;
}