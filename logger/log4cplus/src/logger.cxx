// Module:  Log4CPLUS
// File:    logger.cxx
// Created: 6/2001
// Author:  Tad E. Smith
//
//
// Copyright 2001-2017 Tad E. Smith
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <log4cplus/logger.h>
#include <log4cplus/appender.h>
#include <log4cplus/hierarchy.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/spi/loggerimpl.h>
#include <utility>


namespace log4cplus
{


Logger
DefaultLoggerFactory::makeNewLoggerInstance (const log4cplus::tstring & name,
    Hierarchy& h)
{
    return Logger (makeNewLoggerImplInstance(name, h));
}


spi::LoggerImpl *
DefaultLoggerFactory::makeNewLoggerImplInstance(const log4cplus::tstring& name,
    Hierarchy& h)
{
    return new spi::LoggerImpl (name, h);
}


//////////////////////////////////////////////////////////////////////////////
// static Logger Methods
//////////////////////////////////////////////////////////////////////////////
//
Hierarchy &
Logger::getDefaultHierarchy ()
{
    return log4cplus::getDefaultHierarchy ();
}


bool
Logger::exists (const log4cplus::tstring & name)
{
    return getDefaultHierarchy().exists(name);
}


LoggerList
Logger::getCurrentLoggers ()
{
    return getDefaultHierarchy ().getCurrentLoggers ();
}


Logger
Logger::getInstance (const log4cplus::tstring& name)
{
    return getDefaultHierarchy().getInstance(name);
}


Logger
Logger::getInstance (const log4cplus::tstring& name,
    spi::LoggerFactory& factory)
{
    return getDefaultHierarchy().getInstance(name, factory);
}


Logger
Logger::getRoot ()
{
    return getDefaultHierarchy ().getRoot ();
}

extern bool is_destroyed(); // Added by Xu Xiang

void Logger::shutdown ()
{
    // 发现在某些情况下，执行到shutdown时DefaultContext已经被destroyed了，简单的增加个判断。
    if (!is_destroyed()) // Added by Xu Xiang
        getDefaultHierarchy ().shutdown ();
}


//////////////////////////////////////////////////////////////////////////////
// Logger ctors and dtor
//////////////////////////////////////////////////////////////////////////////

Logger::Logger () LOG4CPLUS_NOEXCEPT
{ }


Logger::Logger (spi::LoggerImpl * ptr) LOG4CPLUS_NOEXCEPT
    : value (ptr)
{
    if (value)
        value->addReference ();
}


Logger::Logger (const Logger& rhs) LOG4CPLUS_NOEXCEPT
    : spi::AppenderAttachable (rhs)
    , value (rhs.value)
{
    if (value)
        value->addReference ();
}


Logger &
Logger::operator = (const Logger& rhs) LOG4CPLUS_NOEXCEPT
{
    Logger (rhs).swap (*this);
    return *this;
}


Logger::Logger (Logger && rhs) LOG4CPLUS_NOEXCEPT
    : spi::AppenderAttachable (std::move (rhs))
    , value (rhs.value)
{
    rhs.value = nullptr;
}


Logger &
Logger::operator = (Logger && rhs) LOG4CPLUS_NOEXCEPT
{
    Logger (std::move (rhs)).swap (*this);
    return *this;
}


Logger::~Logger ()
{
    if (value)
        value->removeReference ();
}


//////////////////////////////////////////////////////////////////////////////
// Logger Methods
//////////////////////////////////////////////////////////////////////////////

void
Logger::swap (Logger & other) LOG4CPLUS_NOEXCEPT
{
    std::swap (value, other.value);
}


Logger
Logger::getParent () const
{
    if (value->parent)
        return Logger (value->parent.get ());
    else
    {
        helpers::getLogLog().error(
            LOG4CPLUS_TEXT("********* This logger has no parent: ")
            + getName());
        return *this;
    }
}


void
Logger::addAppender (SharedAppenderPtr newAppender)
{
    value->addAppender(newAppender);
}


SharedAppenderPtrList
Logger::getAllAppenders ()
{
    return value->getAllAppenders();
}


SharedAppenderPtr
Logger::getAppender (const log4cplus::tstring& name)
{
    return value->getAppender (name);
}


void
Logger::removeAllAppenders ()
{
    value->removeAllAppenders ();
}


void
Logger::removeAppender (SharedAppenderPtr appender)
{
    value->removeAppender(appender);
}


void
Logger::removeAppender (const log4cplus::tstring& name)
{
    value->removeAppender (name);
}


void
Logger::assertion (bool assertionVal, const log4cplus::tstring& msg) const
{
    if (! assertionVal)
        log (FATAL_LOG_LEVEL, msg, nullptr, -1);
}


void
Logger::closeNestedAppenders () const
{
    value->closeNestedAppenders ();
}


bool
Logger::isEnabledFor (LogLevel ll) const
{
    return value->isEnabledFor (ll);
}


void
Logger::log (LogLevel ll, const log4cplus::tstring& message, const char* file,
    int line, const char* function) const
{
    value->log (ll, message, file, line, function ? function : "");
}


void
Logger::log (spi::InternalLoggingEvent const & ev) const
{
    value->log (ev);
}


void
Logger::forcedLog (LogLevel ll, const log4cplus::tstring& message,
    const char* file, int line, const char* function) const
{
    value->forcedLog (ll, message, file, line, function ? function : "");
}


void
Logger::forcedLog (spi::InternalLoggingEvent const & ev) const
{
    value->forcedLog (ev);
}


void
Logger::callAppenders (const spi::InternalLoggingEvent& event) const
{
    value->callAppenders (event);
}


LogLevel
Logger::getChainedLogLevel () const
{
    return value->getChainedLogLevel ();
}


LogLevel
Logger::getLogLevel() const
{
    return value->getLogLevel ();
}


void
Logger::setLogLevel (LogLevel ll)
{
    value->setLogLevel (ll);
}


Hierarchy &
Logger::getHierarchy () const
{
    return value->getHierarchy ();
}


log4cplus::tstring const &
Logger::getName () const
{
    return value->getName ();
}


bool
Logger::getAdditivity () const
{
    return value->getAdditivity ();
}


void
Logger::setAdditivity (bool additive)
{
    value->setAdditivity (additive);
}


} // namespace log4cplus
