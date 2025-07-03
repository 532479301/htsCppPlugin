// Copyright(c) 2025-present, 徐翔.
// Distributed under the Apache License Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)

#pragma once

#include "htsPluginAPI.h"

namespace hts
{
#define LOG_LEVEL_TRACE		0
#define LOG_LEVEL_DEBUG		1
#define LOG_LEVEL_INFO		2
#define LOG_LEVEL_WARN		3
#define LOG_LEVEL_ERROR		4
#define LOG_LEVEL_FATAL		5
#define LOG_LEVEL_OFF		6

	class ILogger : public IhtsPluginClass
	{
	public:
		virtual ~ILogger() = default;

	public:
		virtual bool Config(const char* configString) = 0;
		virtual bool ConfigFromFile(const char* configFilePath) = 0;
		virtual bool AllowLog(int logLevel) = 0;
		virtual void Log(int logLevel, const char* fileName, int line, const char* funcName, const char* msg) = 0;
		virtual void EnterThread(const char* pThreadName) = 0;
		virtual void LeaveThread() = 0;
	};
	DEFINE_INTERFACE(ILogger);
}

#define USE_LOGGER(loggerName) hts::ILoggerPtr m_logger {IhtsPluginClassFactory::CreateClass<hts::ILogger>(nullptr, loggerName)}
#define USE_PLUGIN_LOGGER(logPluginName, loggerName) hts::ILoggerPtr m_logger {IhtsPluginClassFactory::CreateClass<hts::ILogger>(logPluginName, loggerName)}
#define DONT_USING_LOG hts::ILoggerPtr m_logger

//inline void LogPrintf(hts::ILoggerPtr logger, int logLevel, const char* fileName, int line, const char* funcName, const char* fmt, ...)
//{
//	// vsnprintf_s使用count=0没有正确返回格式化后的长度，放弃...
//	va_list args;
//	va_start(args, fmt);
//	int len = _vscprintf_p(fmt, args) + 1;
//
//	// Allocate memory for our buffer
//	char *buffer = (char*)malloc(len * sizeof(char));
//	if (buffer)
//	{
//		_vsprintf_p(buffer, len, fmt, args);
//		logger->Log(logLevel, fileName, line, funcName, buffer);
//		free(buffer);
//	}
//	va_end(args);
//}

template <class ...Args>
inline void Log(hts::ILoggerPtr logger, int logLevel, const char* fileName, int line, const char* funcName, const char* fmt, Args &&...args)
{
	// 曾经试图在本模板函数里面直接调用vsnprintf_s，使用 va_start(va, args...) 来获取可变参数列表
	// 但是VS2022编译时在莫名奇妙的地方报 fatal  error C1001 : 内部编译器错误
	// 只能放弃这个方式，转调一个普通的C的变参函数
	// 然后找到了_scprintf_p这么个函数，改成现在的这个写法
	int len = _scprintf_p(fmt, std::forward<Args>(args)...) + 1;

	if (len > 128)
	{
		char* buffer = (char*)malloc(len * sizeof(char));
		if (buffer)
		{
			_sprintf_p(buffer, len, fmt, std::forward<Args>(args)...);
			logger->Log(logLevel, fileName, line, funcName, buffer);
			free(buffer);
			return;
		}
	}
	else
	{
		char msg[128];
		_sprintf_p(msg, 128, fmt, std::forward<Args>(args)...);
		logger->Log(logLevel, fileName, line, funcName, msg);
	}
}

template<>
inline void Log(hts::ILoggerPtr logger, int logLevel, const char* fileName, int line, const char* funcName, const char* msg)
{
	logger->Log(logLevel, fileName, line, funcName, msg);
}

// printf控制格式的输出
#define  LogLevel(level, fmt, ...) {if (m_logger && m_logger->AllowLog(Level)) {Log(m_logger, Level, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);}}
#define  LogF(fmt, ...) {if (m_logger && m_logger->AllowLog(LOG_LEVEL_FATAL)) {Log(m_logger, LOG_LEVEL_FATAL, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);}} 
#define  LogE(fmt, ...) {if (m_logger && m_logger->AllowLog(LOG_LEVEL_ERROR)) {Log(m_logger, LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);}} 
#define  LogW(fmt, ...) {if (m_logger && m_logger->AllowLog(LOG_LEVEL_WARN)) {Log(m_logger, LOG_LEVEL_WARN , __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);}} 
#define  LogI(fmt, ...) {if (m_logger && m_logger->AllowLog(LOG_LEVEL_INFO)) {Log(m_logger, LOG_LEVEL_INFO , __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);}} 
#define  LogD(fmt, ...) {if (m_logger && m_logger->AllowLog(LOG_LEVEL_DEBUG)) {Log(m_logger, LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);}} 
#define  LogT(fmt, ...) {if (m_logger && m_logger->AllowLog(LOG_LEVEL_TRACE)) {Log(m_logger, LOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__);}} 

// libFmt或者说C++20的format控制格式的输出
#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))	// 202002L 是 C++ 20版本的意思
#include <format>
#define  LogFmtLevel(level, fmt, ...) {if (m_logger && m_logger->AllowLog(Level)) {m_logger->Log(Level, __FILE__, __LINE__, __func__, std::format(fmt, ##__VA_ARGS__).c_str());}}
#define  LogFmtF(fmt, ...) {if (m_logger && m_logger->AllowLog(LOG_LEVEL_FATAL)) {m_logger->Log(LOG_LEVEL_FATAL , __FILE__, __LINE__, __func__, std::format(fmt, ##__VA_ARGS__).c_str());}} 
#define  LogFmtE(fmt, ...) {if (m_logger && m_logger->AllowLog(LOG_LEVEL_ERROR)) {m_logger->Log(LOG_LEVEL_ERROR , __FILE__, __LINE__, __func__, std::format(fmt, ##__VA_ARGS__).c_str());}} 
#define  LogFmtW(fmt, ...) {if (m_logger && m_logger->AllowLog(LOG_LEVEL_WARN)) {m_logger->Log(LOG_LEVEL_WARN , __FILE__, __LINE__, __func__, std::format(fmt, ##__VA_ARGS__).c_str());}} 
#define  LogFmtI(fmt, ...) {if (m_logger && m_logger->AllowLog(LOG_LEVEL_INFO)) {m_logger->Log(LOG_LEVEL_INFO , __FILE__, __LINE__, __func__, std::format(fmt, ##__VA_ARGS__).c_str());}} 
#define  LogFmtD(fmt, ...) {if (m_logger && m_logger->AllowLog(LOG_LEVEL_DEBUG)) {m_logger->Log(LOG_LEVEL_DEBUG , __FILE__, __LINE__, __func__, std::format(fmt, ##__VA_ARGS__).c_str());}} 
#define  LogFmtT(fmt, ...) {if (m_logger && m_logger->AllowLog(LOG_LEVEL_TRACE)) {m_logger->Log(LOG_LEVEL_TRACE , __FILE__, __LINE__, __func__, std::format(fmt, ##__VA_ARGS__).c_str());}} 
#endif