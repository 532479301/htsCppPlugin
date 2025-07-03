#pragma once
#include "log4cplus/logger.h"
#include "log4cplus/configurator.h"
#include "log4cplus/spi/loggingevent.h"
#include "log4cplus/helpers/loglog.h"
#include "log4cplus/helpers/stringhelper.h"
#include "log4cplus/helpers/fileinfo.h"
#include "log4cplus/loggingmacros.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/initializer.h"
using namespace log4cplus;
using namespace log4cplus::spi;
using namespace log4cplus::helpers;

#include <htsLogInterface.h>

class Log4cplusLogger : public hts::ILogger
{
public:
	Log4cplusLogger();
	virtual ~Log4cplusLogger();
	virtual void SetInstanceName(const char* name) override;

public:
	virtual bool Config(const char* configString) override;
	virtual bool ConfigFromFile(const char* configFilePath) override;
	virtual bool AllowLog(int logLevel);
	virtual void Log(int logLevel, const char* fileName, int line, const char* funcName, const char* msg) override;
	virtual void EnterThread(const char* pThreadName) override;
	virtual void LeaveThread() override;

private:
	log4cplus::Logger m_logger;
	log4cplus::Initializer initializer;
};