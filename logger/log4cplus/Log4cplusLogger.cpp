#include <filesystem>
#include "Log4cplusLogger.h"

static bool g_Log4cplusLoggerRegisted = IhtsPluginClassFactory::RegistClass<hts::ILogger, Log4cplusLogger>();

Log4cplusLogger::Log4cplusLogger()
{
	LogLog::getLogLog()->setInternalDebugging(false);
}

Log4cplusLogger::~Log4cplusLogger()
{
}

void Log4cplusLogger::EnterThread(const char* pThreadName)
{
	NDC& ndc = log4cplus::getNDC();
	ndc.push(pThreadName);
}

void Log4cplusLogger::LeaveThread()
{
	NDC& ndc = log4cplus::getNDC();
	ndc.pop();
	ndc.remove();
}

bool Log4cplusLogger::Config(const char* configString)
{
	return false;
}

bool Log4cplusLogger::ConfigFromFile(const char* configFileName)
{
	std::filesystem::path configFile(configFileName);
	if (std::filesystem::exists(configFile))
	{
		PropertyConfigurator::doConfigure(configFileName);
		m_logger = Logger::getInstance(log4cplus::tstring(instanceName().c_str()));
		return true;
	}
	else
		return false;
}

void Log4cplusLogger::SetInstanceName(const char* name)
{
	IhtsPluginClass::SetInstanceName(name);
	m_logger = Logger::getInstance(log4cplus::tstring(name));
}

/* 我们定义的log level为：
#define LOG_LEVEL_TRACE		0
#define LOG_LEVEL_DEBUG		1
#define LOG_LEVEL_INFO		2
#define LOG_LEVEL_WARN		3
#define LOG_LEVEL_ERROR		4
#define LOG_LEVEL_FATAL		5
#define LOG_LEVEL_OFF		6
而Log4cplus使用的是：
const LogLevel TRACE_LOG_LEVEL = 0;
const LogLevel DEBUG_LOG_LEVEL = 10000;
const LogLevel INFO_LOG_LEVEL  = 20000;
const LogLevel WARN_LOG_LEVEL  = 30000;
const LogLevel ERROR_LOG_LEVEL = 40000;
const LogLevel FATAL_LOG_LEVEL = 50000;
const LogLevel OFF_LOG_LEVEL   = 60000;
因此使用时需要转换一下
*/
void Log4cplusLogger::Log(int logLevel, const char* fileName, int line, const char* funcName, const char* msg)
{
	log4cplus::detail::macro_forced_log(m_logger, logLevel*10000, msg, fileName, line, funcName);
}

bool Log4cplusLogger::AllowLog(int logLevel)
{
	return m_logger.isEnabledFor(logLevel*10000);
}